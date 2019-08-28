// GPS   
int getGPStime() 
{
  return (GPS.getHour() * 3600 + GPS.getMinute() * 60 + GPS.getSecond());
}

void FixCheck(){                                                        //Check if gps fix is good
  if (GPS.getFixAge() < 4000) 
  {
    FixStatus = Fix;
  }
  else if(GPS.getFixAge() > 4000)
  {
    FixStatus = NoFix;
  }
  else
  {
    FixStatus = NoFix;
  }
}

void SetFirstAlt() {
  if (GPSstatus == Lock) {
    Initial_Altitude = GPS.getAlt_feet();
    FirstAlt = true;
  }
}

String flightTimeStr() {                                                //Returns the flight time as a usable string for print statements  
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

float flightMinutes() {                                                 //Return time in minutes
  float minutes = millis() / 1000;
  minutes = minutes / 60;
  return minutes;
}

float masterClockMinutes() {                                            //Return master time in minutes
  float returnclock = masterClock / MINUTES_TO_MILLIS;
  returnclock = (millis()/MINUTES_TO_MILLIS) - (returnclock);
  return returnclock;
}

boolean FlightlogOpen = false;                                          //SD for Flight Computer
void openFlightlog() {                                                  //Open flight log
  if (!FlightlogOpen&&SDcard) {
    //add .c_str() next to Fname
    Flog = SD.open(Fname.c_str(), FILE_WRITE);
    FlightlogOpen = true;
    digitalWrite(LED_SD, HIGH);
  }
}
void closeFlightlog() {                                                 //Close flight log
  if (FlightlogOpen&&SDcard) {
    Flog.close();
    FlightlogOpen = false;
    digitalWrite(LED_SD, LOW);
  }
}

void SmartUpdate(){
      if (ChangeData){                                                  //System to request data from Smart A and B
      SmartLogA="";
      SOCO.RequestTemp(1);
      smartTimer=millis();
      while(millis()-smartTimer<150 && SmartLogA == "")
      {
        SmartLogA=SOCO.Response();
      }
      
      SmartLogB="";
      SOCO.RequestTemp(2);
      smartTimer=millis();
      while(millis()-smartTimer<150 && SmartLogB == "")
      {
        SmartLogB=SOCO.Response();
      }

      ChangeData=false;
      }
}
