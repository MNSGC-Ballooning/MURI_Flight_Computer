

//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
  static unsigned long prevTime = 0;
  if(millis() - prevTime >= 1000){
    prevTime = millis();
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
    psi = (pressureV - (0.1*5.0))/(4.0/15.0);
    kpa = psi * 6.89476; //Pressure in kpa
    
    openFlightlog();
    openBloxlog();
    
    //Copernicus and sensor suite data
    String data = flightTimeStr() + "," + String(Copernicus.getLat(), 4) + "," + String(Copernicus.getLon(), 4) + "," + String(Copernicus.getAlt()*3.28084, 1) + "," 
    + String(Copernicus.getMonth()) + "/" + String(Copernicus.getDay()) + "/" + String(Copernicus.getYear()) + ","
    + String(Copernicus.getHour()) + ":" + String(Copernicus.getMinute()) + ":" + String(Copernicus.getSecond()) + ","
    + String(Copernicus.getSats()) + ",";
    //GPS should update once per second, if data is over two seconds old, fix was probably lost
    if(Copernicus.getFixAge()>2000){
      data += "No Fix,";
    }
    else{
      data += "Fix,";
    }
    data += String(x) + "," + String(y) + "," + String(z) + ","
    + String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + ","
    + String(batHeatRelay.getRelayStatus()) + "," + String(opcHeatRelay.getRelayStatus()) + ","
    + String(kpa) + ",";
    
    //Ublox data
    String data2 = flightTimeStr() + "," + String(Ublox.getLat(), 4) + "," + String(Ublox.getLon(), 4) + "," + String(Ublox.getAlt()*3.28084, 1) + "," 
    + String(Ublox.getMonth()) + "/" + String(Ublox.getDay()) + "/" + String(Ublox.getYear()) + ","
    + String(Ublox.getHour()) + ":" + String(Ublox.getMinute()) + ":" + String(Ublox.getSecond()) + ","
    + String(Ublox.getSats()) + ",";
    //GPS should update once per second, if data is more than 2 seconds old, fix was likely lost
    if(Ublox.getFixAge() > 2000){
      data2 += "No Fix,";
    }
    else{
      data2 += "Fix,";
    }
    
    Serial.println(data);
    //Serial.println(data2);
    
    Flog.println(data);
    bloxLog.println(data2);
    closeFlightlog();
    closeBloxlog();
  }
  
}


