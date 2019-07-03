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
