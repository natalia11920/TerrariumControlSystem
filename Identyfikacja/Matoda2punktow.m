%%

data1=data1(32:300)
[m,n]=size(data1);
y0=data1(1);
deltay=data1(m)-data1(1)
y1=0.632*deltay+y0;
y2=0.282*deltay+y0;
for i = 1:length(data1)
    if( data1(i) < y1+0.0625 && data1(i) > y1-0.0625)
    t2=(i-1)*30;
    end
end

for i = 1:length(data1)
    if( data1(i) < y2+0.0625 && data1(i) > y2-0.0625)
    t1=(i-1)*30;
    end
end
T1=1.5*(t2-t1)
tau=t2-T1
k=deltay/0.5;
num = k;
den = [T1 1];
P = tf(num,den,'InputDelay',tau)
time = 30*(0:length(data1)-1);
figure(1)
yline(y1, 'c')
hold on
yline(y2, 'm')
hold on
step(0.5*P+y0)
hold on
plot(time,data1,'g')
hold off
kr1=0.9*T1/(k*tau)
 Ti1=3.33*tau 
 grid on

xlabel('t ')
ylabel('T(t) (°C)')
legend("63.2%", "28.3%", "Układ przybliżony", "Układ rzeczywisty" )

%%

% data3=data3(45:300)
[m1,n1]=size(data3);
y0=data3(1);
deltay=data3(m1)-data3(1)
y1=0.632*deltay+y0;
y2=0.283*deltay+y0;
for i = 1:length(data3)
    if( data3(i) < y1+0.0625 && data3(i) > y1-0.0625)
    t2=(i-1)*30;
    disp('szukam')
    end
end

for i = 1:length(data3)
    if( data3(i) < y2+0.0625 && data3(i) > y2-0.0625)
    t1=(i-1)*30;
    end
end
T1=1.5*(t2-t1)
tau=t2-T1
k=deltay/0.5;
num = k;
den = [T1 1];
G = tf(num,den,'InputDelay',tau)
time = 30*(0:length(data3)-1);
figure(2)
yline(y1, 'c')
hold on
yline(y2, 'm')
hold on
hold on
plot(time,data3,'g')
hold on
step(0.5*G+y0)
hold off
grid on
kr3=0.9*T1/(k*tau)
Ti3=3.33*tau
xlabel('t ')
ylabel('y(t) (°C)')
legend("63.2%", "28.3%", "Układ przybliżony", "Układ rzeczywisty" )

%%

data2=data2(13:end);
[m2,n2]=size(data2);
y0=data2(1);
deltay=data2(m2)-data2(1)
y1=0.632*deltay+y0;
y2=0.283*deltay+y0;
for i = 13:length(data2)
    if( data2(i) < y1+0.0625 && data2(i) > y1-0.0625)
    t2=(i-1)*30;
    disp('szukam')
    end
end

for i = 13:length(data2)
    if( data2(i) < y2+0.0625 && data2(i) > y2-0.0625)
    t1=(i-1)*30;
    end
end
T1=1.5*(t2-t1)
tau=t2-T1
k=deltay/0.5;
num = k;
den = [T1 1];
K = tf(num,den,'InputDelay',tau)
time = 30*(0:length(data2)-1);
figure(3)
yline(y1, 'c')
hold on
yline(y2, 'm')
hold on
plot(time,data2,'g')
hold on
step(0.5*K+y0)
hold off
kr2=0.9*T1/(k*tau)
Ti2=3.33*tau
grid on
xlabel('t')
ylabel('y(t) [°C]')
legend("63.2%", "28.3%", "Układ przybliżony", "Układ rzeczywisty" )

