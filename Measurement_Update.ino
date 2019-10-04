//function to handle both retrieval of data from sensors, as well as recording it on the SD card
void updateSensors() {
  
  oledTime = millis();
  if(batHeatRelay.getState()==true){                                     //Relay Status Collection
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

//Pressure, Temp, and Altitude
  sensor1.requestTemperatures();
  sensor2.requestTemperatures();
  sensor3.requestTemperatures();

  t1 = sensor1.getTempCByIndex(0);
  t2 = sensor2.getTempCByIndex(0);
  t3 = sensor3.getTempCByIndex(0);
  t4 = thermocouple.readCJTemperature();
  t5 = thermocouple.readThermocoupleTemperature();

  alt_GPS = GPS.getAlt_feet();                                          //Altitude calulated by the Ublox GPS
  RFD_SERIAL.print(String(alt_GPS));

  pressureSensor = analogRead(HONEYWELL_PRESSURE);                      //Read the analog pin
  pressureSensorVoltage = pressureSensor * (5.0/1024);                  //Convert the analog number to voltage
  PressurePSI = (pressureSensorVoltage - (0.1*5.0))/(4.0/15.0);         //Convert the voltage to PSI
  PressureATM = PressurePSI*PSI_TO_ATM;                                 //Convert PSI reading to ATM

  OPCdata = SPSA.logUpdate();                                          //Populate a string with the OPC data
  OPCdata += ",=," + SPSB.logUpdate();
  OPCdata += ",=," + HPMA.logUpdate();
  
  data = "";
  data = flightTimeStr()+ "," + String(flightMinutes()) + "," +  String(masterClockMinutes()) + "," + String(GPS.getLat(), 4) + "," + String(GPS.getLon(), 4) + "," 
  + String(alt_GPS, 1) + ","
  + String(GPS.getMonth()) + "/" + String(GPS.getDay()) + "/" + String(GPS.getYear()) + ","
  + String(GPS.getHour()) + ":" + String(GPS.getMinute()) + ":" + String(GPS.getSecond()) + ","
  + String(GPS.getSats()) + ",";
  
  if(GPS.getFixAge() > 4000){                                           //GPS should update once per second, if data is more than 2 seconds old, fix was likely lost
    data += "No Fix,";
  }
  else{
    data += "Fix,";
  }

  data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + "," + String(t5) + ",");     //Data string population
  data += (String(PressurePSI) + "," + String(PressureATM) + ",");
  data += (batHeat_Status + "," + sensorHeat_Status + ",");
  data += (String(Control_Altitude) + ",");
  data += (String(ascent_rate) + "," + stateString + "," + SmartLogA + "," + SmartLogB + "," + packet);
  data += (",=," + OPCdata);
  openFlightlog();
  Serial.println(data);
  delay(100);
  
  Flog.println(data);
  closeFlightlog();   
}
