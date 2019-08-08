// LED
//void actionBlink(){
//  digitalWrite(LED_PIN,HIGH);
//  delay(10);
//  digitalWrite(LED_PIN,LOW);
//}
//
//void fixBlink(){
//  static unsigned long prevTime = 0;
//  if (GPS.getFixAge()<4000){
//    if(millis()-prevTime>=15000){
//      prevTime = millis();
//      digitalWrite(LED_FIX,HIGH);
//      delay(10);
//      digitalWrite(LED_FIX,LOW);
//    }
//  }
//  else{
//      digitalWrite(LED_FIX,HIGH);
//      delay(10);
//      digitalWrite(LED_FIX,LOW);
//  }
//   
//}
//
//void logBlink(){
//  digitalWrite(LED_SD,HIGH);
//  delay(10);
//  digitalWrite(LED_SD,LOW);
//}

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
    digitalWrite(LED_SD, LOW);
  }
}

void oledOn(MicroOLED &named){ named.command(DISPLAYON); }

void oledOff(MicroOLED &named){ named.command(DISPLAYOFF); }

void oledPrintNew(MicroOLED &named, String message){
  named.clear(PAGE);
  named.setCursor(0, 0);
  named.print(message);
  named.display();
}

void oledPrintAdd(MicroOLED &named, String message){
  named.print(message);
  named.display();
}

void oledUpdate(){
  if (recovery&&!finalMessage[1]){
    oledOn(oled);
    oled.setFontType(0);
    oledPrintNew(oled, "If Found  Call:JamesFlaten    (651).399.2423");
    finalMessage[1] = true;
  } else if ((GPS.getAlt_feet()>2000)&&!finalMessage[0]) {
    oledPrintNew(oled, "");
    oledOff(oled);
    finalMessage[0] = true;
  } else if(millis()-screenUpdateTimer>=SCREEN_UPDATE_RATE){
     String localDataPrint = "";
     
     screenUpdateTimer = millis();

     if (screen == 0) {
      localDataPrint += "GPS:";
      if (GPS.getSats()<10) {
        localDataPrint += '0' + String(GPS.getSats());
      } else {
        localDataPrint += String(GPS.getSats()) + ' ';
      }
      if (oledTime < LOG_TIMER) {
        localDataPrint += "GoodLog";
      } else {
        localDataPrint += "BadLog!";
       }
            
     localDataPrint +=  (MASTER_TIMER*MINUTES_TO_MILLIS-millis());
       
     screen++; 
     oledPrintNew(oled, localDataPrint);
     
     
    } else if (screen == 1) {
      if (SmartLogA != "") {
        localDataPrint += "SMRTA=1";
      } else {
        localDataPrint += "SMRTA=0";
      }
      if (SmartLogB != "") {
        localDataPrint += "SMRTB=1";
      } else {
        localDataPrint += "SMRTB=0";
      }

      if (PlanA.getLogQuality()){
        localDataPrint += "P1";
      } else {
        localDataPrint += "P0";
      }

//      if (SPSA.getLogQuality()){
//        localDataPrint+= "S1";
//      } else {
//        localDataPrint += "S0";
//      }

      if (R1A.getLogQuality()){
        localDataPrint+= "R1";
      } else {
        localDataPrint += "R0";
      }      

       oledPrintNew(oled, localDataPrint);
    }
  }
}
