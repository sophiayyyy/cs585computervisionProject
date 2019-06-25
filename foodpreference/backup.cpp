//
////===================  Food preference ===================//
//
//#include "opencv2/core/core.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <vector>
//#include <unordered_map>
//#include <fstream>
//
//using namespace cv;
//using namespace std;
//
//const static int SENSITIVITY_VALUE = 20;
//RNG rng(12345);
//vector<Point> solidFood = {Point(230, 200), Point(600, 500)};
//vector<Point> softFood = {Point(650, 200), Point(1000, 550)};
//
///*
// Confidence Value:
// -3 -- eating the solid-food area
// -2 -- smelling the solid-food area
// -1 -- hanging around the solid-food area
// 0 -- at the middle area between the solid-food area and the soft-food area
// 1 -- hanging around the soft-food area
// 2 -- smelling the soft-food area
// 3 -- eating the soft-food area
// */
//
///*
// judging body centroid
// +-1, +-2
// if(+-3)
// judging head centroid
// +-2 / +-1
// */
//
//class MouseAnalysis {
//public:
//    Point headCentroid = Point(0, 0);
//    Point bodyCentroid = Point(0, 0);
//    int gender = 1; // 0 - female  1-male
//    MouseAnalysis(Point hc, Point bc, int g): headCentroid(hc), bodyCentroid(bc), gender(g){
//        
//    }
//    // return position value
//    int position_analysis(){
//        int pv = 0;
//        
//        if(bodyCentroid.x >= solidFood[0].x
//           && bodyCentroid.x <= solidFood[1].x && bodyCentroid.y >= solidFood[0].y && bodyCentroid.y <= solidFood[1].y){
//            pv = -2;
//            if(headCentroid.x >= solidFood[0].x
//               && headCentroid.x <= solidFood[1].x && headCentroid.y >= solidFood[0].y && headCentroid.y <= solidFood[1].y)
//                pv = -3;
//        } else if(bodyCentroid.x + 20 >= softFood[0].x && bodyCentroid.x - 20 <= softFood[1].x && bodyCentroid.y + 40 >= softFood[0].y && bodyCentroid.y - 40 <= softFood[1].y){
//            pv = 2;
//            if(headCentroid.x >= softFood[0].x && headCentroid.x <= softFood[1].x && headCentroid.y >= softFood[0].y && headCentroid.y <= softFood[1].y)
//                pv = 3;
//        } else if(bodyCentroid.x < solidFood[0].x || ((bodyCentroid.y < solidFood[0].y || bodyCentroid.y > solidFood[1].y) && (bodyCentroid.x >= solidFood[0].x && bodyCentroid.x <= solidFood[1].x))){
//            pv = -1;
//        } else if(bodyCentroid.x > softFood[1].x || ((bodyCentroid.y < softFood[0].y || bodyCentroid.y > softFood[1].y) && (bodyCentroid.x >= softFood[0].x && bodyCentroid.x <= softFood[1].x))){
//            pv = 1;
//        }else{
//            pv = 0;
//        }
//        
//        return pv;
//    }
//};
//
//class PeriodAnalysis{
//public:
//    vector<int> positions;
//    vector<Point> bodycentroids;
//    vector<Point> headcentroids;
//    
//    PeriodAnalysis(){
//        //        positions = vector<int>(size, -4);
//        //        bodycentroids = vector<Point>(size, Point(-1, -1));
//        //        headcentroids = vector<Point>(size, Point(-1, -1));
//    }
//    int analysis(){
//        int cv = 0;
//        unordered_map<int, int> valueMap;
//        for(int val : positions){
//            valueMap[val]++;
//        }
//        int max_count = INT_MIN;
//        for(auto it = valueMap.begin(); it != valueMap.end(); it++)
//            if(it->second > max_count)
//                cv = it->first;
//        return cv;
//    }
//};
//
//vector<vector<Point>> findTail(Mat& src, Mat& dst, vector<Point>& sandArea);
//void findBody(Mat& src, Mat& dst, vector<Point>& sandArea);
//vector<Point> findSand(Mat& src);
//void morphological(Mat& src);
//pair<Rect, vector<Point>> getMouseCounter(Mat& src, Mat& frame);
//void findEars(Rect& mouse, Mat& src, Mat& dst);
//void getEarsCounter(Mat& src, Mat& frame);
//Point getHeadCentroid(Rect& mouse, vector<Point>& body, Mat& src);
//Point getCentroid(vector<Point>& contour);
//unordered_map<string, unordered_map<int, vector<vector<Point>>>> readLocations(string filePath);
//
//vector<int> confidenceValues;
//
//int main()
//{
//    string filePath = "./foodlocation.txt";
//    unordered_map<string, unordered_map<int, vector<vector<Point>>>> foodLocations = readLocations(filePath);
//    
//    ofstream outfile("foodPreference_confidenceValue.txt");
//    
//    vector<Point> solidOriginal = solidFood;
//    vector<Point> softOriginal = softFood;
//    
//    String trainpath = "./test";
//    vector<String> filenames;
//    cv::glob(trainpath, filenames);
//    
//    for(int i = 0; i < filenames.size(); i++){
//        if(filenames[i] == "./test/.DS_Store")
//            continue;
//        VideoCapture capture;
//        capture.open(filenames[i]);
//        if(!capture.isOpened()){
//            cout << "ERROR ACQUIRING VIDEO FEED\n";
//            getchar();
//            return -1;
//        }
//        
//        cout << filenames[i] << endl;
//        
//        string subFilename;
//        vector<string> subnames;
//        stringstream ss(filenames[i]);
//        while (getline(ss, subFilename, '/')) {
//            subnames.push_back(subFilename);
//        }
//        
//        double totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
//        int periodFrame = 10;
//        vector<PeriodAnalysis> periods(totalFrameNumber/periodFrame, PeriodAnalysis());
//        
//        
//        namedWindow(filenames[i], CV_WINDOW_NORMAL);
//        
//        int frameCount = 0;
//        int periodCount = 0;
//        
//        unordered_map<int, vector<vector<Point>>> locs;
//        if(foodLocations.find(subnames.back()) != foodLocations.end()){
//            locs = foodLocations[filenames[i]];
//        }
//        
//        while(1){
//            
//            Mat frame1;
//            if(!capture.read(frame1)){
//                cout << "Cannot read the video file. \n";
//                break;
//            }
//            frameCount++;
//            
//            if(locs.find(frameCount) != locs.end()){
//                solidFood = locs[frameCount][0];
//                softFood = locs[frameCount][1];
//            } else {
//                solidFood = solidOriginal;
//                softFood = softOriginal;
//            }
//            
//            if(frameCount % 3 == 0){
//                if(frameCount >= 30){
//                    frameCount = 0;
//                    int cv = periods[periodCount].analysis();
//                    confidenceValues.push_back(cv);
//                    periodCount++;
//                }
//                
//                
//                rectangle(frame1, solidFood[0], solidFood[1], Scalar(0, 255, 0));
//                rectangle(frame1, softFood[0], softFood[1], Scalar(0, 0, 255));
//                
//                // find mouse' body
//                Mat src = frame1;
//                Mat dst = Mat::zeros(frame1.rows, frame1.cols, CV_8UC1);
//                vector<Point> sandArea = findSand(src);
//                findBody(src, dst, sandArea);
//                morphological(dst);
//                pair<Rect, vector<Point>> mouse = getMouseCounter(dst, src);
//                
//                // get body's centroid
//                Point bodyCentroid = getCentroid(mouse.second);
//                
//                // find mouse' head
//                Mat src2 = frame1;
//                // get head's centroid
//                Point headCentroid = getHeadCentroid(mouse.first, mouse.second, src2);
//                
//                //draw 2 centroids
//                Mat centroid_src = frame1;
//                circle(centroid_src, bodyCentroid, 4, Vec3b(255, 0, 0), -1, 8, 0);
//                circle(centroid_src, headCentroid, 4, Vec3b(255, 0, 0), -1, 8, 0);
//                //                imshow(filenames[i]+"Centroids", centroid_src);
//                
//                // get confidence value
//                MouseAnalysis ma(headCentroid, bodyCentroid, 0);
//                int pv = ma.position_analysis();
//                periods[periodCount].positions.push_back(pv);
//                
//                // find ears - discard
//                //            Mat ear_dst = Mat::zeros(frame1.rows, frame1.cols, CV_8UC1);
//                //            findEars(mouse.first, src, ear_dst);
//                //            getEarsCounter(ear_dst, src);
//                
//                // find tails - discard
//                //            Mat src3 = frame1;
//                //            Mat tail_dst = Mat::zeros(frame1.rows, frame1.cols, CV_8UC1);
//                //            vector<vector<Point>> tails = findTail(src3, tail_dst, sandArea);
//                
//                if(waitKey(30) == 27) // Wait for 'esc' key press to exit
//                {
//                    break;
//                }
//            }
//            
//        }
//        outfile << filenames[i] << ",";
//        for(int cv : confidenceValues)
//            outfile << cv << ",";
//        confidenceValues.clear();
//        outfile << endl;
//    }
//    
//    outfile.close();
//    return 0;
//}
//
//unordered_map<string, unordered_map<int, vector<vector<Point>>>> readLocations(string filePath){
//    unordered_map<string, unordered_map<int, vector<vector<Point>>>> res;
//    ifstream inFile;
//    inFile.open(filePath);
//    if(!inFile){
//        cerr << "Unable to open file foodlocation.txt";
//        exit(1);
//    }
//    string file;
//    while (getline(inFile, file, '\n')) {
//        string segement;
//        vector<string> substr;
//        stringstream ss(file);
//        while(getline(ss, segement, ':')){
//            substr.push_back(segement);
//        }
//        res[substr[0]] = unordered_map<int, vector<vector<Point>>>();
//        for(int i = 1; i < substr.size(); i++){
//            vector<int> loc;
//            string subloc;
//            stringstream ssloc(substr[i]);
//            while(getline(ssloc, subloc, ',')){
//                loc.push_back(stoi(subloc));
//            }
//            res[substr[0]][loc[0]] = vector<vector<Point>>();
//            vector<Point> solidFood = {Point(loc[0], loc[1]), Point(loc[2], loc[3])};
//            vector<Point> softFood = {Point(loc[4], loc[5]), Point(loc[6], loc[7])};
//            res[substr[0]][loc[0]] = {solidFood, softFood};
//        }
//    }
//    return res;
//}
//
////Function that returns the maximum of 3 integers
//int myMax(int a, int b, int c) {
//    int m = a;
//    (void)((m < b) && (m = b)); //short-circuit evaluation
//    (void)((m < c) && (m = c));
//    return m;
//}
//
////Function that returns the minimum of 3 integers
//int myMin(int a, int b, int c) {
//    int m = a;
//    (void)((m > b) && (m = b));
//    (void)((m > c) && (m = c));
//    return m;
//}
//
//void findSkin(Mat& src, Mat& dst){
//    for (int i = 0; i < dst.rows; i++) {
//        for (int j = 0; j < dst.cols; j++) {
//            //For each pixel, compute the average intensity of the 3 color channels
//            Vec3b intensity = src.at<Vec3b>(i, j); //Vec3b is a vector of 3 uchar (unsigned character)
//            int B = intensity[0]; int G = intensity[1]; int R = intensity[2];
//            if (((R > 170 && R < 210 && G > 120 && G < 170 && B > 120 && B < 170) && abs(G-B) < 10) || (R > 110 && R < 150 && G > 60 && G < 90 && B > 60 && B < 90 && abs(G-B) < 25)) {
//                dst.at<uchar>(i, j) = 255;
//            }
//        }
//    }
//}
//
//bool isSand(Vec3b point){
//    int B = point[0]; int G = point[1]; int R = point[2];
//    if (R > 100 && G > 70 && B > 50)
//        return true;
//    return false;
//}
//
//vector<Point> findSand(Mat& src){
//    vector<Point> sandArea;
//    vector<int> rowHistogram(src.rows, 0);
//    vector<int> colHistogram(src.cols, 0);
//    for(int i = 0; i < src.rows; i++){
//        for(int j = 0; j < src.cols; j++){
//            if(isSand(src.at<Vec3b>(i, j))){
//                rowHistogram[i]++;
//                colHistogram[j]++;
//            }
//        }
//    }
//    
//    int srow = 0, erow = 0;
//    // up
//    for(int i = 80; i < rowHistogram.size(); i++){
//        if(rowHistogram[i] > 350){
//            srow = i;
//            break;
//        }
//    }
//    
//    // bottom
//    for(int i =  (int)rowHistogram.size() - 100; i >= 0; i--){
//        if(rowHistogram[i] > 350){
//            erow = i;
//            break;
//        }
//    }
//    // left
//    int scol = 0, ecol = 0;
//    for(int j = 0; j < colHistogram.size(); j++){
//        if(colHistogram[j] > 200){
//            scol = j;
//            break;
//        }
//    }
//    // right
//    for(int j = (int)colHistogram.size() - 150; j >= 0; j--){
//        if(colHistogram[j] > 200){
//            ecol = j;
//            break;
//        }
//    }
//    rectangle(src, Point(scol, srow), Point(ecol, erow),  cv::Scalar(255, 0, 0));
//    sandArea = {Point(scol, srow), Point(ecol, erow)};
//    return sandArea;
//}
//
//bool isMouse(Vec3b point){
//    int B = point[0]; int G = point[1]; int R = point[2];
//    if (R < 40 && abs(R-G) < 20 && abs(G-B) < 10)
//        return true;
//    return false;
//}
//
//void findBody(Mat& src, Mat& dst, vector<Point>& sandArea){
//    for(int i = sandArea[0].x; i < sandArea[1].x; i++){
//        for(int j = sandArea[0].y; j < sandArea[1].y; j++){
//            Vec3b point = src.at<Vec3b>(j, i);
//            if(isMouse(point))
//                dst.at<uchar>(j, i) = 255;
//        }
//    }
//}
//
//void morphological(Mat& src) {
//    Mat element1 = getStructuringElement(MORPH_RECT, Size(3, 3));
//    Mat element2 = getStructuringElement(MORPH_RECT, Size(4, 4));
//    Mat element3 = getStructuringElement(MORPH_RECT, Size(5, 5));
//    medianBlur(src, src, 3);
//    //    erode(src, src, element1);
//    dilate(src, src, element2);
//    //    erode(src, src, element3);
//}
//
//pair<Rect, vector<Point>> getMouseCounter(Mat& src, Mat& frame) {
//    Mat thres_output;
//    threshold(src, thres_output, SENSITIVITY_VALUE, 255, THRESH_BINARY);
//    vector<vector<Point>> contours;
//    vector<Vec4i> hierarchy;
//    findContours(thres_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
//    
//    int maxSize = 0;
//    int maxInd = 0;
//    for(int i = 0; i < contours.size(); i++){
//        double area = contourArea(contours[i]);
//        if(area > maxSize){
//            maxSize = area;
//            maxInd = i;
//        }
//    }
//    vector<Point> contour_poly;
//    approxPolyDP(Mat(contours[maxInd]), contour_poly, 3, true);
//    Rect boundRect = boundingRect(Mat(contour_poly));
//    rectangle(frame, boundRect.tl(), boundRect.br(), Scalar(0, 0, 255), 2, 8, 0);
//    drawContours(frame, contours, maxInd, Scalar(0, 0, 255), 2, 8, hierarchy);
//    return make_pair(boundRect, contours[maxInd]);
//}
////
////void findEars(Rect& mouse, Mat& src, Mat& dst){
////    Point tl = mouse.tl();
////    Point br = mouse.br();
////    for (int i = tl.x; i < br.x; i++) {
////        for(int j = tl.y; j < br.y; j++){
////            Vec3b point = src.at<Vec3b>(i, j);
////            int B = point[0]; int G = point[1]; int R = point[2];
////            if(R - G > 30 && R - G < 50 && G - B < 15)
////                dst.at<uchar>(j, i) = 255;
////        }
////    }
////}
////
////
////void getEarsCounter(Mat& src, Mat& frame){
////    Mat thres_output;
////    threshold(src, thres_output, SENSITIVITY_VALUE, 255, THRESH_BINARY);
////    vector<vector<Point>> contours;
////    vector<Vec4i> hierarchy;
////    findContours(thres_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
////
////    vector<vector<Point> > contours_poly( contours.size() );
////    vector<Rect> boundRect( contours.size() );
////
////    for(int i = 0; i < contours.size(); i++){
////        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
////        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
////    }
////
////    for( size_t i = 0; i< contours.size(); i++ )
////    {
////        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//////        rectangle( frame, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
////    }
////}
//
//Point getHeadCentroid(Rect& mouse, vector<Point>& body, Mat& src) {
//    vector<int> widthv;
//    vector<int> heightv;
//    vector<int> xv;
//    vector<int> yv;
//    vector<Vec3b> color = {Vec3b(255, 0, 0), Vec3b(0, 255, 0), Vec3b(0, 0, 255)};
//    vector<vector<Point>> areas(3, vector<Point>());
//    
//    if(mouse.width < mouse.height){
//        heightv = {mouse.height/3, mouse.height*2/3, mouse.height};
//        yv = {mouse.y, mouse.y + heightv[0], mouse.y + heightv[1]};
//        widthv = {mouse.width, mouse.width, mouse.width};
//        xv = {mouse.x, mouse.x, mouse.x};
//    } else {
//        widthv = {mouse.width/3, mouse.width*2/3, mouse.width};
//        xv= {mouse.x, mouse.x + widthv[0], mouse.x + widthv[1]};
//        heightv = {mouse.height, mouse.height, mouse.height};
//        yv = {mouse.y, mouse.y, mouse.y};
//    }
//    
//    for(int k = 0; k < 3; k++){
//        for(int i = xv[k]; i < xv[k] + widthv[0]; i++){
//            for(int j = yv[k]; j < yv[k] + heightv[0]; j++){
//                int where = (int) pointPolygonTest(body, Point2f(i, j), false);
//                if(where >= 0){
//                    src.at<Vec3b>(j, i) = color[k];
//                    areas[k].push_back(Point(j, i));
//                }
//            }
//        }
//    }
//    
//    int minInd = 0;
//    int minArea = INT_MAX;
//    
//    for(int i = 0; i < areas.size(); i++){
//        if(areas[i].size() < minArea){
//            minArea = (int)areas[i].size();
//            minInd = i;
//        }
//    }
//    
//    // complete head contour
//    vector<Point> headContour = areas[minInd];
//    int js = yv[minInd];
//    for(; js < yv[minInd] + heightv[0]; js++){
//        if((int) pointPolygonTest(body, Point2f(xv[minInd], js), false) == 0)
//            break;
//    }
//    int je = yv[minInd] + heightv[0];
//    for (; je >= yv[minInd]; je--) {
//        if((int) pointPolygonTest(body, Point2f(xv[minInd], je), false) == 0)
//            break;
//    }
//    
//    for(int j = js; j <= je; j++)
//        headContour.push_back(Point(j, xv[minInd]));
//    
//    Point headCentroid = Point(getCentroid(headContour).y, getCentroid(headContour).x);
//    
//    // show head part
//    for(Point p : areas[minInd])
//        src.at<Vec3b>(p.x, p.y) = Vec3b(255, 255, 255);
//    
//    return headCentroid;
//}
//
////vector<vector<Point>> findTail(Mat& src, Mat& dst, vector<Point>& sandArea) {
////    vector<vector<Point>> tails;
////    for(int i = 0; i < src.rows; i++){
////        for(int j = 0; j < src.cols; j++){
////            //For each pixel, compute the average intensity of the 3 color channels
////            Vec3b intensity = src.at<Vec3b>(j, i); //Vec3b is a vector of 3 uchar (unsigned character)
////            int B = intensity[0]; int G = intensity[1]; int R = intensity[2];
////            if ((R > 150 && R < 200) && (R - B < 50 && R - B > 30) && abs(G - B) < 10) {
////                dst.at<uchar>(j, i) = 255;
////            }
////        }
////    }
////    morphological(dst);
////
////
////    Mat thres_output;
////    threshold(dst, thres_output, SENSITIVITY_VALUE, 255, THRESH_BINARY);
////    vector<vector<Point>> contours;
////    vector<Vec4i> hierarchy;
////    findContours(thres_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
////
////    int maxSize = 0;
////    int maxInd = 0;
////    for(int i = 0; i < contours.size(); i++){
////        double area = contourArea(contours[i]);
////        if(area > maxSize){
////            maxSize = area;
////            maxInd = i;
////        }
////    }
////    drawContours(src, contours, maxInd, Scalar(0, 0, 255), 2, 8, hierarchy);
////
////    return tails;
////}
//
//Point getCentroid(vector<Point>& contour){
//    Moments moment = moments(contour, false);
//    Point centroid;
//    centroid = Point(moment.m10 / moment.m00, moment.m01 / moment.m00);
//    
//    return centroid;
//}
