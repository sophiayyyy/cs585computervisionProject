data = importdata('/Users/sophia/Documents/cs585/project/project/result/Foodpreference-3dayspostmasseterparalysisC57.txt', ',', 0);
row = size(data.data);
path_in = '/Users/sophia/Documents/cs585/project/project/';
for i = 1:row
    figure(i);
    col = length(data.data(i,:));
    x = 0:1:col - 1; 
    plot(x,data.data(i, :),'.-');
    axis([0,col,-4,4]);
    yticks([-3 -2 -1 0 1 2 3]);
    xtickangle(45);
    grid on;
    set(gca,'linewidth',1,'fontsize',8,'fontname','Times');
    set(gca,'XTick',[0:100:col - 1]);
    set(gca,'YTick',[-3:1:3]);
    
    xlabel('t/s');
    ylabel('confident value');
    name = data.textdata(i,1);
    newname = name{1,1}(1,8:end-4);
    title(newname);
    
    saveas(gca,[path_in,newname],'jpg');
    close;
end