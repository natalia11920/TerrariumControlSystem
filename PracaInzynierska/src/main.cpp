#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <time.h>
#include <WiFiUdp.h>
#include <Wire.h>

#define countof(a) (sizeof(a) / sizeof(a[0]))
#define DallasThermometers 15
#define Dht11Pin1 18   
#define Dht11Pin2 5 
#define CapacitiveSensor 32
#define HeatingLamp 14
#define HeatingMat 33
#define HeatingCable 12
#define Pump 27
#define PWMPin 13



const char* ssid = "2.4G-Vectra-WiFi-224A24" ;
const char* pass ="ez0zkxdqnzkfydj1";
const char* APIKey= "W55RWPVWCLJOIC82";
const char* APIKeyControl= "Z3F8BLN1OO93OWM8";
const char* APISetValue="YGJM8QNYFYNKCZFO";
unsigned long int chanel = 2279857;
unsigned long int chanelControl = 2300632;
unsigned long int chanelSetValue = 2334001; 


const int MorningHour = 8;
const int EveningHour = 20;
static float Helper = 1;
int TurnOn = 1;
int TurnOff = -1;
int ManualMode;
//int PWMFreq=5000;
//int PWMRes=10;
//int Duty;
unsigned long Now=0;
unsigned long Last=0;
unsigned long Difference=0;

//Regulation
float HeatIsland=35;
float HeatIslandHisteresis=1;
float InteriorTemp=25;
float InteriorTempHisteresis=1;
float InteriorHumidity=80;
float HumidityHisteresis=10;
float SoilMoistureTreshold=10;


WiFiClient client;
OneWire OneWireBus(DallasThermometers);
DallasTemperature TempSensors(&OneWireBus);
DHT HumSensor1(Dht11Pin1, DHT11);
DHT HumSensor2(Dht11Pin2, DHT11);
int RTCAdress=0x64;

DeviceAddress TempAdress2={0x28, 0x94, 0x49, 0x38, 0x80, 0x22, 0xB, 0x2F};
DeviceAddress TempAdress3={ 0x28, 0x3C, 0xE1, 0x7F, 0x80, 0x22, 0xB, 0xB9};
DeviceAddress TempAdress1={0x28, 0xC6, 0x3E, 0xBE, 0x80, 0x22, 0xB, 0x83};

float ReadingTemperature(DeviceAddress);
float ReadingHumidity(int);
float ReadingMoisture();
int TempRelayRegulator(float, float, float);
int HumRelayRegulator(float, float, float);
int SoilMoistureMaintenance(float, float);
void ManualRegulation();
void SetTime();
void printDateTime();

void setup() {

Serial.begin(115200);

pinMode(HeatingCable, OUTPUT);
pinMode(HeatingLamp, OUTPUT);
pinMode(HeatingMat, OUTPUT);
pinMode(Pump, OUTPUT);

digitalWrite(HeatingCable, HIGH);
digitalWrite(HeatingLamp, HIGH);
digitalWrite(HeatingMat, HIGH);
digitalWrite(Pump, HIGH);

WiFi.mode(WIFI_STA);
WiFi.begin(ssid, pass);
 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }

Serial.println("WiFi network established");

ThingSpeak.begin(client);
TempSensors.begin();
HumSensor1.begin();
HumSensor2.begin();
 Wire.begin();



//SetTime();
}

void SetTime()
{
  byte Data[7]={0, 5, 18, 5, 0, 11, 11};//{seconds,minutes,hours,day,date,month,year};
Wire.beginTransmission(RTCAdress);
 Wire.write(0);
for (int i=0; i<7;i++)
{
Wire.write(Data[i]);
}
Wire.endTransmission();

}

void loop() {
Now=millis();
Difference=Now-Last;

if (Difference>60000)
{
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



  Last=Now;
  if(WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_DISCONNECTED){
    Serial.println("Connection lost");
    Serial.println("Measurements don't appear on ThingSpeak");
    WiFi.reconnect();
 }

 ThingSpeak.readMultipleFields(chanelControl, APIKeyControl);
if ((ThingSpeak.getFieldAsInt(1) == TurnOn || ManualMode == 1) && ThingSpeak.getFieldAsInt(1) != TurnOff)
      {
        ManualMode=1;
        ManualRegulation();
      }
    else
if(ThingSpeak.getFieldAsInt(1) == TurnOff )
      {
          ManualMode=0;
      }
      

ThingSpeak.readMultipleFields(chanelSetValue, APISetValue);
if(ThingSpeak.getFieldAsInt(1) != 0)
{
  HeatIsland=ThingSpeak.getFieldAsInt(1);
}
if(ThingSpeak.getFieldAsInt(2) != 0)
{
  InteriorTemp=ThingSpeak.getFieldAsInt(2);
}
if(ThingSpeak.getFieldAsInt(3) != 0)
{
  InteriorHumidity=ThingSpeak.getFieldAsInt(3);
}
if(ThingSpeak.getFieldAsInt(4) != 0)
{
  SoilMoistureTreshold=ThingSpeak.getFieldAsInt(4);
}

delay(1000);
float outputT1=ReadingTemperature(TempAdress1);
delay(1000);
float outputT2=ReadingTemperature(TempAdress2);
delay(1000);
float outputT3=ReadingTemperature(TempAdress3);
delay(1000);
float AverageTemp=(outputT1+outputT2)/2;

//temperature regulation
//delay(10);
//bool onT1 = TempRelayRegulator(HeatIsland, outputT1, HeatIslandHisteresis);
//digitalWrite(HeatingLamp,onT1);
//delay(10);
//bool onT2 = TempRelayRegulator(InteriorTemp, AverageTemp, InteriorTempHisteresis);
//if (InteriorHumidity>=70)
//{
  //digitalWrite(HeatingCable,onT2);
//}
//if (InteriorHumidity<70)
//{
  //digitalWrite(HeatingMat,onT2);
//}

float outputH1= ReadingHumidity(Dht11Pin1);
delay(2000);
float outputH2=ReadingHumidity(Dht11Pin2);
delay(2000); 

//delay(10);
float AverageHum=(outputH1+outputH2)/2;
//bool onH1 = HumRelayRegulator(InteriorHumidity, AverageHum, HumidityHisteresis);
//digitalWrite(Pump,onH1);
//delay(10);
float outputS1=ReadingMoisture();
//bool onS1 = SoilMoistureMaintenance(outputS1, SoilMoistureTreshold);
//digitalWrite(Pump,onS1);
delay(1000);
ThingSpeak.writeFields(chanel, APIKey);	
}
}




