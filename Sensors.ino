

//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
  static unsigned long prevTime = 0;
  if(millis() - prevTime >= 5000){
    prevTime = millis();
    
    adxl.readAccel(&x,&y,&z);
    
    //Request temp values
    sensor1.requestTemperatures();
    sensor2.requestTemperatures();
    sensor3.requestTemperatures();
    sensor4.requestTemperatures();
    //MS5607 temp and pressure
    MS5.ReadProm();
    MS5.Readout();
    ms_temp = (MS5.GetTemp()/100);    //because temp is given in .01 C
    ms_pressure = MS5.GetPres();

    //Get temp values
    t1 = sensor1.getTempCByIndex(0) + 273.15;
    t2 = sensor2.getTempCByIndex(0) + 273.15;
    t3 = sensor3.getTempCByIndex(0) + 273.15;
    t4 = sensor4.getTempCByIndex(0) + 273.15;

    pressure = analogRead(A0);
    pressureV=pressure*(5.0/1024);
    psi = (pressureV - (0.1*5.0))/(4.0/15.0);
    kpa = psi * 6.89476; //Pressure in kpa
    String data = "";
    openFlightlog();
    //openBloxlog();
    if(GPS.Fix && GPS.altitude.feet()!=0) {
      data += (flightTimeStr() + "," + String(GPS.location.lat(), 6) + "," + String(GPS.location.lng(), 6) + ",");
      data += ((String(GPS.altitude.feet())) + ",");    //convert meters to feet for datalogging
      data += (String(GPS.date.month()) + "/" + String(GPS.date.day()) + "/" + String(GPS.date.year()) + ",");
      data += (String(GPS.time.hour()) + ":" + String(GPS.time.minute()) + ":" + String(GPS.time.second()) + ",");
      data += "fix,";
    }
    else{
    data += (flightTimeStr() + ",0.000000,0.000000,0.00,0/0/2000,00:00:00,No Fix,");
    
    }
    data += (String(x) + "," + String(y) + "," + String(z) + ","); 
    data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + ",");
    data += (String(batHeatRelay.getRelayStatus()) + "," + String(opcHeatRelay.getRelayStatus()) + ",");
    data += (String(kpa) + ",");
    data += (String(ms_temp)+ ",");
    data += (String(ms_pressure) + ",");
//    String data2 = String(Ublox.getMonth()) + "/" + String(Ublox.getDay()) + "/" + String(Ublox.getYear()) + ","
//    + String(Ublox.getHour()) + ":" + String(Ublox.getMinute()) + ":" + String(Ublox.getSecond()) + ","
//    + String(Ublox.getLat(), 4) + "," + String(Ublox.getLon(), 4) + "," + String(Ublox.getAlt(), 1) + ","
//    + String(Ublox.getSats()) + ",";
//    //GPS should update once per second, if data is more than 2 seconds old, fix was likely lost
//    if(Ublox.getFixAge() > 2000){
//      data2 += "No Fix,";
//    }
//    else{
//      data2 += "Fix,";
//    }
    Serial.println(data);
    //Serial.println(data2);
    Flog.println(data);
    //bloxLog.println(data2);
    closeFlightlog();
    writeEvents();
    //closeBloxlog();

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
//    if(GPS.Fix && GPS.altitude.feet()!=0) {
//      data += (flightTimeStr() + "," + String(GPS.location.lat(), 6) + "," + String(GPS.location.lng(), 6) + ",");
//      data += ((String(GPS.altitude.feet())) + ",");    //convert meters to feet for datalogging
//      data += (String(GPS.date.month()) + "/" + String(GPS.date.day()) + "/" + String(GPS.date.year()) + ",");
//      data += (String(GPS.time.hour()) + ":" + String(GPS.time.minute()) + ":" + String(GPS.time.second()) + ",");
//      data += "fix,";
//    }
//    else{
//    data += (flightTimeStr() + ",0.000000,0.000000,0.00,0/0/2000,00:00:00,No Fix,");
//    
//    }
    
    data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + ",");
    data += (batHeatRelay.getRelayStatus() + "," + opcHeatRelay.getRelayStatus() + ",");
    data += (String(ms_temp)+ ",");
    data += (String(ms_pressure) + ",");
    

    Serial.println(data);
    Flog.println(data);
    closeFlightlog();
  }
  
}
