

//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
  static unsigned long prevTime = 0;
  if(millis() - prevTime >= 5000){
    prevTime = millis();
    adxl.readAccel(&x,&y,&z);
    sensor1.requestTemperatures();
    sensor2.requestTemperatures();
    sensor3.requestTemperatures();
    sensor4.requestTemperatures();
    t1 = sensor1.getTempCByIndex(0) + 273.15;
    t2 = sensor2.getTempCByIndex(0) + 273.15;
    t3 = sensor3.getTempCByIndex(0) + 273.15;
    t4 = sensor4.getTempCByIndex(0) + 273.15;
    pressure = analogRead(A0);
    pressureV=pressure*(5.0/1024);
    psi = (pressureV - (0.1*5.0))/(4.0/15.0);
    kpa = psi * 6.89476; //Pressure in kpa
    String data = "";
    String data2 = "";
    openFlightlog();
    openeDatlog();
    data += flightTimeStr() + "," + String(gpsLattitude) + "," + String(gpsLongitude) + "," + String(gpsAltitude*meters2feet) + "," + String(gpsHours) + ":" + String(gpsMinutes) + ":" + String(gpsSeconds) + "," + String(gpsSatellites) + ",";
    if(gpsLattitude!=0 && gpsLongitude!=0){
      data += "fix,";
    }
    else{
      data += "no fix,";
    }
    if(GPS.Fix && GPS.altitude.feet()!=0) {
      data2 += (flightTimeStr() + "," + String(GPS.location.lat(), 6) + "," + String(GPS.location.lng(), 6) + ",");
      data2 += ((String(GPS.altitude.feet())) + ",");    //convert meters to feet for datalogging
      data2 += (String(GPS.date.month()) + "/" + String(GPS.date.day()) + "/" + String(GPS.date.year()) + ",");
      data2 += (String(GPS.time.hour()) + ":" + String(GPS.time.minute()) + ":" + String(GPS.time.second()) + ",");
      data2 += "fix,";
    }
    else{
    data2 += (flightTimeStr() + ",0.000000,0.000000,0.00,0/0/2000,00:00:00,No Fix,");
    
    }
    data += (String(x) + "," + String(y) + "," + String(z) + ","); 
    data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + ",");
    data += (String(batHeatRelay.getRelayStatus()) + "," + String(opcHeatRelay.getRelayStatus()) + ",");
    data += (String(kpa) + ",");

    Serial.println(data);
    Serial.println(data2);
    Flog.println(data);
    eDatLog.println(data2);
    closeFlightlog();
    closeeDatlog();
    writeEvents();
    
  }
  
}


