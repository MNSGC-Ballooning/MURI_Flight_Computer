//function to handle both retrieval of data from sensors, as well as recording it on the SD card
void updateSensors() {

//Pressure, Temp, and Altitude
  sensor1.requestTemperatures();

  t1 = sensor1.getTempCByIndex(0);

//  alt_GPS = GPS.getAlt_feet();                                          //Altitude calulated by the Ublox GPS

  pressureSensor = analogRead(HONEYWELL_PRESSURE);                      //Read the analog pin
  pressureSensorVoltage = pressureSensor * (5.0/8196);                  //Convert the analog number to voltage
  PressurePSI = (pressureSensorVoltage - (0.1*5.0))/(4.0/15.0);         //Convert the voltage to PSI
  PressureATM = PressurePSI*PSI_TO_ATM;                                 //Convert PSI reading to ATM

 // OPCdata = PlanA.logUpdate();                                          //Populate a string with the OPC data
 // OPCdata += ",=," + PlanB.logUpdate();
  OPCdata = SPSA.logUpdate();
  OPCdata += ",=," + SPSB.logUpdate();

  data = "";
  data = String(millis()) + ",";


  data += (String(t1,4) + ",");     //Data string population
  data += (String(PressurePSI,6) + "," + String(PressureATM,6) + ",");
  data += (",=," + OPCdata);
  openFlightlog();
  Serial.println(data);
  delay(100);
  
  Flog.println(data);
  closeFlightlog();  
}
