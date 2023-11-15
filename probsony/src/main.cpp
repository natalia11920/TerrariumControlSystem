#include <Arduino.h>
#define HeatingLamp 14
int poziom;
hw_timer_t * timer1 = NULL; 
hw_timer_t * timer2 = NULL; 

void IRAM_ATTR pwmInterrupt1(){
static int sum;
if (sum<=poziom)
{
    digitalWrite(HeatingLamp,1);
}
if (sum>poziom)
{
    digitalWrite(HeatingLamp,0);
    Serial.println('.');
}
if (sum==255){
    sum=0;
}
sum+=1;
}

void IRAM_ATTR pwmInterrupt2(){
Serial.println("Mam nadzieje ze ten tez");
}

void setup(){
Serial.begin(115200);
pinMode(HeatingLamp,OUTPUT);
timer1 = timerBegin(1, 80, true);
timer2 = timerBegin(2 , 80, true);
timerAttachInterrupt(timer1,pwmInterrupt1,true);
timerAttachInterrupt(timer2,pwmInterrupt2,true);
timerAlarmWrite(timer1,0000,true);
timerAlarmWrite(timer2,30000000,true);
timerAlarmEnable(timer1);
timerAlarmEnable(timer2);
}

void loop(){
vTaskDelay(portMAX_DELAY);
}
