// SD for Flight Computer
boolean FlightlogOpen = false;

void SDLog() {
  data = "";
  data = flightTimeStr()+ "," + String(GPS.getLat(), 4) + "," + String(GPS.getLon(), 4) + "," 
  + String(alt_GPS, 1) + ","
  + String(GPS.getMonth()) + "/" + String(GPS.getDay()) + "/" + String(GPS.getYear()) + ","
  + String(GPS.getHour()) + ":" + String(GPS.getMinute()) + ":" + String(GPS.getSecond()) + ","
  + String(GPS.getSats()) + ",";
  
  CheckFix();

  data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(PressurePSI) + "," + String(PressureATM) + ",");
  data += (batHeat_Status + "," + opcHeat_Status + ",");
  data += (String(Control_Altitude) + ",");
  data += (SmartLog + "," + String(ascent_rate) + "," + stateString + ",");
  openFlightlog();
  Serial.println(data + "," + "=");

  Serial.println(OPCdata);
  Flog.println(data + ",=," + OPCdata);
  closeFlightlog();

  
  //SMART 
  ChangeData=true; //Telling SmartController that we have logged the data
}





void openFlightlog() {
  if (!FlightlogOpen&&SDcard) {
    Flog = SD.open(Fname.c_str(), FILE_WRITE);
    FlightlogOpen = true;
    digitalWrite(SD_LED, HIGH);
  }
}

void closeFlightlog() {
  if (FlightlogOpen&&SDcard) {
    Flog.close();
    FlightlogOpen = false;
    if (!FlightlogOpen)
      digitalWrite(SD_LED, LOW);
  }
}
