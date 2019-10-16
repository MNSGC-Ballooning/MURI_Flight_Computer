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
  float returnclock = 0;
  if (hdotInit) {
    returnclock = (millis() - masterClock) / MINUTES_TO_MILLIS; }
  
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

void telemetry(){
  if(RFD_SERIAL.available()>0){                                        //Checks for any incoming bytes
    Serial.println("Packet Recieved!");
    packet = "";
    activeTelemetry = true;
    delay(10);                                                         //Bytes will be received one at a time unless you add a small delay so the buffer fills with your message
    int incomingBytes = RFD_SERIAL.available();                        //Checks number of total bytes to be read
    Serial.println(incomingBytes);                                     //Just for testing to see if delay is sufficient to receive all bytes.
    for(int j=0; j<100; j++)
    {
      packetRecieve[j] = '\0';
    }
    for(int i=0; i<incomingBytes; i++)
    {
      packetRecieve[i] = RFD_SERIAL.read();                              //Reads bytes one at a time and stores them in a character array.
      if (packetRecieve[i] != '\n'){
         packet += packetRecieve[i];
      }
    }
     RFD_SERIAL.print("return " + packet);
  }
}
