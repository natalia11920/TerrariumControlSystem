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

/*-----------------------------------------------------------------------------------------------------------------------------*/
#define DallasThermometers 15
#define Dht11Pin1 18   
#define Dht11Pin2 5 
#define CapacitiveSensor 32
#define HeatingLamp 14
#define HeatingMat 12
#define HeatingCable 33
#define Pump 27
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
int poziom=127;
//unsigned long int chanelControl = 2300632;
//unsigned long int chanelSetValue = 2334001; 

int ManualMode;
unsigned long Now=0;
unsigned long Last=0;
unsigned long Difference=0;

//Regulation
float SetTempUp=0;
float HisteresisTempUp=0.5;
float SetTempDown=0;
float HisteresisTempDown=0.5;
float SetTempMiddle=0;
float HisteresisTempMiddle=0.5;
float InteriorHumidity=5;
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

void IRAM_ATTR measureFunction(){

float outputT1=ReadingTemperature(TempAdress1);
float outputT2=ReadingTemperature(TempAdress2);
float outputT3=ReadingTemperature(TempAdress3);


if (digitalRead(HeatingLamp)==0)
{
  ThingSpeak.setField(7, 1);
}
if (digitalRead(HeatingLamp)==1)
{
  ThingSpeak.setField(7, 0);
}

ThingSpeak.writeFields(chanel, APIKey);	
}

void setup() {

Serial.begin(115200);

pinMode(HeatingCable, OUTPUT);
pinMode(HeatingLamp, OUTPUT);
pinMode(HeatingMat, OUTPUT);
pinMode(Pump, OUTPUT);

timer1 = timerBegin(1, 80, true);
timer2 = timerBegin(2 , 80, true);
timerAttachInterrupt(timer1,pwmInterrupt,true);
timerAttachInterrupt(timer2,measureFunction,true);
timerAlarmWrite(timer1,40000,true);
timerAlarmWrite(timer2,30000000,true);
timerAlarmEnable(timer1);
timerAlarmEnable(timer2);

/*
digitalWrite(HeatingCable, 1);
digitalWrite(HeatingLamp, 1);
digitalWrite(HeatingMat, 1);
digitalWrite(Pump, 1);
*/

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
  reconnect();
}
mqttClient.loop();
delay(100);
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
digitalWrite(Pump,onH1);

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



void reconnect() {

    if (mqttClient.connect("ESP32TerrariumClient")==1) {
      Serial.println("connected");
      mqttClient.subscribe(topic1);
      mqttClient.subscribe(topic2);
      mqttClient.subscribe(topic3);
      mqttClient.subscribe(topic4);
  }
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
  SetTempUp=Status;
}

if(strcmp(topic2, topic)==0)
{
  SetTempDown=Status;
}

if(strcmp(topic3, topic)==0)
{
  InteriorHumidity=Status;
}

if(strcmp(topic4, topic)==0)
{
  SetTempMiddle=Status;
}

}



int TempRelayRegulator(float SetValue, float ActualValue, float Histeresis)
{
      float Error;
      bool On;
      if (ActualValue<SetValue-Histeresis)
      {
        On=0;
      }
        if (ActualValue>SetValue+Histeresis)
      {
        On=1;
      }

    return On;
}

int HumRelayRegulator(float SetValue, float ActualValue, float Histeresis)
{
      float Error;
      bool On;
      if (ActualValue<SetValue-Histeresis)
      {
        On=0;
      }
        if (ActualValue>SetValue+Histeresis)
      {
        On=1;
      }

    return On;
}

int SoilMoistureMaintenance(float ActualValue, float SMoistureTreshold)
{
      bool On;
      if (ActualValue<=SMoistureTreshold)
      {
        On=1;
      }
        if (ActualValue>SMoistureTreshold)
      {
        On=0;
      }

      return On;
}


/*-----------------------------------------Measurements-------------------------------------------------*/

float ReadingTemperature( DeviceAddress TempSensorAdress)
{
float tempValue;
//int errTemp;
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
