//=================== pursuit ===================//

/*
 1. If there is only one rectangular, then calculate its ratio(longer side / shorter side).
 2. If the ratio is larger than the threhold value, then we think it may be a action of pursuit.
 3. divide the current rectangular into two parts（2 mice）by the smallest value of histogram which is also at the middle of the longer side.
 4. find the rear part of each mouse by find the head part （using the method in food preference）
 5. find the current red dot position, if the red dot can not be detected in the current frame, we need to get the position of red dot from the nearest 2 frames（before and after） of the current frame.
 6. There are 2 cases of comparing the distances between the 2 red dots（R1, R2） and 2 rear centroids （C1, C2）
 6.1 if (D(R1, C1) >= D(R1, C2) && D(R2, C1) >= D(R2, C2)) || (D(R1, C1) <= D(R1, C2) && D(R2, C1) <= D(R2, C2)), then we think the C2(C1) is the male mouse
 6.2 if (D(R1, C1) >= D(R1, C2) && D(R2, C1) <= D(R2, C2)) || (D(R1, C1) <= D(R1, C2) && D(R2, C1) >= D(R2, C2)), then we need to compare D(R1, C2) and D(R2, C1) if D(R1, C2) < D(R2, C1) then C2 is the male mouse.
 */

/*
 confidence value:
 0 -- two rect centroids = {(-2,-2), (-2, -2)}
 1 -- 1, ratio not  centroids = {(-1,-1), (-1, -1)}
 2 -- 1, ratio is, rear head head rear
 3 -- 1, ratio is, male female
 4 -- 1, ratio is, female male
 */

#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <fstream>

using namespace std;
using namespace cv;

const static int SENSITIVITY_VALUE = 20;


vector<Point> findSand(Mat& src);
Point findRedDot(Mat& frame, Mat& src);
vector<Point> findMice(Mat& frame, Mat& src, vector<Point> sandArea);
vector<Point> divideRect(Rect rect, vector<Point> mice, Mat& frame);
Point getCentroid(vector<Point>& contour);
vector<int> analysis(vector<Point>& redDots, vector<vector<Point>>& rearCetroids);

int main()
{
    String trainpath = "./pursuit";
    vector<String> filenames;
    cv::glob(trainpath, filenames);
    ofstream outfile("pursuit_confidenceValue.txt");
    
    for(int i = 0; i < filenames.size(); i++){
        if(filenames[i] == "./pursuit/.DS_Store")
            continue;
        VideoCapture capture;
        capture.open(filenames[i]);
        if(!capture.isOpened()){
            cout << "ERROR ACQUIRING VIDEO FEED\n";
            getchar();
            return -1;
        }
        
        cout << filenames[i] << endl;
        
        vector<Point> redDotAppearance;
        vector<vector<Point>> rearCentoids;
        
        while(1){
            Mat frame1;
            if(!capture.read(frame1)){
                cout << "Cannot read the video file. \n";
                break;
            }
//            double rate=capture.get(CV_CAP_PROP_FPS);
//            cout << "rate is : " << rate << endl;
            // find sand area
            Mat src = frame1;
            Mat dst = Mat::zeros(frame1.rows, frame1.cols, CV_8UC1);
            vector<Point> sandArea = findSand(src);
            
            // find red dot
            Point reddot = findRedDot(frame1, src);
            redDotAppearance.push_back(reddot);
            
            // find two mice
            vector<Point> cps = findMice(frame1, src, sandArea);
            rearCentoids.push_back(cps);
            Point c1 = cps[0];
            Point c2 = cps[1];
            if(!(c1.x == -2 && c1.y == -2 && c2.x == -2 && c2.y == -2) && !(c1.x == -1 && c1.y == -1 && c2.x == -1 && c2.y == -1)){
                int cdistance = pow(c1.x - c2.x, 2) + pow(c1.y - c2.y, 2);
                int rc1 = pow(c1.x - reddot.x, 2) + pow(c1.y - reddot.y, 2);
                int rc2 = pow(c2.x - reddot.x, 2) + pow(c2.y - reddot.y, 2);
                cout << "================ centroid distance = " << cdistance << endl;
                cout << "================ rc1 distance = " << rc1 << endl;
                cout << "================ rc2 distance = " << rc2 << endl;
                cout << endl;
            }
            
//            imshow("Result", src);
            
            if(waitKey(30) == 27) // Wait for 'esc' key press to exit
            {
                break;
            }
        }
        vector<int> confidenceValues = analysis(redDotAppearance, rearCentoids);
        
        // write cv into txt
        outfile << filenames[i] << ",";
        for(int cv : confidenceValues)
            outfile << cv << ",";
        outfile << endl;
    }
    return 0;
}

