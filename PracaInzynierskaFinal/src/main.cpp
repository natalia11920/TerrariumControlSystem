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


#define DallasThermometers 15
#define Dht22Pin 5   
#define CapacitiveSensor 32
#define HeatingLamp 14
#define HeatingMat 12
#define HeatingCable 33
#define Pump 27
#define Max_PWM 255
#define U_max 1
#define U_min 0


const char* ssid = "2.4G-Vectra-WiFi-224A24" ;
const char* pass ="ez0zkxdqnzkfydj1";
const char* APIKey= "W55RWPVWCLJOIC82";
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
volatile int flagM=0;
volatile int flagP=0;
int T01=34;
int T02=25;
int T03=26;

float SetTemp1=0.0;
float SetTemp2=0.0;
float SetTemp3=0.0;
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
int RTCAdress=0x64;

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
  digitalWrite(HeatingCable,sum>level3);
  
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
Wire.begin();
//SetTime();

ParametersT1.Kp = 0.2135;
ParametersT1.Ti = 767.232;
ParametersT2.Kp = 0.5899;
ParametersT2.Ti = 469.6299;
ParametersT3.Kp = 0.6398;
ParametersT3.Ti = 453.8457;
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
      }

}
mqttClient.loop();


if (flagM==1)
{
  TSstruct sensorData;
  sensorData=Measurements();
 ParametersT1.PIControl(SetTemp1-T01,sensorData.tempUp-T01);
  ParametersT2.PIControl(SetTemp2-T02,sensorData.tempMiddle-T02);
  ParametersT3.PIControl(SetTemp3-T03,sensorData.tempDown-T03);
  SetHeaterLevel(1, ParametersT1.u);
  sensorData.heater1 = ParametersT1.u;
  SetHeaterLevel(2, ParametersT2.u);
  sensorData.heater2 = level2;
  SetHeaterLevel(3, ParametersT3.u);
  sensorData.heater3 = level3;
 /*if (level1==127)
  {
      sensorData.heater1 = 0.5;
  }
  else
  {
      sensorData.heater1 = 1;
  }
  if (level2==127)
  {
      sensorData.heater2 = 0.5;
  }
  else
  {
      sensorData.heater2 = 1;
  }

    if (level3==127)
  {
      sensorData.heater3 = 0.5;
  }
  else
  {
      sensorData.heater3 = 1;
  }*/
  bool RelayStatus=RelayRegulator(sensorData.hum, 5.0);
  digitalWrite(Pump,!RelayStatus);
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

     /* if (U<U_min)
      {
          U=U_min;
      }

      if (U>U_max)
      {
        U=U_max;
      }*/


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

void ReadMqtt(char* topic, byte* payload, unsigned int length) {

      int Status;
      String MessageString;
        for (int i = 0; i < length; i++) {
          MessageString.concat((char)payload[i]);
        }
      Status=MessageString.toInt();
      if(strcmp(topic1, topic)==0 )
      {
        SetTemp1=Status;
        //level1=Status;
      }

      if(strcmp(topic2, topic)==0)
      {
        SetTemp3=Status;
        //level3=Status;
      }

      if(strcmp(topic3, topic)==0)
      {
        SetHum=Status;
        
      }

      if(strcmp(topic4, topic)==0)
      {
        SetTemp2=Status;
        //level2=Status;
      }
}

TSstruct Measurements()
{
  TSstruct data;
  TempSensors.requestTemperatures();
  data.tempUp=TempSensors.getTempC(TempAdress2);
  data.tempMiddle=TempSensors.getTempC(TempAdress1);
  data.tempDown=TempSensors.getTempC(TempAdress3);
  data.hum=HumSensor1.readHumidity();
  data.humDown=ReadingMoisture();
  data.heater1=0;
  data.heater2=0;
  data.heater3=0;
  return data;
}


float ReadingMoisture()
{
  int SoilMoisture,SoilDry=4095,SoilWet=2512;
  float SoilPercentege;
  SoilMoisture=analogRead(CapacitiveSensor);
  SoilPercentege=map(SoilMoisture,SoilWet,SoilDry,100,0);

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
