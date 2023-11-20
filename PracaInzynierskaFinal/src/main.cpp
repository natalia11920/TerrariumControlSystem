#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <time.h>
#include <Wire.h>
#include <PubSubClient.h>

#include "structury.h"

/*-----------------------------------------------------------------------------------------------------------------------------*/
#define DallasThermometers 15
#define Dht11Pin1 18   
#define Dht11Pin2 5 
#define CapacitiveSensor 32
#define HeatingLamp 14
#define HeatingMat 12
#define HeatingCable 33
#define Pump 27

#define SAMPLE_TIME 10.0
//byte pinSDA=21;
//byte pinSCL=22;


const char* ssid = "2.4G-Vectra-WiFi-224A24" ;
const char* pass ="ez0zkxdqnzkfydj1";
const char* APIKey= "W55RWPVWCLJOIC82";
//const char* APIKeyControl= "Z3F8BLN1OO93OWM8";
//const char* APISetValue="YGJM8QNYFYNKCZFO";
const char* mqttServer="test.mosquitto.org";
const char* topic1="BulbTerr_01";
const char* topic2="CableTerr_01";
const char* topic3="PumpTerr_01";
const char* topic4="MatTerr_01";
unsigned int mqttPort=1883;
unsigned long int chanel = 2279857;
volatile int level1=127;
volatile int level2=127;
volatile int level3=127;
int level1H=0;
int level2H=0;
int level3H=0;
//unsigned long int chanelControl = 2300632;
//unsigned long int chanelSetValue = 2334001; 

volatile int flagM=0;
int flagP=0;
unsigned long Now=0;
unsigned long Last=0;
unsigned long Difference=0;

//Regulation
float SetTemp1=0;
float SetTemp2=0;
float SetTemp3=0;
float SetHum=0;
float HumidityHisteresis=5;
float SoilMoistureTreshold=10;


hw_timer_t * timer1 = NULL; 
hw_timer_t * timer2 = NULL; 
WiFiClient client;
PubSubClient mqttClient(client);
OneWire OneWireBus(DallasThermometers);
DallasTemperature TempSensors(&OneWireBus);
DHT HumSensor1(Dht11Pin1, DHT11);
DHT HumSensor2(Dht11Pin2, DHT11);

DeviceAddress TempAdress2={0x28, 0x94, 0x49, 0x38, 0x80, 0x22, 0xB, 0x2F};
DeviceAddress TempAdress3={ 0x28, 0x3C, 0xE1, 0x7F, 0x80, 0x22, 0xB, 0xB9};
DeviceAddress TempAdress1={0x28, 0xC6, 0x3E, 0xBE, 0x80, 0x22, 0xB, 0x83};
int RTCAdress=0x64;

float ReadingTemperature(DeviceAddress);
float ReadingHumidity(int);
float ReadingMoisture();
int TempRelayRegulator(float, float, float);
int HumRelayRegulator(float, float, float);
int SoilMoistureMaintenance(float, float);
void SetTime();
void ReadMqtt(char*, byte*, unsigned int );
void reconnect();

/*-----------------------------------------------------------------------------------------------------------------------------*/
void IRAM_ATTR pwmInterrupt(){
  static int sum=0;
  
  digitalWrite(HeatingLamp,sum>level1);
  digitalWrite(HeatingMat,sum>level2);
  digitalWrite(HeatingCable,sum>level3);
  
  if (sum >= 255){
    sum=0;
  }
  sum += 1;
}

void IRAM_ATTR measureFunction(){
  flagM=1;
}

void setup() {

Serial.begin(115200);

pinMode(HeatingCable, OUTPUT);
pinMode(HeatingLamp, OUTPUT);
pinMode(HeatingMat, OUTPUT);
pinMode(Pump, OUTPUT);

digitalWrite(HeatingCable, 1);
digitalWrite(HeatingLamp, 1);
digitalWrite(HeatingMat, 1);
digitalWrite(Pump, 1);

WiFi.mode(WIFI_STA);
WiFi.begin(ssid, pass);
 while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("...");
  }

Serial.println("WiFi network established");

mqttClient.setServer(mqttServer,mqttPort);
mqttClient.setCallback(ReadMqtt);
mqttClient.subscribe(topic1,2);
mqttClient.subscribe(topic2,2);
mqttClient.subscribe(topic3,2);
mqttClient.subscribe(topic4,2);

