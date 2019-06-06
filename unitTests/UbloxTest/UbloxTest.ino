//Author: Asif Ally

#include <SD.h>
#include <UbloxGPS.h>

const int chipSelect = BUILTIN_SDCARD;

UbloxGPS gps = UbloxGPS(&Serial2);
boolean fixU = false;

File uLog;
String Fname = "";
String data;
boolean SDcard = true;
unsigned long timer = 0;


void setup() {
  //Start serial
  Serial.begin(9600);
  //Start ublox
  Serial2.begin(UBLOX_BAUD);
  
  while(!Serial){
    ; //Wait for serial port to connect
  }
  Serial.print("Initializing SD card..");
  if (!SD.begin(chipSelect)){
    Serial.println("Initialization Failed!");
    SDcard = false;
  }
  SDcard=true;
  
  //This "for" loop checks to see if there are any previous files on
  //the SD card already with the generated name
  for (int i = 0; i < 100; i++) {
    Fname = String("tLog" + String(i/10) + String(i%10) + ".csv");
    if (!SD.exists(Fname.c_str())){
      break; 
    }
  }
  
  //Start GPS 
  gps.init();
  //Attempt to set to airborne 3 times. If successful, records result and breaks loop. If unsuccessful, saves warning and moves on
  byte i = 0;
  while (i < 3) {
    i++;
    if (gps.setAirborne()) {
      Serial.println("Air mode successfully set.");
      break;
    }
    else if (i == 3)
      Serial.println("WARNING: Failed to set to air mode (3 attemtps). Altitude data may be unreliable.");
    else
      Serial.println("Error: Air mode set unsuccessful. Reattempting...");
  }
  Serial.println("GPS configured");
  //print header line to SD file
  
  uLog = SD.open(Fname.c_str(), FILE_WRITE);
  String header = "Date,Time,Lattitude,Longitude,Altitude (m),Satellites,Fix";
  uLog.println(header);
  uLog.close();
  Serial.println("Ublox Logger header added");
  
}
void getUbloxData(){
  gps.update();

  //log data once every second
  if(millis() - timer > 1000) {
    timer = millis();
    //All data is returned as numbers (int or float as appropriate), so values must be converted to strings before logging
    String data = String(gps.getMonth()) + "/" + String(gps.getDay()) + "/" + String(gps.getYear()) + ","
                  + String(gps.getHour()) + ":" + String(gps.getMinute()) + ":" + String(gps.getSecond()) + ","
                  + String(gps.getLat(), 4) + "," + String(gps.getLon(), 4) + "," + String(gps.getAlt_meters(), 1) + ","
                  + String(gps.getSats()) + ",";
    //GPS should update once per second, if data is more than 2 seconds old, fix was likely lost
    if(gps.getFixAge() > 2000)
      data += "No Fix,";
    else
      data += "Fix,";
    Serial.println(data);
    uLog = SD.open(Fname.c_str(), FILE_WRITE);
    uLog.println(data);
    uLog.close();
    delay(10);
  }
}

void loop() {
  getUbloxData();
}
