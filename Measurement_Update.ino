//bool DataUpdate=true;
//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
 static unsigned long prevTime = 0;
 if(millis()-prevTime>=4000){

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

  t1 = sensor1.getTempCByIndex(0) + 273.15;
  t2 = sensor2.getTempCByIndex(0) + 273.15;
  t3 = sensor3.getTempCByIndex(0) + 273.15;
  t4 = sensor4.getTempCByIndex(0) + 273.15;

  myBaro.baroTask();
  pressure = myBaro.getPressure()/10;
  temperature = myBaro.getTemperature()+C2K;


  alt_GPS = GPS.getAlt_feet();                                // altitude calulated by the Ublox GPS
  alt_pressure_library = myBaro.getAltitude()*METERS_TO_FEET;   // altitude calcuated by the pressure sensor library
//  alt_pressure = Pressure_Alt_Calc(pressure*1000, t2);               // altitude calculated by the Hypsometric formula using pressure sensor data
  //openFlightlog();
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
  data += (String(temperature)+ ",");
  data += (String(pressure) + ",");
  data += (String(alt_pressure_library) + ",");
  data += (String(Control_Altitude) + ",");
  data += (SmartLogA + "," + SmartLogB + "," + String(ascent_rate) + "," + stateString + ",");
  openFlightlog();
  Serial.println(data);
  delay(100);
  pmsUpdateA();

  pmsUpdateB();

  Serial.println(dataPMSA);
  Serial.println(dataPMSB);
  Flog.println(data + " " + "," + dataPMSA + "," + " " + "," + dataPMSB);
  closeFlightlog();

//SMART 
  ChangeData=true; //Telling SmartController that we have logged the data
  
  }  
  
}
  
  