timer1 = timerBegin(1, 80, true);
timer2 = timerBegin(2 , 80, true);
timerAttachInterrupt(timer1,pwmInterrupt,true);
timerAttachInterrupt(timer2,measureFunction,true);
timerAlarmWrite(timer1,40000,true);
timerAlarmWrite(timer2,30000000,true);
timerAlarmEnable(timer1);
timerAlarmEnable(timer2);


ThingSpeak.begin(client);
TempSensors.begin();
HumSensor1.begin();
HumSensor2.begin();
Wire.begin();
//SetTime();
}


void loop() {

if(WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_DISCONNECTED){
    WiFi.reconnect();
 }

if (!mqttClient.connected()) {
      if(mqttClient.connect("ESP32TerrariumClient")==1)
     { Serial.println("connected");
      mqttClient.subscribe(topic1);
      mqttClient.subscribe(topic2);
      mqttClient.subscribe(topic3);
      mqttClient.subscribe(topic4);
      }

}
mqttClient.loop();



if (flagM==1)
{
   
float outputT1=ReadingTemperature(TempAdress1);
float outputT2=ReadingTemperature(TempAdress2);
float outputT3=ReadingTemperature(TempAdress3);

if (mqttClient.connect("ESP32TerrariumClient")==1)
{
if (level3!=0)
{
  ThingSpeak.setField(7, 1);
}
if (level3==0)
{
  ThingSpeak.setField(7, 0);
}

int err=ThingSpeak.writeFields(chanel, APIKey);
Serial.println(millis());
Serial.println(err);
}	
flagM=0;
}

 //Wire.beginTransmission(RTCAdress);
 //Wire.requestFrom(RTCAdress, 7);
 //int b=Wire.available();
 //byte Time[7];
 //for (int i=0; i<7; i++)
 //{
  //Time[i]=Wire.read();
  //Serial.println(Time[i]);
 //}

  //Wire.endTransmission();
//delay(1000);
//delay(1000);

/*----------------------------temperature regulation-----------------------------------------*/
/*bool onT1 = TempRelayRegulator(SetTempUp, outputT3, HisteresisTempUp);
digitalWrite(HeatingLamp,onT1);

bool onT2 = TempRelayRegulator(SetTempDown, outputT2, HisteresisTempDown);
  digitalWrite(HeatingCable,onT2);

bool onT3 = TempRelayRegulator(SetTempMiddle, outputT1, HisteresisTempMiddle);
digitalWrite(HeatingMat,onT3);*/

/*
float outputH1= ReadingHumidity(Dht11Pin1);
delay(2000);
float outputH2=ReadingHumidity(Dht11Pin2);
delay(2000); 

/*--------------------------------humidity regulation-----------------------------------------*/
/*float AverageHum=(outputH1+outputH2)/2;
bool onH1 = HumRelayRegulator(InteriorHumidity, AverageHum, HumidityHisteresis);
digitalWrite(Pump,!onH1);

float outputS1=ReadingMoisture();
//bool onS1 = SoilMoistureMaintenance(outputS1, SoilMoistureTreshold);
//digitalWrite(Pump,onS1);
delay(1000);
if (digitalRead(HeatingLamp)==0)
{
  ThingSpeak.setField(7, 1);
}
if (digitalRead(HeatingLamp)==1)
{
  ThingSpeak.setField(7, 0);
}

if (digitalRead(HeatingCable)==0)
{
  ThingSpeak.setField(8, 1);
}
if (digitalRead(HeatingCable)==1)
{
  ThingSpeak.setField(8, 0);
}

delay(500);*/
}

/*-----------------------------------------------------------------------------------------------------------------------------*/


void SetTime()
{
  byte Data[7]={0, 5, 18, 5, 0, 11, 11};//{seconds,minutes,hours,day,date,month,year};
Wire.beginTransmission(RTCAdress);
 //Wire.write(0);
for (int i=0; i<7;i++)
{
Wire.write(Data[i]);
}
Wire.endTransmission();

}



void ReadMqtt(char* topic, byte* payload, unsigned int length) {

int Status;
String MessageString;
  for (int i = 0; i < length; i++) {
    MessageString.concat((char)payload[i]);
  }
Status=MessageString.toInt();
Serial.println(Status);
Serial.println(topic);

if(strcmp(topic1, topic)==0 )
{
  level1=Status;
}

if(strcmp(topic2, topic)==0)
{
  level3=Status;
}

if(strcmp(topic3, topic)==0)
{
  SetHum=Status;
}

if(strcmp(topic4, topic)==0)
{
  level2=Status;
}
}


