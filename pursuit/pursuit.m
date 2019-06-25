data = importdata('/Users/sophia/Documents/cs585/pursuit/pursuit/result/pursuit -14 days post TN C57-5.txt', ',', 0);
row = size(data.data);
path_in = '/Users/sophia/Documents/cs585/pursuit/pursuit/';
for i = 1:row
    figure(i);
    col = length(data.data(i,:));
    x = 0:1:col - 1;
    x1 = x * 0.0333;
    plot(x1,data.data(i, :),'.-');
    axis([0,col * 0.03,0,4]);
    yticks([0 1 2 3 4]);
    xtickangle(45);
    grid on;
    set(gcf,'unit','normalized','position',[0.2,0.2,0.64,0.32]);
    set(gca,'linewidth',1,'fontsize',8,'fontname','Times');
    set(gca,'XTick',[0:100:col * 0.03]);
    set(gca,'YTick',[0:1:4]);
    
    xlabel('t/s');
    ylabel('pursuit confident value');
    name = data.textdata(i,1);
    newname = name{1,1}(1,11:end-4);
    title(newname);
    
    saveas(gca,[path_in,newname],'jpg');
    close;
end