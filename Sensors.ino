

//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
  static unsigned long prevTime = 0;
  if(millis() - prevTime >= 5000){
    prevTime = millis();
    sensor1.requestTemperatures();
    sensor2.requestTemperatures();
    sensor3.requestTemperatures();
    sensor4.requestTemperatures();

    //Smart Unit temp requests

    if (!tempA){
      SOCO.request(1);
      tempA=true;
    }
    else {
      SOCO.request(2);
      tempA=false;
    }

    
    //MS5607 temp and pressure
    myBaro.baroTask();
    Serial.println(myBaro.getReferencePressure());
    pressure = myBaro.getPressure()/10;
    altitude = myBaro.getAltitude(); //- start
    temperature = myBaro.getTemperature()+C2K;
    t1 = sensor1.getTempCByIndex(0) + 273.15;
    t2 = sensor2.getTempCByIndex(0) + 273.15;
    t3 = sensor3.getTempCByIndex(0) + 273.15;
    t4 = sensor4.getTempCByIndex(0) + 273.15;
    String data = "";
    openFlightlog();
    data = flightTimeStr()+ "," + String(Ublox.getLat(), 4) + "," + String(Ublox.getLon(), 4) + "," 
    + String(Ublox.getAlt_feet(), 1) + ","
    + String(Ublox.getMonth()) + "/" + String(Ublox.getDay()) + "/" + String(Ublox.getYear()) + ","
    + String(Ublox.getHour()) + ":" + String(Ublox.getMinute()) + ":" + String(Ublox.getSecond()) + ","
    
    + String(Ublox.getSats()) + ",";
    
    //GPS should update once per second, if data is more than 2 seconds old, fix was likely lost
    if(Ublox.getFixAge() > 2000){
      data += "No Fix,";
      fixU == false;
    }
  else{
      data += "Fix,";
      fixU == true;
    }
    
    data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + ",");
    data += (batHeatRelay.getRelayStatus() + "," + opcHeatRelay.getRelayStatus() + ",");
    data += (String(temperature)+ ",");
    data += (String(pressure) + ",");
    data += (String(altitude) + ",");
    data += (SmartLog + ",");
    ChangeData=true; //Telling SmartController that we have logged the data

    Serial.println(data);
    Flog.println(data);
    closeFlightlog();
  }
  
}
