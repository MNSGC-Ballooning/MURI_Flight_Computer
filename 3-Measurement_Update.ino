//bool DataUpdate=true;
//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
 static unsigned long prevTime = 0;
 if(millis()-prevTime>=3000){

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


  alt_GPS = GPS.getAlt_feet();                                // altitude calulated by the Ublox GPS


  pressureSensor = analogRead(HONEYWELL_PRESSURE);              //Read the analog pin
  pressureSensorVoltage = pressureSensor * (5.0/1024);          //Convert the analog number to voltage
  PressurePSI = (pressureSensorVoltage - (0.1*5.0))/(4.0/15.0); //Convert the voltage to PSI
  PressureATM = PressurePSI*PSI_TO_ATM;

  //openFlightlog();
  data = "";
  data = flightTimeStr()+ "," + String(GPS.getLat(), 4) + "," + String(GPS.getLon(), 4) + "," 
  + String(alt_GPS, 1) + ","
  + String(GPS.getMonth()) + "/" + String(GPS.getDay()) + "/" + String(GPS.getYear()) + ","
  + String(GPS.getHour()) + ":" + String(GPS.getMinute()) + ":" + String(GPS.getSecond()) + ","
  + String(GPS.getSats()) + ",";
  
  //GPS should update once per second, if data is more than 4 seconds old, fix was likely lost
  if(GPS.getFixAge() > 4000){
    data += "No Fix,";
    GPSfix = false;
  }
  else{
    data += "Fix,";
    GPSfix = true;
  }


  data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + ",");
  data += (batHeat_Status + "," + opcHeat_Status + ",");
  data += (String(Control_Altitude) + ",");
  data += (SmartLogA + "," + String(ascent_rate) + "," + stateString + ",");
  openFlightlog();
  Serial.println(data);
  delay(100);
  pmsUpdateA();

  //pmsUpdateB();

  Serial.println(dataPMSA);
  Flog.println(data + " " + "," + dataPMSA);
  closeFlightlog();

//SMART 
  ChangeData=true; //Telling SmartController that we have logged the data
  
  }  
  
}
  
  
