

float checkAlt;
long lastGPS = -1000000;  //for testing purposes

//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
  static unsigned long prevTime = 0;
  if(millis() - prevTime >= 1400){
    adxl.readAccel(&x,&y,&z);
    sensor1.requestTemperatures();
    sensor2.requestTemperatures();
    sensor3.requestTemperatures();
    sensor4.requestTemperatures();
    t1 = sensor1.getTempCByIndex(0) + 273.15;
    t2 = sensor2.getTempCByIndex(0) + 273.15;
    t3 = sensor3.getTempCByIndex(0) + 273.15;
    t4 = sensor4.getTempCByIndex(0) + 273.15;
    pressure = analogRead(A0);
    pressureV=pressure*(5.0/1024);
    psi = (pressureV = (0.1*5.0))/(4.0/15.0);
    while (Serial1.available() > 0) {
      GPS.encode(Serial1.read());
    }
    if (GPS.altitude.isUpdated() || GPS.location.isUpdated()) {
      newData= true;
      if (!firstFix && GPS.Fix) {     //gps.fix
        GPSstartTime = GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second();
        firstFix = true;
  
      }
      if (getGPStime() > lastGPS && newData) {
        openFlightlog();
        String data = "";
        data += (flightTimeStr() + "," + String(GPS.location.lat(), 6) + "," + String(GPS.location.lng(), 6) + ",");
        data += ((String(GPS.altitude.feet())) + ",");    //convert meters to feet for datalogging
        data += (String(GPS.date.month()) + "/" + String(GPS.date.day()) + "/" + String(GPS.date.year()) + ",");
        data += (String(GPS.time.hour()) + ":" + String(GPS.time.minute()) + ":" + String(GPS.time.second()) + ","); 
        if (GPS.Fix) { 
          data += "fix,";
          lastGPS = GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second();
        }
        else{
          data += ("No fix,");
          lastGPS = GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second();
        }
        data += (String(x) + "," + String(y) + "," + String(z) + ","); 
        data += (String(t1) + "," +String(t2) + "," + String(t3) + String(t4) + ",");
        data += (Bat_heaterStatus + "," + OPC_heaterStatus + ",");
        data += (String(psi));
        Flog.println(data);
        closeFlightlog();
      }
    }
    prevTime=millis();
  }
}
int getGPStime() {
  return (GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second());
}

int getLastGPS() {    //returns time in seconds between last successful fix and initial fix. Used to match with altitude data
  if (!newDay && lastGPS < GPSstartTime) {
    days++;
    newDay = true;
  }
  else if (newDay && lastGPS > GPSstartTime)
    newDay = false;
  return days * 86400 + lastGPS;
}

