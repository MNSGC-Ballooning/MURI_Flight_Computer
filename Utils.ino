// LED
void actionBlink(){
  digitalWrite(LED_PIN,HIGH);
  delay(10);
  digitalWrite(LED_PIN,LOW);
}

void fixBlink(){
  static unsigned long prevTime = 0;
  if (GPS.getFixAge()<4000){
    if(millis()-prevTime>=15000){
      prevTime = millis();
      digitalWrite(LED_FIX,HIGH);
      delay(10);
      digitalWrite(LED_FIX,LOW);
    }
  }
  else{
      digitalWrite(LED_FIX,HIGH);
      delay(10);
      digitalWrite(LED_FIX,LOW);
  }
   
}

void logBlink(){
  digitalWrite(LED_SD,HIGH);
  delay(10);
  digitalWrite(LED_SD,LOW);
}

// GPS   
int getGPStime() 
{
  return (GPS.getHour() * 3600 + GPS.getMinute() * 60 + GPS.getSecond());
}
//
//// Flight Time
//unsigned long flightTime() {
//  return millis();
//}

//returns the above flight time as a usable string for print statements
String flightTimeStr() {
  unsigned long t = millis() / 1000;
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

float flightMinutes() {
  float minutes = millis() / 1000;
  minutes = minutes / 60;
  return minutes;
}

float masterClockMinutes() {
  float returnclock = masterClock / 1000;
  returnclock = returnclock / 60;
  return returnclock;
}

// SD for Flight Computer
boolean FlightlogOpen = false;

void openFlightlog() {
  if (!FlightlogOpen&&SDcard) {
    //add .c_str() next to Fname
    Flog = SD.open(Fname.c_str(), FILE_WRITE);
    FlightlogOpen = true;
    digitalWrite(LED_SD, HIGH);
  }
}
void closeFlightlog() {
  if (FlightlogOpen&&SDcard) {
    Flog.close();
    FlightlogOpen = false;
    if (!FlightlogOpen)
      digitalWrite(LED_SD, LOW);
  }
}
