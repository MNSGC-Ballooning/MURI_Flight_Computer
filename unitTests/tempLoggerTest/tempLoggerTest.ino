
// Written by: Asif Ally
// 5/31/2019

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
//Setting up SD card file stuff//
File tLog;
// NOTE the SD wont work if the file name is
// longer than 8 characters. 
// Also the for loop further down adds more characters 
// so be careful
const int chipSelect = BUILTIN_SDCARD;
String data;
String Fname = "";
boolean SDcard = true;
//Setting up variables and such for temp sensors//
#define ONE_WIRE_BUS 29
#define TWO_WIRE_BUS 30
#define THREE_WIRE_BUS 31
#define FOUR_WIRE_BUS 32
OneWire oneWire1(ONE_WIRE_BUS);
OneWire oneWire2(TWO_WIRE_BUS);
OneWire oneWire3(THREE_WIRE_BUS);
OneWire oneWire4(FOUR_WIRE_BUS);
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);
DallasTemperature sensor3(&oneWire3);
DallasTemperature sensor4(&oneWire4);
float t1;
float t2;
float t3;
float t4;



void setup() {
  Serial.begin(9600);
  //Starting the Temp sensors:
  sensor1.begin();
  sensor2.begin();
  sensor3.begin();
  sensor4.begin();

  while(!Serial){
    ; //Wait for serial port to connect
  }

  Serial.print("Initializing SD card...");
  //Tells us if the SD card faled to open:
  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization Failed!");
    SDcard = false;
  }
  SDcard = true;
  Serial.println("Initialization done.");
  //This "for" loop checks to see if there are any previous files on
  //the SD card already with the generated name
  for (int i = 0; i < 100; i++) {
    
    Fname = String("tLog" + String(i/10) + String(i%10) + ".csv");
    if (!SD.exists(Fname.c_str())){
      break; 
    }
  }
  
  Serial.println("Temperature Logger created: " + Fname);
  tLog = SD.open(Fname.c_str(), FILE_WRITE);
  String FHeader = "Temp 1,Temp 2,Temp 3,Temp 4";
  tLog.println(FHeader);//Set up temp log format and header
  tLog.close();
  Serial.println("Temp Logger header added");
  
}
void writeSensorsSD(){
  sensor1.requestTemperatures();
  sensor2.requestTemperatures();
  sensor3.requestTemperatures();
  sensor4.requestTemperatures();
  t1 = sensor1.getTempCByIndex(0) + 273.15;
  t2 = sensor2.getTempCByIndex(0) + 273.15;
  t3 = sensor3.getTempCByIndex(0) + 273.15;
  t4 = sensor4.getTempCByIndex(0) + 273.15;
  tLog = SD.open(Fname.c_str(), FILE_WRITE);
  data = "t1=" + String(t1) +',' + "t2=" + String(t2) +',' + "t3=" + String(t3) + ',' + "t4=" + String(t4) + ',';
  Serial.println(data);
  tLog.println(data);
  Serial.println("Data line was added");
  tLog.close();
}
void loop() {
  writeSensorsSD();
}
