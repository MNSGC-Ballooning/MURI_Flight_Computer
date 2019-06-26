//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
 static unsigned long prevTime = 0;
 if(millis()-prevTime>=5000){
  prevTime = millis();
 
  if(opcRelay.getState()==true){
    opcRelay_Status = "ON";
  }
  else if(opcRelay.getState()==false){
    opcRelay_Status = "OFF";
  }
  if(batHeatRelay.getState()==true){
    batHeat_Status = "ON";
  }
  else if(batHeatRelay.getState()==false){
    batHeat_Status = "OFF";
  }
  if(opcHeatRelay.getState()==true){
    opcHeat_Status = "ON";
  }
  else if(opcHeatRelay.getState()==false){
    opcHeat_Status = "OFF";
  }

  sensor1.requestTemperatures();
  sensor2.requestTemperatures();
  sensor3.requestTemperatures();
  sensor4.requestTemperatures();



  //Pressure, Temp, and Altitude

  t1 = sensor1.getTempCByIndex(0) + 273.15;
  t2 = sensor2.getTempCByIndex(0) + 273.15;
  t3 = sensor3.getTempCByIndex(0) + 273.15;
  t4 = sensor4.getTempCByIndex(0) + 273.15;

  myBaro.baroTask();
  pressure = myBaro.getPressure()/10;
  temperature = myBaro.getTemperature()+C2K;

  alt_GPS = GPS.getAlt_feet();                                // altitude calulated by the Ublox GPS
  alt_pressure_library = myBaro.getAltitude()*METERS_TO_FEET;   // altitude calcuated by the pressure sensor library
  alt_pressure = Pressure_Alt_Calc(pressure*1000, t2);               // altitude calculated by the Hypsometric formula using pressure sensor data

  String data = "";
  openFlightlog();
  data = flightTimeStr()+ "," + String(GPS.getLat(), 4) + "," + String(GPS.getLon(), 4) + "," 
  + String(alt_GPS, 1) + ","
  + String(GPS.getMonth()) + "/" + String(GPS.getDay()) + "/" + String(GPS.getYear()) + ","
  + String(GPS.getHour()) + ":" + String(GPS.getMinute()) + ":" + String(GPS.getSecond()) + ","
  + String(GPS.getSats()) + ",";
  
  //GPS should update once per second, if data is more than 2 seconds old, fix was likely lost
  if(GPS.getFixAge() > 4000){
    data += "No Fix,";
    //fixU == false;
  }
else{
    data += "Fix,";
    //fixU == true;
  }


  
  data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + ",");
  data += (batHeat_Status + "," + opcHeat_Status + ",");
  data += (String(temperature)+ ",");
  data += (String(pressure) + ",");
  data += (String(alt_pressure_library) + ",");
  data += (String(alt_pressure) + ",");
  data += (SmartLog + "," + String(ascent_rate) + "," + stateString + ",");
  ChangeData=true; //Telling SmartController that we have logged the data

  Serial.println(data + ",");

  Flog.println(data);
  closeFlightlog();
 
  
//Smart Unit temp requests (Moved closer to end to be closer to SOCO response, maybe in future could move request temp right before SOCO response)

  if (!tempA){
    SOCO.RequestTemp(1);
    tempA=true;
  }
  else {
    SOCO.RequestTemp(2);
    tempA=false;
  }
 
//  if(readPMSdata(&PMSserial)){
//    Serial.println("Reading data was successful!");
//    openFlightlogPMS();
//    String dataPMS ="";
//  // log sample number, in flight time
//    dataPMS += ntot;
//    dataPMS += ",";
//    dataPMS += flightTimeStr(); //in flight time from Flight_Timer 
//    dataPMS += "," + PMSdata.particles_03um;
//    dataPMS += "," + PMSdata.particles_05um;
//    dataPMS += "," + PMSdata.particles_10um;
//    dataPMS += "," + PMSdata.particles_25um;
//    dataPMS += "," + PMSdata.particles_50um;
//    dataPMS += "," + PMSdata.particles_100um;
//    dataPMS += "," + String(GPS.getSats());
//  }
    Serial.println(dataPMS);
    PMSLog.println(dataPMS);
    nhits+=1;
    ntot+=1;
    closeFlightlogPMS();
  }
}
