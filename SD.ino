boolean eventlogOpen = false;
boolean FlightlogOpen = false;
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
      sendXBee(action + ", " + String(GPS.altitude.feet()) + "ft. Watch your heads!");
    }
    else{
      logAction(action + ", " + flightTimeStr() + "," + String(GPS.location.lat(), 4) + "," + String(GPS.location.lng(), 4) + ", Altitude: " + String(GPS.altitude.feet()) + "ft. NO FIX");
      sendXBee(action + ", " + "altitude unknown" + " Watch your heads!");
    }
  }
