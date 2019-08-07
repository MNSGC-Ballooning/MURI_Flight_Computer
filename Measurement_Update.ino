//bool DataUpdate=true;
//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
 static unsigned long prevTime = 0;
 if(millis()-prevTime>= LOG_TIMER){

  prevTime = millis();
  

  if(batHeatRelay.getState()==true){
    batHeat_Status = "ON";
  }
  else if(batHeatRelay.getState()==false){
    batHeat_Status = "OFF";
  }
  if(sensorHeatRelay.getState()==true){
    sensorHeat_Status = "ON";
  }
  else if(sensorHeatRelay.getState()==false){
    sensorHeat_Status = "OFF";
  }

  sensor1.requestTemperatures();
  sensor2.requestTemperatures();
  sensor3.requestTemperatures();



  //Pressure, Temp, and Altitude

  t1 = sensor1.getTempCByIndex(0);
  t2 = sensor2.getTempCByIndex(0);
  t3 = sensor3.getTempCByIndex(0);


  alt_GPS = GPS.getAlt_feet();                                    // altitude calulated by the Ublox GPS

  pressureSensor = analogRead(HONEYWELL_PRESSURE);                //Read the analog pin
  pressureSensorVoltage = pressureSensor * (5.0/1024);            //Convert the analog number to voltage
  PressurePSI = (pressureSensorVoltage - (0.1*5.0))/(4.0/15.0);   //Convert the voltage to PSI
  PressureATM = PressurePSI*PSI_TO_ATM;                           //Convert PSI reading to ATM


  //Populate a string with the OPC data
  
  OPCdata = PlanA.logUpdate();
  OPCdata += ",=," + SPSA.logUpdate();
  OPCdata += ",=," + R1A.logUpdate();
  
  data = "";
  data = flightTimeStr()+ "," + flightMinutes() + "," + String(GPS.getLat(), 4) + "," + String(GPS.getLon(), 4) + "," 
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


  data += (String(t1) + "," +String(t2) + "," + String(t3) + ",");
  data += (String(PressurePSI) + "," + String(PressureATM) + ",");
  data += (batHeat_Status + "," + sensorHeat_Status + ",");
  data += (String(Control_Altitude) + ",");
  data += (SmartLogA + "," + smartOneCut + "," + SmartLogB + "," + smartTwoCut + "," + String(ascent_rate) + "," + stateString + ",");
  data += (",=," + OPCdata);
  openFlightlog();
  Serial.println(data);
  delay(100);
  
  Flog.println(data);
  closeFlightlog();

//SMART 
  ChangeData=true; //Telling SmartController that we have logged the data
  
  }  
  
}
  
  
