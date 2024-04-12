#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <PubSubClient.h>
#include "structury.h"


#define DallasThermometers 15
#define Dht22Pin 5   
#define CapacitiveSensor 32
#define HeatingLamp 14
#define HeatingMat 12
#define HeatingMatDown 33
#define Humidifier 27
#define Max_PWM 255
#define U_max 1
#define U_min 0


const char* ssid = "" ;
const char* pass ="";
const char* APIKey= "";
const char* mqttServer="test.mosquitto.org";
const char* topic1="";
const char* topic2="";
const char* topic3="";
const char* topic4="";
const char* topic5="";
unsigned int mqttPort=1883;
unsigned long int chanel = 2279857;
volatile int level1=127;
volatile int level2=127;
volatile int level3=127;
volatile int flagM=0;
volatile int flagP=0;
int Mode=0;

float SetTemp1=0;
float SetTemp2=0;
float SetTemp3=0;
float SetHum=0.0;

hw_timer_t * timer1 = NULL; 
hw_timer_t * timer2 = NULL; 
WiFiClient client;
PubSubClient mqttClient(client);
OneWire OneWireBus(DallasThermometers);
DallasTemperature TempSensors(&OneWireBus);
DHT HumSensor1(Dht22Pin, DHT22);
PIRegulation ParametersT1;
PIRegulation ParametersT2;
PIRegulation ParametersT3;


DeviceAddress TempAdress2={0x28, 0x94, 0x49, 0x38, 0x80, 0x22, 0xB, 0x2F};
DeviceAddress TempAdress3={ 0x28, 0x3C, 0xE1, 0x7F, 0x80, 0x22, 0xB, 0xB9};
DeviceAddress TempAdress1={0x28, 0xC6, 0x3E, 0xBE, 0x80, 0x22, 0xB, 0x83};

float ReadingMoisture();
bool RelayRegulator(float,float);
void ReadMqtt(char*, byte*, unsigned int );
void reconnect();
TSstruct Measurements();
void SendTSData(TSstruct );
void SetHeaterLevel(int, float);

void IRAM_ATTR pwmInterrupt(){
  static int sum=0;
  
  digitalWrite(HeatingLamp,sum>level1);
  digitalWrite(HeatingMat,sum>level2);
  digitalWrite(HeatingMatDown,sum>level3);
  
  if (sum >= Max_PWM){
    sum=0;
  }
  sum += 1;
}

void IRAM_ATTR measureFunction(){
  flagM=1;
}

void setup() {

Serial.begin(115200);

pinMode(HeatingMatDown, OUTPUT);
pinMode(HeatingLamp, OUTPUT);
pinMode(HeatingMat, OUTPUT);
pinMode(Humidifier, OUTPUT);

digitalWrite(HeatingMatDown, 1);
digitalWrite(HeatingLamp, 1);
digitalWrite(HeatingMat, 1);
digitalWrite(Humidifier, 1);

WiFi.mode(WIFI_STA);
WiFi.begin(ssid, pass);
 while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

mqttClient.setServer(mqttServer,mqttPort);
mqttClient.setCallback(ReadMqtt);
mqttClient.subscribe(topic1,2);
mqttClient.subscribe(topic2,2);
mqttClient.subscribe(topic3,2);
mqttClient.subscribe(topic4,2);
mqttClient.subscribe(topic5,2);

timer1 = timerBegin(1, 80, true);
timer2 = timerBegin(2, 80, true);
timerAttachInterrupt(timer1,pwmInterrupt,true);
timerAttachInterrupt(timer2,measureFunction,true);
timerAlarmWrite(timer1,40000,true);
timerAlarmWrite(timer2,30000000,true);
timerAlarmEnable(timer1);
timerAlarmEnable(timer2);


ThingSpeak.begin(client);
TempSensors.begin();
HumSensor1.begin();



ParametersT1.Kp = 0.167;
ParametersT1.Ti = 900;
ParametersT2.Kp = 0.40;
ParametersT2.Ti = 750;
ParametersT3.Kp = 0.43;
ParametersT3.Ti = 600;

ParametersT1.w1=0.8497;
ParametersT1.w2=-0.4629;
ParametersT1.w3=-0.2525;

ParametersT2.w1=-0.1007;
ParametersT2.w2=0.9874;
ParametersT2.w3=-0.1221;

ParametersT3.w1=-0.0693;
ParametersT3.w2=-0.1204;
ParametersT3.w3=0.9903;

}