Point getCentroid(vector<Point>& contour){
    Moments moment = moments(contour, false);
    Point centroid;
    centroid = Point(moment.m10 / moment.m00, moment.m01 / moment.m00);
    
    return centroid;
}

void morphological(Mat& src) {
    Mat element1 = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat element2 = getStructuringElement(MORPH_RECT, Size(4, 4));
    Mat element3 = getStructuringElement(MORPH_RECT, Size(5, 5));
    medianBlur(src, src, 3);
    //    erode(src, src, element1);
    dilate(src, src, element2);
    //    erode(src, src, element3);
}

bool isSand(Vec3b point){
    int B = point[0]; int G = point[1]; int R = point[2];
    if (R > 100 && G > 70 && B > 50)
        return true;
    return false;
}


vector<Point> findSand(Mat& src){
    vector<Point> sandArea;
    vector<int> rowHistogram(src.rows, 0);
    vector<int> colHistogram(src.cols, 0);
    for(int i = 0; i < src.rows; i++){
        for(int j = 0; j < src.cols; j++){
            if(isSand(src.at<Vec3b>(i, j))){
                rowHistogram[i]++;
                colHistogram[j]++;
            }
        }
    }
    
    int srow = 0, erow = 0;
    // up
    for(int i = 80; i < rowHistogram.size(); i++){
        if(rowHistogram[i] > 350){
            srow = i;
            break;
        }
    }
    
    // bottom
    for(int i =  (int)rowHistogram.size() - 60; i >= 0; i--){
        if(rowHistogram[i] > 350){
            erow = i;
            break;
        }
    }
    // left
    int scol = 0, ecol = 0;
    for(int j = 0; j < colHistogram.size(); j++){
        if(colHistogram[j] > 200){
            scol = j;
            break;
        }
    }
    // right
    for(int j = (int)colHistogram.size() - 150; j >= 0; j--){
        if(colHistogram[j] > 200){
            ecol = j;
            break;
        }
    }
    rectangle(src, Point(scol, srow), Point(ecol, erow),  cv::Scalar(255, 0, 0));
    sandArea = {Point(scol, srow), Point(ecol, erow)};
    return sandArea;
}

bool isRedDot(Vec3b point){
    int B = point[0]; int G = point[1]; int R = point[2];
    if (R > 180 && R < 230 && G > 50 && G < 100 && B > 70 && B < 120)
        return true;
    return false;
}


