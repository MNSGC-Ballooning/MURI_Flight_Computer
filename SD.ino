boolean eventlogOpen = false;
boolean FlightlogOpen = false;
boolean eDatlogOpen = false;
//The following functions handle both opening files and controlling the data indicator LED

void openEventlog() {
  if (!eventlogOpen&&SDcard) {
    eventLog = SD.open(Ename, FILE_WRITE);
    eventlogOpen = true;
    digitalWrite(ledSD, HIGH);
  }
}
void closeEventlog() {
  if (eventlogOpen&&SDcard) {
    eventLog.close();
    eventlogOpen = false;
    if (!eventlogOpen)
      digitalWrite(ledSD, LOW);
  }
}
void openFlightlog() {
  if (!FlightlogOpen&&SDcard) {
    Flog = SD.open(Fname, FILE_WRITE);
    FlightlogOpen = true;
    digitalWrite(ledSD, HIGH);
  }
}

void closeFlightlog() {
  if (FlightlogOpen&&SDcard) {
    Flog.close();
    FlightlogOpen = false;
    if (!FlightlogOpen)
      digitalWrite(ledSD, LOW);
  }
}

void openeDatlog() {
  if (!eDatlogOpen&&SDcard) {
    eDatLog = SD.open(Dname, FILE_WRITE);
    eDatlogOpen = true;
    digitalWrite(ledSD, HIGH);
  }
}

void closeeDatlog() {
  if (eDatlogOpen&&SDcard) {
    eDatLog.close();
    eDatlogOpen = false;
    if (!eDatlogOpen)
      digitalWrite(ledSD, LOW);
  }
}

void writeEvents() {
  static unsigned long prevTime = 0;
  //if(millis()-prevTime>3000){
    prevTime=millis();
    String eventString = "";
    eventString += flightTimeStr() + "," + stateString + ",";
    eventString += hDOT.getHDot() + "," + tickTock.getDuration() + ",";
    eventString += String(opcRelay.getRelayStatus()) + "," + String(opcHeatRelay.getRelayStatus()) + "," + String(batHeatRelay.getRelayStatus()) + ",";
    eventString += smartOneString + "," + smartTwoString;
    
    if (SDcard){
      openEventlog();
      eventLog.println(eventString);
      closeEventlog();
    }
  //}
}

//Takes a string describing any event that takes place and records it in the eventlog with a timestamp. 
void logAction(String event) {
  if(SDcard){
  openEventlog();
  eventLog.println(flightTimeStr() + "  AC  " + event);
  closeEventlog();
  }
}

void GPSaction(String action){
    if(GPS.Fix){   //GPS.fix
      logAction(action + ", " + flightTimeStr() + "," + String(GPS.location.lat(), 4) + "," + String(GPS.location.lng(), 4) + ", Altitude: " + String(GPS.altitude.feet()) + "ft. FIX");  
    }
    else{
      logAction(action + ", " + flightTimeStr() + "," + String(GPS.location.lat(), 4) + "," + String(GPS.location.lng(), 4) + ", Altitude: " + String(GPS.altitude.feet()) + "ft. NO FIX");
    }
  }

