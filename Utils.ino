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

void printData(){
  Serial.println();
  Serial.println("Measurement Update");
  Serial.println("========================================================================================");
  Serial.println("             Time");
  Serial.print("Flight Time String: ");
  Serial.println(flightTimeStr());
  Serial.print("Flight Minutes: ");
  Serial.println(String(flightMinutes()));
  Serial.print("Master Clock: ");
  Serial.println(String(masterClockMinutes(),2));
  Serial.println("------------------------------");
  Serial.println("             GPS");
  Serial.print("Latitude: ");
  Serial.println(String(GPS.getLat(), 4));
  Serial.print("Longitude: ");
  Serial.println(String(GPS.getLon(), 4));  
  Serial.print("Altitude: ");
  Serial.println(String(alt_GPS, 1));
  Serial.print("Date and Time: ");
  Serial.println(String(GPS.getMonth()) + "/" + String(GPS.getDay()) + "/" + String(GPS.getYear()) + " " + String(GPS.getHour()) + ":" + String(GPS.getMinute()) + ":" + String(GPS.getSecond()));
  Serial.print("Satellites and Fix Age: ");
  Serial.println((String(GPS.getSats()) + ", " + String(GPS.getFixAge())));
  Serial.println("------------------------------");
  Serial.println("          Temperature");
  Serial.println("   t1        t2        t3        t4        t5"); 
  Serial.println((String(t1,4) + ", " +String(t2,4) + ", " + String(t3,4))); // + ", " + String(t4,4) + ", " + String(t5,4)));
  Serial.println("------------------------------");
  Serial.println("           Pressure");
  Serial.print("Pressure(PSI): ");
  Serial.println(String(PressurePSI,6));
  Serial.print("Pressure(ATM): ");
  Serial.println(String(PressureATM,6));
  Serial.println("------------------------------");
  Serial.println("       System Statuses");
  Serial.println("Battery Heater Relay    Sensor Heater Relay");
  Serial.println(("         " + batHeat_Status + "                  " + sensorHeat_Status));
  Serial.print("Control Altitude: ");
  Serial.println(String(Control_Altitude));
  Serial.print("Smart A: ");
  Serial.println((SmartLogA + ", " + smartOneCut));
  Serial.print("Smart B: ");
  Serial.println((SmartLogB + ", " + smartTwoCut));
  Serial.print("Instantaneous Ascent Rate: ");
  Serial.println(String(ascent_rate));
  Serial.print("Average Ascent Rate: ");
  Serial.println(String(avg_ascent_rate));
  Serial.print("State: ");
  Serial.println(stateString);
  Serial.println("------------------------------");
  Serial.println("             OPCs");
  Serial.print("Plan A: ");
      if (PlanA.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      } 
  Serial.print("SPS A: ");
      if (SpsA.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      }
//  Serial.print("R1 A: ");
//      if (R1A.getLogQuality()){                                       //OPC Statuses
//        Serial.println("Good Log");
//      } else {
//        Serial.println("Bad Log");
//      }
//  Serial.print("N3 A: ");
//      if (N3A.getLogQuality()){                                       //OPC Statuses
//        Serial.println("Good Log");
//      } else {
//        Serial.println("Bad Log");
//      }
  Serial.println("Raw data:");          
  Serial.println(OPCdata);
  Serial.println("========================================================================================");  
}

/*
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
}*/

void telemetry(){
  XBEE_SERIAL.print((String(flightMinutes()) + ',' + String(GPS.getLat(), 4) + "," + String(GPS.getLon(), 4) + "," + String(alt_GPS, 1) + "," + String(GPS.getSats()) + "," + String(t1,4) + "," +String(t2,4) + "," + String(t3,4) + "," + String(-127.000) + "," + String("off") + OPCdata));
}


void vent(){
  BLUETOOTH_SERIAL.print("PING");
  delay(15);
  if (BLUETOOTH_SERIAL.read() == "PiNG"){
    ventConnect = true;
  }
}
