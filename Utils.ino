// LED
void actionBlink(){
  digitalWrite(ledPin,HIGH);
  delay(10);
  digitalWrite(ledPin,LOW);
}

void fixBlink(){
  static unsigned long prevTime = 0;
  if (GPS.getFixAge()<4000){
    if(millis()-prevTime>=15000){
      prevTime = millis();
      digitalWrite(fix_led,HIGH);
      delay(10);
      digitalWrite(fix_led,LOW);
    }
  }
  else{
      digitalWrite(fix_led,HIGH);
      delay(10);
      digitalWrite(fix_led,LOW);
  }
   
}

void logBlink(){
  digitalWrite(ledSD,HIGH);
  delay(10);
  digitalWrite(ledSD,LOW);
}

// GPS   
int getGPStime() 
{
  return (GPS.getHour() * 3600 + GPS.getMinute() * 60 + GPS.getSecond());
}

// Flight Time
unsigned long flightTime() {
  return millis();
}

//returns the above flight time as a usable string for print statements
String flightTimeStr() {
  unsigned long t = flightTime() / 1000;
  String fTime = "";
  fTime += (String(t / 3600) + ":");
  t %= 3600;
  fTime += String(t / 600);
  t %= 600;
  fTime += (String(t / 60) + ":");
  t %= 60;
  fTime += (String(t / 10) + String(t % 10));
  return fTime;
}

// SD for Flight Computer
boolean FlightlogOpen = false;

void openFlightlog() {
  if (!FlightlogOpen&&SDcard) {
    //add .c_str() next to Fname
    Flog = SD.open(Fname.c_str(), FILE_WRITE);
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
//SD for Plan Tower
boolean PMSLogOpen = false;

void openFlightlogPMS() {
  if (!PMSLogOpen&&SDcard) {
    //add .c_str() next to Fname
    PMSLog = SD.open(FnamePMS.c_str(), FILE_WRITE);
    PMSLogOpen = true;
    digitalWrite(ledSD, HIGH);
  }
}

void closeFlightlogPMS() {
  if (PMSLogOpen&&SDcard) {
    PMSLog.close();
    PMSLogOpen = false;
    if (!PMSLogOpen)
      digitalWrite(ledSD, LOW);
  }
}

boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
 
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }

  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
  // put it into a nice struct :)
  memcpy((void *)&PMSdata, (void *)buffer_u16, 30);
 
  if (sum != PMSdata.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