int HumRelayRegulator(float SetValue, float ActualValue, float Histeresis)
{
      static bool On=0;
      if (ActualValue<SetValue-Histeresis)
      {
        On=1;
      }
        if (ActualValue>SetValue+Histeresis)
      {
        On=0;
      }

    return On;
}

int SoilMoistureMaintenance(float ActualValue, float SMoistureTreshold)
{
      return (ActualValue<=SMoistureTreshold);
}


/*-----------------------------------------Measurements-------------------------------------------------*/

float ReadingTemperature( DeviceAddress TempSensorAdress)
{
float tempValue;
TempSensors.requestTemperatures();
tempValue=TempSensors.getTempC(TempSensorAdress);
if (TempSensorAdress==TempAdress2) 
{
Serial.print("Temperature on the top of the terrarium:");
Serial.println(tempValue);
Serial.println("ºC");
ThingSpeak.setField(1, tempValue);
}
if (TempSensorAdress==TempAdress3)
{
Serial.print("Temperature on the bottom of the terrarium:");
Serial.println(tempValue);
Serial.println("ºC");
ThingSpeak.setField(3, tempValue);
}
if (TempSensorAdress==TempAdress1)
{
  tempValue=TempSensors.getTempC(TempAdress1);
Serial.print("Temperature in the middle of the terrarium:");
Serial.println(tempValue);
Serial.println("ºC");
ThingSpeak.setField(2, tempValue);
}

return tempValue;
}


float ReadingHumidity(int dhtPin)
{
  float humidityValue;
  //int humErr;
if(dhtPin==Dht11Pin1){
  humidityValue=HumSensor1.readHumidity();
  Serial.print("Humidity on the top of the terrarium: ");
  Serial.print(humidityValue);
  Serial.println("%");
  ThingSpeak.setField(4, humidityValue);
}
if(dhtPin==Dht11Pin1){

  humidityValue=HumSensor2.readHumidity();
  Serial.print("Humidity on the bottom of the terrarium: ");
  Serial.print(humidityValue);
  Serial.println("%");
  ThingSpeak.setField(5, humidityValue);
}
  return humidityValue;
}


float ReadingMoisture()
{
  int SoilMoisture,SoilDry=4095,SoilWet=2512;
  float SoilPercentege;
  SoilMoisture=analogRead(CapacitiveSensor);
  Serial.print("Soil moisture: ");
  Serial.println(SoilMoisture);
  Serial.print("Percentege soil moisture: ");
  SoilPercentege=map(SoilMoisture,SoilWet,SoilDry,100,0);
  Serial.print(SoilPercentege);
  Serial.println("%");
  ThingSpeak.setField(6, SoilPercentege);

  return SoilPercentege;
}

void Control1(float SetValue, float ActualValue, int TempFlag)
{
  float deltaTemp1=24.2, deltaTemp2=9, deltaTemp3=9, Error;
  if (TempFlag==1)
  {
      Error=SetValue-ActualValue;
      level1H=round(map(Error,0,deltaTemp1,0,255));

      if (Error!=0)
      {
        level1=level1+level1H;

        if (level1>255)
        {
          level1=255;
        }
        if (level1<0)
        {
          level1=0;
        }
      }
  }

  if (TempFlag==2)
  {
      Error=SetValue-ActualValue;
      level1H=round(map(Error,0,deltaTemp1,0,255));

      if (Error!=0)
      {
        level2=level2+level2H;

        if (level2>255)
        {
          level2=255;
        }
        if (level2<0)
        {
          level2=0;
        }
      }
  }

  if (TempFlag==3)
  {
      Error=SetValue-ActualValue;
      level1H=round(map(Error,0,deltaTemp1,0,255));

      if (Error!=0)
      {
        level3=level3+level3H;

        if (level3>255)
        {
          level3=255;
        }
        if (level3<0)
        {
          level3=0;
        }
      }
  }

}

float PIDController(float actualValue, float setValue)
{
  static float sum_error;
  
  float Error,up,ui,u,kp,Ti,Tp;
  Error=setValue-actualValue;
  sum_error=sum_error+Error;
  up=kp*Error;
  ui=kp/Ti*SAMPLE_TIME*sum_error;
  u=up+ui;
  return u;
}