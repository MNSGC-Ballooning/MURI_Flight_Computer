

//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
  static unsigned long prevTime = 0;
  if(millis() - prevTime >= 5000){
    prevTime = millis();
    sensor1.requestTemperatures();
    sensor2.requestTemperatures();
    sensor3.requestTemperatures();
    sensor4.requestTemperatures();
    //MS5607 temp and pressure
    MS5.ReadProm();
    MS5.Readout();
    ms_temp = (MS5.GetTemp()/100);    //because temp is given in .01 C
    ms_pressure = MS5.GetPres();
    t1 = sensor1.getTempCByIndex(0) + 273.15;
    t2 = sensor2.getTempCByIndex(0) + 273.15;
    t3 = sensor3.getTempCByIndex(0) + 273.15;
    t4 = sensor4.getTempCByIndex(0) + 273.15;
    String data = "";
    openFlightlog();
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
    data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + ",");
    data += (String(batHeatRelay.getRelayStatus()) + "," + String(opcHeatRelay.getRelayStatus()) + ",");
    data += (String(ms_temp)+ ",");
    data += (String(ms_pressure) + ",");
    

    Serial.println(data);
    Flog.println(data);
    closeFlightlog();
  }
  
}


