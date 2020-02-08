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
  sensor4.requestTemperatures();

  t1 = sensor1.getTempCByIndex(0);
  t2 = sensor2.getTempCByIndex(0);
  t3 = sensor3.getTempCByIndex(0);
  t4 = sensor4.getTempCByIndex(0);
  t5 = thermocouple.readThermocoupleTemperature();

  alt_GPS = GPS.getAlt_feet();                                          //Altitude calulated by the Ublox GPS

  pressureSensor = analogRead(HONEYWELL_PRESSURE);                      //Read the analog pin
  pressureSensorVoltage = pressureSensor * (3.3/8196);                  //Convert the analog number to voltage    //THESE NEED TO BE 3.3 INSTEAD OF 5.0!!!!!!!!!!
  PressurePSI = (pressureSensorVoltage - (0.1*3.3))/(4.0/15.0);         //Convert the voltage to PSI
  PressureATM = PressurePSI*PSI_TO_ATM;                                 //Convert PSI reading to ATM

  OPCdata = PlanA.logUpdate();                                          //Populate a string with the OPC data
  OPCdata += ",=," + PlanB.logUpdate();
  OPCdata += ",=," + SpsA.logUpdate();
  OPCdata += ",=," + R1A.logUpdate();

  data = "";
  data = flightTimeStr()+ "," + String(flightMinutes()) + "," +  String(masterClockMinutes(),2) + "," + String(GPS.getLat(), 4) + "," + String(GPS.getLon(), 4) + "," 
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

  data += (String(t1,4) + "," +String(t2,4) + "," + String(t3,4) + "," + String(t4,4) + "," + String(t5,4) + ",");     //Data string population
  data += (String(PressurePSI,6) + "," + String(PressureATM,6) + ",");
  data += (batHeat_Status + "," + sensorHeat_Status + ",");
  data += (String(Control_Altitude) + ",");
  data += (SmartLogA + "," + smartOneCut + "," + SmartLogB + "," + smartTwoCut + "," + String(ascent_rate) + "," + String(avg_ascent_rate) + ","  + stateString);
  data += (",=," + OPCdata);
  openFlightlog();
//  Serial.println(data);
  delay(100);
  
  Flog.println(data);
  closeFlightlog();
 
  ChangeData=true;                                                     //Telling SmartController that we have logged the data

  Serial.println();
  Serial.println("Measurement Update");
  Serial.println("========================================================================================");
  Serial.println("             Time");
  Serial.print("Flight Time String: ");
  Serial.println(flightTimeStr());
  Serial.print("Flight Minutes: ");
  Serial.println(String(flightMinutes()));
  Serial.print("Master Clock: ");
  Serial.println(String(masterClockMinutes(),2));
  Serial.println("------------------------------");
  Serial.println("             GPS");
  Serial.print("Latitude: ");
  Serial.println(String(GPS.getLat(), 4));
  Serial.print("Longitude: ");
  Serial.println(String(GPS.getLon(), 4));  
  Serial.print("Altitude: ");
  Serial.println(String(alt_GPS, 1));
  Serial.print("Date and Time: ");
  Serial.println(String(GPS.getMonth()) + "/" + String(GPS.getDay()) + "/" + String(GPS.getYear()) + " " + String(GPS.getHour()) + ":" + String(GPS.getMinute()) + ":" + String(GPS.getSecond()));
  Serial.print("Satellites and Fix Age: ");
  Serial.println((String(GPS.getSats()) + ", " + String(GPS.getFixAge())));
  Serial.println("------------------------------");
  Serial.println("          Temperature");
  Serial.println("   t1        t2        t3        t4        t5"); 
  Serial.println((String(t1,4) + ", " +String(t2,4) + ", " + String(t3,4) + ", " + String(t4,4) + ", " + String(t5,4)));
  Serial.println("------------------------------");
  Serial.println("           Pressure");
  Serial.println("Pressure(PSI)  Pressure(ATM)");
  Serial.print("  ");
  Serial.println((String(PressurePSI,6) + "      " + String(PressureATM,6)));
  Serial.println("------------------------------");
  Serial.println("       System Statuses");
  Serial.println("Battery Heater Relay    Sensor Heater Relay");
  Serial.println(("         " + batHeat_Status + "                  " + sensorHeat_Status));
  Serial.print("Control Altitude: ");
  Serial.println(String(Control_Altitude));
  Serial.print("Smart A: ");
  Serial.println((SmartLogA + ", " + smartOneCut));
  Serial.print("Smart B: ");
  Serial.println((SmartLogB + ", " + smartTwoCut));
  Serial.print("Instantaneous Ascent Rate and Average Ascent Rate: ");
  Serial.println((String(ascent_rate) + ", " + String(avg_ascent_rate)));
  Serial.print("State: ");
  Serial.println(stateString);
  Serial.println("------------------------------");
  Serial.println("             OPCs");
  Serial.print("Plan A: ");
      if (PlanA.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      } 
  Serial.print("Plan B: ");
      if (PlanB.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      }
  Serial.print("SPS A: ");
      if (SpsA.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      }
  Serial.print("R1 A: ");
      if (R1A.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      }
  Serial.println("Raw data:");          
  Serial.println(OPCdata);
  Serial.println("========================================================================================");  
  
}
