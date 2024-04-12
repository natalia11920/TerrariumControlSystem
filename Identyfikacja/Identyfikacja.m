
dataA=data1;
y0 = dataA(1);
dT = 30;
time = dT*(0:length(dataA)-1);
u =dataW-0.5;

%%

dane=iddata(dataA-y0,u,dT);
proc=idproc('P2');
sys_P2 = procest(dane,proc);
proc=idproc('P1D');
sys_P2D = procest(dane,proc);
sys_P2D

%%
simP2 = lsim(sys_P2,u,time') + y0;

%%
close all;
plot(time,dataA);
hold on;
plot(time,simP2);
hold on

ylabel('T(t) [°C]');
xlabel('t [min]');

sys_P2

%% drugi skok
dataA=data2;
y0 = dataA(1);
dT = 30;
time = dT*(0:length(dataA)-1);
u =dataW-0.5;

%%

dane=iddata(dataA-y0,u,dT);
proc=idproc('P2');
sys_P2 = procest(dane,proc);

%%
simP2 = lsim(sys_P2,u,time') + y0;

%%

plot(time,dataA);
hold on;
plot(time,simP2);
hold on

sys_P2


%% trzeci skok
dataA=data3;
y0 = dataA(1);
dT = 30;
time = dT*(0:length(dataA)-1);
u =dataW-0.5;

%%

dane=iddata(dataA-y0,u,dT);
proc=idproc('P2');
sys_P2 = procest(dane,proc);

%%
simP2 = lsim(sys_P2,u,time') + y0;

%%
plot(time,dataA);
hold on;
plot(time,simP2);
grid on
%xlim([1,9000])
legend( "Układ rzeczywisty, temperatura na górze", "Przybliżony model II-rzędu, temperatura na górze","Układ rzeczywisty, temperatura na środku", "Przybliżony model II-rzędu, temperatura na środku", "Układ rzeczywisty temperatura na dole", "Przybliżony model II-rzędu,temperatura na dole");

ylabel('T(t) [°C]');
xlabel('t [s]');

sys_P2
