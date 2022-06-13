%--------------初态到抓取点--------------------
mdl_puma560 %加载内设的机械臂模型
p = [0.8 0 -0.2];% 球的空间位置
T = transl(p) * troty(pi/2);%到抓取点的变换阵，旋转阵用于末端位置的调整
q1 = [0,pi/2,-pi/2,0,0,-pi/2];%初始状态，必须给初始状态，否则jtraj会报错
%p560.plot(q1);%查看手臂当前状态
qqr = p560.ikine6s(T, 'ru');% 逆运动学求解，设定的是抓取的姿态，不一定是最优的
qqr(6)=1.2;%设定抓手姿态
qrt = jtraj(q1, qqr,20);  % 计算从q1到qqr的空间轨迹，经过20步
ae = [-132 24];%视角调整
p(1) = 1;
clf%清除当前图像窗口
p = [0.8055 0 0];
plot_sphere(p, 0.045, 'b');%在p点处摆放一个小球，大小0.045
plotcube([0.1 0.1 0.1],[-1-0.04 -0.04 -0.1],.8,[1 1 1]);%放置台的位置
%plotcube([0.1 0.1 0.1],[0.75 -0.05 -0.15],.8,[1 0 1]);%起始台的位置
p560.plot3d(qrt, 'view', ae, 'nowrist');  % 轨迹绘制
msgbox("已抓到目标（小球变红）");
%--------------抓取点到放置点-----------------
qr =[19*pi/18,0.48,-2.4,0.5,0.4,1.2];%放置点机械臂姿态，示教得到
qrt = jtraj(qqr, qr, 20);  % 从抓取处到最终点的路径逆解
clf
for i=1:length(qrt)
    T = p560.fkine(qrt(i,:)); % 正运动学求解
    Tball = T * SE3(0,0,0.2); %抓手点到球重心的变换
    P = Tball.t;%球的坐标
    clf%必须清屏，否则会显示小球的轨迹
    plot_sphere(P, 0.045, 'r');%小球的运动轨迹
    plotcube([0.1 0.1 0.1],[-1-0.04 -0.04 -0.1],.8,[1 1 1]);%放置台的位置
    p560.plot3d(qrt(i,:), 'view', ae, 'nowrist');
    %pause(0.01)
end
msgbox("目标已放置（小球变蓝）");
%---------------放置点到初态--------------------
qrr=[0,pi/2,-pi/2,0,0,-pi/2];
qrt1 = jtraj(qr, qrr, 20);  % 从放置点到初始位置的路径逆解
pause(0.5)
for i=1:length(qrt1)
    T = p560.fkine(qrt1(i,:)); % 正运动学求解
    clf
    plot_sphere(P, 0.045, 'b');%小球位置固定
    plotcube([0.1 0.1 0.1],[-1-0.04 -0.04 -0.1],.8,[1 1 1]);
    p560.plot3d(qrt1(i,:), 'view', ae, 'nowrist');
    %pause(0.01)
end
msgbox("本次抓取已完成");