void loop() {

if(WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_DISCONNECTED){
    WiFi.reconnect();
 }

if (!mqttClient.connected()) {
      if(mqttClient.connect("ESP32TerrariumClient")==1)
     {
      mqttClient.subscribe(topic1);
      mqttClient.subscribe(topic2);
      mqttClient.subscribe(topic3);
      mqttClient.subscribe(topic4);
      mqttClient.subscribe(topic5);
      }

}
mqttClient.loop();


if (flagM==1)
{
  TSstruct sensorData;
  sensorData=Measurements();
  
if (Mode==1){
  digitalWrite(Humidifier,1);
  if (SetTemp1!=0)
  {
      ParametersT1.PIControl(SetTemp1,sensorData.tempUp);
      SetHeaterLevel(1, ParametersT1.u1);
      sensorData.heater1 = ParametersT1.u1;
      SetHeaterLevel(2, ParametersT1.u2);
      sensorData.heater2 = ParametersT1.u2;
      SetHeaterLevel(3, ParametersT1.u3);
      sensorData.heater3 = ParametersT1.u3;
  }

  if (SetTemp2!=0)
  {
      ParametersT2.PIControl(SetTemp2,sensorData.tempMiddle);  
      SetHeaterLevel(1, ParametersT2.u1);
      sensorData.heater1 = ParametersT2.u1;
      SetHeaterLevel(2, ParametersT2.u2);
      sensorData.heater2 = ParametersT2.u2;
      SetHeaterLevel(3, ParametersT2.u3);
      sensorData.heater3 = ParametersT2.u3;
  }
  if (SetTemp3!=0)
  {
      ParametersT3.PIControl(SetTemp3,sensorData.tempDown);
      SetHeaterLevel(1, ParametersT3.u1);
      sensorData.heater1 = ParametersT3.u1;
      SetHeaterLevel(2, ParametersT3.u2);
      sensorData.heater2 = ParametersT3.u2;
      SetHeaterLevel(3, ParametersT3.u3);
      sensorData.heater3 = ParametersT3.u3;
  }

  if (SetTemp1==0 && SetTemp2==0 && SetTemp3==0)
  {
      SetHeaterLevel(1, 0.5);
      sensorData.heater1 = 0.5;
      SetHeaterLevel(2, 0.5);
      sensorData.heater2 = 0.5;
      SetHeaterLevel(3, 0.5);
      sensorData.heater3 = 0.5;
  }
  }

  if (Mode==2){

      SetHeaterLevel(1, 0.5);
      sensorData.heater1 = 0.5;
      SetHeaterLevel(2, 0.5);
      sensorData.heater2 = 0.5;
      SetHeaterLevel(3, 0.5);
      sensorData.heater3 = 0.5;

      bool RelayStatus=RelayRegulator(sensorData.hum, 5.0);
      digitalWrite(Humidifier,!RelayStatus);
      sensorData.heater3=RelayStatus;

  }

  if (Mode!=1 && Mode!=2){

      SetHeaterLevel(1, 0.5);
      sensorData.heater1 = 0.5;
      SetHeaterLevel(2, 0.5);
      sensorData.heater2 = 0.5;
      SetHeaterLevel(3, 0.5);
      sensorData.heater3 = 0.5;

      digitalWrite(Humidifier,1);
      
}
  
  SendTSData(sensorData);
  flagM=0;

}
}




void SendTSData(TSstruct data)
{
  ThingSpeak.setField(1,data.tempUp);
  ThingSpeak.setField(2,data.tempMiddle);
  ThingSpeak.setField(3,data.tempDown);
  ThingSpeak.setField(4,data.hum);
  ThingSpeak.setField(5,data.humDown);
  ThingSpeak.setField(6,data.heater1);
  ThingSpeak.setField(7,data.heater2);
  ThingSpeak.setField(8,data.heater3);
  ThingSpeak.writeFields(chanel,APIKey);
}


void SetHeaterLevel(int id, float U)
{

      if (U<U_min)
      {
          U=U_min;
      }

      if (U>U_max)
      {
        U=U_max;
      }


      int PWM=Max_PWM*U;

      switch (id)
      {
      case 1: 
        level1=PWM;
        break;
      case 2:
        level2=PWM;
        break;
      case 3:
        level3=PWM;
        break;
      }

}

void ReadMqtt(char* topic, byte* sign, unsigned int length) {

      int Status,i;
      String MessageString;
      i=0;
      while ( i < length) {
        MessageString.concat((char)sign[i]);
        i++;
      };

      Status=MessageString.toInt();
      if(strcmp(topic1, topic)==0 )
      {
        SetTemp1=Status;
      }

      if(strcmp(topic2, topic)==0)
      {
        SetTemp3=Status;
      }

      if(strcmp(topic3, topic)==0)
      {
        SetHum=Status; 
      }

      if(strcmp(topic4, topic)==0)
      {
        SetTemp2=Status;
      }

      if(strcmp(topic5, topic)==0)
      {
        Mode=Status;
      }
}

TSstruct Measurements()
{
  TSstruct data;
  TempSensors.requestTemperatures();
  static float T1_prev,T2_prev,T3_prev;
  float T1=TempSensors.getTempC(TempAdress2);
    if (T1!=DEVICE_DISCONNECTED_C)
    { 
      T1_prev=T1;
      data.tempUp=T1;
    }
    else
    {
      data.tempUp=T1_prev;
    }

    float T2=TempSensors.getTempC(TempAdress1);
    if (T2!=DEVICE_DISCONNECTED_C)
    {
      T2_prev=T2;
      data.tempMiddle=T2;
    }
    else
    {
      data.tempMiddle=T2_prev;
    }

    float T3=TempSensors.getTempC(TempAdress3);
    if (T3!=DEVICE_DISCONNECTED_C)
    {
      T3_prev=T3;
      data.tempDown=T3;
    }
    else
    {
      data.tempDown=T3_prev;
    }
  
  data.hum=HumSensor1.readHumidity();
  data.humDown=ReadingMoisture();
  data.heater1=0;
  data.heater2=0;
  data.heater3=0;
  return data;
}


float ReadingMoisture()
{
  int SoilMoisture;
  float SoilDry=2710,SoilWet=993,maxOut=100,minOut=0;
  float SoilPercentege;
  SoilMoisture=analogRead(CapacitiveSensor);
  float a=(maxOut-minOut)/(SoilWet-SoilDry);
  float b=(minOut*SoilWet-maxOut*SoilDry)/(SoilWet-SoilDry);
  SoilPercentege=round(a*SoilMoisture+b);
  return SoilPercentege;
}


bool RelayRegulator(float ActualValue, float Histeresis)
{
      static bool On=0;
      if (ActualValue<SetHum-Histeresis)
      {
        On=1;
      }
        if (ActualValue>SetHum+Histeresis)
      {
        On=0;
      }

    return On;
}
