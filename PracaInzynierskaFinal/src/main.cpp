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
#define Max_PWM 255
#define SAMPLE_TIME 10.0
//byte pinSDA=21;
//byte pinSCL=22;


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
volatile int level1=0;
volatile int level2=0;
volatile int level3=0;
volatile int flagM=0;
volatile int flagP=0;
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

TSstruct TempMeasurements();
/*-----------------------------------------------------------------------------------------------------------------------------*/
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

PID PIDController(PID Params, float refValue, float setValue)
{
  float Error=setValue-refValue;
  Params.sum_err=Params.sum_err+Error;
  Params.u=Params.Kp*Error+Params.Ki/Params.Ti*SAMPLE_TIME*Params.sum_err;
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
}

void SendTSData(TSstruct data)
{
  ThingSpeak.setField(1,data.tempUp);
  ThingSpeak.setField(2,data.tempMiddle);
  ThingSpeak.setField(3,data.tempDown);
  ThingSpeak.setField(4,data.hum);
  ThingSpeak.setField(5,data.humDown);
  ThingSpeak.setField(6,data.heater);
  ThingSpeak.writeFields(chanel,APIKey);
}


void SetHeaterLevel(int id, float u)
{

      if (u<0.0)
      {
          u=0.0;
      }

      if (u>1.0)
      {
        u=1.0;
      }


      int PWM=Max_PWM*u;

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
  TSstruct sensorData;
  PID Control;
  sensorData=TempMeasurements();

  float wysterowanie = 0.5;
  SetHeaterLevel(1, wysterowanie);

  sensorData.heater = wysterowanie;

  Control.PIDController(sensorData.tempUp,SetTemp1);
  SendTSData(sensorData);




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
}

/*-----------------------------------------------------------------------------------------------------------------------------*/

TSstruct TempMeasurements()
{
  TSstruct data;
  TempSensors.requestTemperatures();
  data.tempUp=TempSensors.getTempC(TempAdress2);
  data.tempMiddle=TempSensors.getTempC(TempAdress1);
  data.tempDown=TempSensors.getTempC(TempAdress3);
  data.hum=HumSensor1.readHumidity();
  data.humDown=ReadingMoisture();
  data.heater=0;
  return data;
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


/*-----------------------------------------Measurements-------------------------------------------------*/


float ReadingMoisture()
{
  int SoilMoisture,SoilDry=4095,SoilWet=2512;
  float SoilPercentege;
  SoilMoisture=analogRead(CapacitiveSensor);
  SoilPercentege=map(SoilMoisture,SoilWet,SoilDry,100,0);

  return SoilPercentege;
}