void ManualRegulation()
{
  int Cable, Bulb, Mat, Motor;
  if((ThingSpeak.getFieldAsInt(2) == TurnOn || Cable == 1) && ThingSpeak.getFieldAsInt(2) != TurnOff)
  {
    Cable=1;
    digitalWrite(HeatingCable, LOW);
  }
  else 
  if(ThingSpeak.getFieldAsInt(2) == TurnOff)
  {
    Cable=0;
    digitalWrite(HeatingCable, HIGH);
  }


    if((ThingSpeak.getFieldAsInt(3) == TurnOn || Bulb == 1) && ThingSpeak.getFieldAsInt(3) != TurnOff)
  {
    Bulb=1;
    digitalWrite(HeatingLamp, LOW);
  }
  else 
  if(ThingSpeak.getFieldAsInt(3) == TurnOff)
  {
    Bulb=0;
    digitalWrite(HeatingLamp, HIGH);
  }  


      if((ThingSpeak.getFieldAsInt(4) == TurnOn || Mat == 1) && ThingSpeak.getFieldAsInt(4) != TurnOff)
  {
    Mat=1;
    digitalWrite(HeatingMat, LOW);
    ThingSpeak.setField(7, 1);
  }
  else 
  if(ThingSpeak.getFieldAsInt(4) == TurnOff)
  {
    Mat=0;
    digitalWrite(HeatingMat, HIGH);
    ThingSpeak.setField(7, 0);
  }  
  
    if((ThingSpeak.getFieldAsInt(5) == TurnOn || Motor == 1) && ThingSpeak.getFieldAsInt(5) != TurnOff)
  {
    Motor=1;
    digitalWrite(Pump, LOW);
  }
  else 
  if(ThingSpeak.getFieldAsInt(5) == TurnOff)
  {
    Motor=0;
    digitalWrite(Pump, HIGH);
  }  


}

int TempRelayRegulator(float SetValue, float ActualValue, float Histeresis)
{
      float Error;
      bool On;
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

int HumRelayRegulator(float SetValue, float ActualValue, float Histeresis)
{
      float Error;
      bool On;
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


float ReadingTemperature( DeviceAddress TempSensorAdress)
{
float tempValue;
int errTemp;
TempSensors.requestTemperatures();
tempValue=TempSensors.getTempC(TempSensorAdress);
if (TempSensorAdress==TempAdress3)
{
Serial.print("Temperature on the top of the terrarium:");
Serial.println(tempValue);
Serial.println("ºC");
int errTemp=ThingSpeak.setField(3, tempValue);
}
if (TempSensorAdress==TempAdress2)
{
Serial.print("Temperature on the bottom of the terrarium:");
Serial.println(tempValue);
Serial.println("ºC");
int errTemp=ThingSpeak.setField(2, tempValue);
}
if (TempSensorAdress==TempAdress1)
{
  tempValue=TempSensors.getTempC(TempAdress1);
Serial.print("Temperature in the middle of the terrarium:");
Serial.println(tempValue);
Serial.println("ºC");
int errTemp=ThingSpeak.setField(1, tempValue);
}

return tempValue;
}


float ReadingHumidity(int dhtPin)
{
  float humidityValue;
  int humErr;
if(dhtPin==Dht11Pin1){
  humidityValue=HumSensor1.readHumidity();
  Serial.print("Humidity on the top of the terrarium: ");
  Serial.print(humidityValue);
  Serial.println("%");
  humErr=ThingSpeak.setField(4, humidityValue);
}
if(dhtPin==Dht11Pin1){

  humidityValue=HumSensor2.readHumidity();
  Serial.print("Humidity on the bottom of the terrarium: ");
  Serial.print(humidityValue);
  Serial.println("%");
  humErr=ThingSpeak.setField(5, humidityValue);
}
  return humidityValue;
}


float ReadingMoisture()
{
  int SoilMoisture,SoilDry=4095,SoilWet=2453,errMois;
  float SoilPercentege;
  SoilMoisture=analogRead(CapacitiveSensor);
  Serial.print("Soil moisture: ");
  Serial.println(SoilMoisture);
  Serial.print("Percentege soil moisture: ");
  SoilPercentege=map(SoilMoisture,SoilWet,SoilDry,100,0);
  Serial.print(SoilPercentege);
  Serial.println("%");
  errMois=ThingSpeak.setField(6, SoilPercentege);

  return SoilPercentege;
}


float PIDController(float actualValue, float setValue)
{
  static float sum_error;
  float Error,up,ud,ui,u,kp,Td,Ti,Tp,previousError;
  Error=setValue-actualValue;
  sum_error=sum_error+Error;
  up=kp*Error;
  ui=kp/Ti*sum_error;
  ud=kp*Td*(Error-previousError);
  u=up+ui+ud;
  previousError=Error;
  return u;
}