Point findRedDot(Mat& frame, Mat& src) {
    Mat dst = Mat::zeros(frame.rows, frame.cols, CV_8UC1);
    for(int i = 0; i < frame.rows; i++){
        for(int j = 0; j < frame.cols; j++){
            if(isRedDot(frame.at<Vec3b>(i, j))){
                dst.at<uchar>(i, j) = 255;
            }
        }
    }
    morphological(dst);
    
    // draw red dot's contour
    Mat thres_output;
    threshold(dst, thres_output, SENSITIVITY_VALUE, 255, THRESH_BINARY);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(thres_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    
    int maxSize = 0;
    int maxInd = 0;
    for(int i = 0; i < contours.size(); i++){
        double area = contourArea(contours[i]);
        if(area > maxSize){
            maxSize = area;
            maxInd = i;
        }
    }
    drawContours(src, contours, maxInd, Scalar(255, 0, 255), 2, 8, hierarchy);
    Point dot = Point(-1, -1);
    if(contours.size() > 0)
        dot = getCentroid(contours[maxInd]);
    
    return dot;
}

bool isMouse(Vec3b point){
    int B = point[0]; int G = point[1]; int R = point[2];
    //    if (R < 40 && abs(R-G) < 20 && abs(G-B) < 10)
    if (abs(R-G) < 20 && abs(G-B) < 10)
        return true;
    return false;
}

bool compContourPairs(pair<double, int> p1, pair<double, int> p2){
    return p1.first > p2.first;
}

vector<Point> findMice(Mat& frame, Mat& src, vector<Point> sandArea) {
    
    vector<Point> centroids = {Point(-2, -2), Point(-2, -2)};
    
    Mat dst = Mat::zeros(frame.rows, frame.cols, CV_8UC1);
    for(int i = sandArea[0].x; i < sandArea[1].x; i++){
        for(int j = sandArea[0].y; j < sandArea[1].y; j++){
            Vec3b point = src.at<Vec3b>(j, i);
            if(isMouse(point))
                dst.at<uchar>(j, i) = 255;
        }
    }
    
    Mat thres_output;
    threshold(dst, thres_output, SENSITIVITY_VALUE, 255, THRESH_BINARY);
    vector<vector<Point>> contours; // all points of all contours
    vector<Vec4i> hierarchy;
    findContours(thres_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    vector<pair<double, int>> contourPairs; // <area, index>
    
    for(int i = 0; i < contours.size(); i++){
        double area = contourArea(contours[i]);
        contourPairs.push_back({area, i});
    }
    
    std::sort(contourPairs.begin(), contourPairs.end(), compContourPairs);
    
    int rect_count = 0;
    for(int i = 0; i < 2 ; i++){
        if(contourPairs[i].first > 7000) {
            rect_count++;
            
            vector<Point> contour_poly;
            approxPolyDP(Mat(contours[contourPairs[i].second]), contour_poly, 3, true);
            Rect boundRect = boundingRect(Mat(contour_poly));
            rectangle(frame, boundRect.tl(), boundRect.br(), Scalar(0, 0, 255), 2, 8, 0);
            drawContours(frame, contours, contourPairs[i].second, Scalar(0, 0, 255), 2, 8, hierarchy);
        }
    }
    
    // check the number of rect and the ratio of rect
    if(rect_count == 1){
        vector<Point> poly_one;
        approxPolyDP(Mat(contours[contourPairs[0].second]),  poly_one, 3, true);
        Rect rect = boundingRect(Mat( poly_one));
        double ratio = rect.height > rect.width ? (double)rect.height / rect.width : (double)rect.width / rect.height;
        //        cout << "===================== ratio = " << ratio << endl;
        
        centroids = {Point(-1,-1), Point(-1,-1)};
        
        if (ratio > 2.9) {
            rectangle(frame, rect.tl(), rect.br(), Scalar(255, 255, 0), 2, 8, 0);
            centroids = divideRect(rect, contours[contourPairs[0].second], frame);
        }
    }
    return centroids;
}

Point findRearPart(int x, int y, int width, int height, vector<Point> mice, Mat& frame){
    vector<vector<Point>> bodyParts(2, vector<Point>());
    vector<vector<Point>> partContour(2, vector<Point>());
    
    //    vector<pair<int, int>> divide = {{x, x+width/3}, {x+width/3, x+width*2/3},{x+width*2/3, x + width}};
    vector<pair<int, int>> divide = {{x, x+width/2}, {x+width/2, x+width}};
    
    for(int k = 0; k < bodyParts.size(); k++){
        for(int i = divide[k].first; i < divide[k].second; i++){
            for(int j = y; j < y + height; j++){
                int where = (int) pointPolygonTest(mice, Point2f(i, j), false);
                if(where >= 0){
                    if(where == 0)
                        partContour[k].push_back(Point(i, j));
                    bodyParts[k].push_back(Point(i, j));
                }
            }
        }
    }
    
    int minInd;
    int minArea = INT_MAX;
    for(int i = 0; i < bodyParts.size(); i++){
        if(bodyParts[i].size() < minArea){
            minArea = (int)bodyParts[i].size();
            minInd = i;
        }
    }
    
    for(Point p : bodyParts[1-minInd]){
        frame.at<Vec3b>(p.y, p.x) = Vec3b(255, 255, 255);
    }
    
    
    //    int rearInd;
    //    if(minInd == 0 || minInd == 2)
    //        rearInd = 2 - minInd;
    //    else
    //        rearInd = bodyParts[0].size() > bodyParts[2].size() ? 0 : 2;
    
    //    return bodyParts[rearInd];
    
    for(int j = y; j < y+height; j++){
        partContour[1-minInd].push_back(Point(x+width/2, j));
    }
    
    Point centroid = getCentroid( partContour[1-minInd]);
    
    return centroid;
}

vector<Point> divideRect(Rect rect, vector<Point> mice, Mat& frame){
    int rx, ry, rw, rh;
    if(rect.height > rect.width){
        rx = rect.tl().y;
        ry = rect.tl().x;
        rw = rect.height;
        rh = rect.width;
    } else {
        rx = rect.tl().x;
        ry = rect.tl().y;
        rw = rect.width;
        rh = rect.height;
    }
    
    Point rearContour1 = findRearPart(rx, ry, rw/2, rh, mice, frame);
    Point rearContour2 = findRearPart(rx + rw/2, ry, rw/2, rh, mice, frame);
    
    circle(frame, rearContour1, 4, Vec3b(255, 0, 0), -1, 8, 0);
    circle(frame, rearContour2, 4, Vec3b(255, 0, 0), -1, 8, 0);
    
    vector<Point> rearCentroids = {rearContour1, rearContour2};
    
    return rearCentroids;
}

vector<int> analysis(vector<Point>& redDots, vector<vector<Point>>& rearCetroids){
    vector<int> cv;
    int frameSize = (int)rearCetroids.size();
    for(int i = 0; i < frameSize; i++){
        Point c1 = rearCetroids[i][0];
        Point c2 = rearCetroids[i][1];
        if(c1.x == -2 && c1.y == -2 && c2.x == -2 && c2.y == -2){
            // 2 rect
            cv.push_back(0);
        } else if(c1.x == -1 && c1.y == -1 && c2.x == -1 && c2.y == -1){
            // 1 rect, no ratio
            cv.push_back(1);
        } else {
            // 1 rect, ratio, cv = 2/3/4
            
            int cdistance = pow(c1.x - c2.x, 2) + pow(c1.y - c2.y, 2);
            if(cdistance < 60000){
                cv.push_back(2);
            }else {
                Point reddot = redDots[i];
                if(reddot.x == -1 && reddot.y == -1){
                    int back = i;
                    while(back < frameSize && redDots[back].x == -1 && redDots[back].y == -1)
                        back++;
                    int front = i;
                    while (front >= 0 && redDots[front].x == -1 && redDots[front].y == -1)
                        front--;
                    Point frontDot = redDots[front];
                    Point backDot = redDots[back];
                    int front_c1 = pow(frontDot.x - c1.x, 2) + pow(frontDot.y - c1.y, 2);
                    int front_c2 = pow(frontDot.x - c2.x, 2) + pow(frontDot.y - c2.y, 2);
                    int back_c1 = pow(backDot.x - c1.x, 2) + pow(backDot.y - c1.y, 2);
                    int back_c2 = pow(backDot.x - c2.x, 2) + pow(backDot.y - c2.y, 2);
                    
                    if((back_c1 > back_c2 && front_c1 > front_c2) || (back_c2 > back_c1 && front_c2 > front_c1))
                        cv.push_back(4);
                    else
                        cv.push_back(3);
                } else {
                    // assume rc1 is the larger distance rc2 is the smaller distance
                    int rc1 = pow(c1.x - reddot.x, 2) + pow(c1.y - reddot.y, 2);
                    int rc2 = pow(c2.x - reddot.x, 2) + pow(c2.y - reddot.y, 2);
                    
                    if(rc1 < rc2)
                        swap(rc1, rc2);
                    if(rc1 > 100000 && rc2 < 30000)
                        cv.push_back(4);
                    else
                        cv.push_back(3);
                }
            }
        }
    }
    return cv;
}
