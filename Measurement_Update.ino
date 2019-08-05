//bool DataUpdate=true;
//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
 static unsigned long prevTime = 0;
 if(millis()-prevTime>= LOG_TIMER){

  prevTime = millis();
  
//  if(opcRelay.getState()==true){
//    opcRelay_Status = "ON";
//  }
//  else if(opcRelay.getState()==false){
//    opcRelay_Status = "OFF";
//  }
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

  t1 = sensor1.getTempCByIndex(0);
  t2 = sensor2.getTempCByIndex(0);
  t3 = sensor3.getTempCByIndex(0);
  t4 = sensor4.getTempCByIndex(0);


  alt_GPS = GPS.getAlt_feet();                                // altitude calulated by the Ublox GPS
  
  OPCdata = PlanA.logUpdate();
  OPCdata += ",=," + SPSA.logUpdate();
  OPCdata += ",=," + R1A.logUpdate();
  
  data = "";
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
  data += (String(Control_Altitude) + ",");
  data += (SmartLogA + "," + SmartLogB + "," + String(ascent_rate) + "," + stateString + ",");
  data += ("=," + OPCdata);
  openFlightlog();
  Serial.println(data);
  delay(100);
  
  Flog.println(data);
  closeFlightlog();

//SMART 
  ChangeData=true; //Telling SmartController that we have logged the data
  
  }  
  
}
  
  
