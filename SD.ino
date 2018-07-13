boolean eventlogOpen = false;
boolean GPSlogOpen = false;
boolean TemplogOpen = false;
//The following functions handle both opening files and controlling the data indicator LED

void openEventlog() {
  if (!eventlogOpen&&SDcard) {
    eventLog = SD.open(Ename, FILE_WRITE);
    eventlogOpen = true;
    digitalWrite(ledSD, HIGH);
  }
}

void openTemplog() {
  if (!TemplogOpen&&SDcard) {
    tempLog = SD.open(temp_filename, FILE_WRITE); // open file
    TemplogOpen = true;
    digitalWrite(ledSD, HIGH);
  }
}

void closeTemplog() {
  if (TemplogOpen&&SDcard) {
    tempLog.close();
    TemplogOpen = false;
    if (!TemplogOpen)
      digitalWrite(ledSD, LOW);
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
void openGPSlog() {
  if (!GPSlogOpen&&SDcard) {
    GPSlog = SD.open(GPSname, FILE_WRITE);;
    GPSlogOpen = true;
    digitalWrite(ledSD, HIGH);
  }
}

void closeGPSlog() {
  if (GPSlogOpen&&SDcard) {
    GPSlog.close();
    GPSlogOpen = false;
    if (!GPSlogOpen)
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

