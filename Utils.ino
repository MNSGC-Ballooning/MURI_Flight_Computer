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
  Serial.println("   t1       t2       t3"); 
  Serial.println((String(t1,4) + ", " +String(t2,4) + ", " + String(t3,4)));
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
  Serial.print("Vent Status: ");
  Serial.println(ventStatus,HEX);
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
  Serial.print("Plan B: ");
      if (PlanB.getLogQuality()){                                       //OPC Statuses
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
  Serial.print("SPS B: ");
      if (SpsB.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      }      
  Serial.print("N3 A: ");
      if (N3A.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      }
  Serial.println("Raw data:");          
  Serial.println(OPCdata);
  Serial.println("========================================================================================");  
}

void ventComm(byte commands){
  String packet = "";
  uint16_t checksumCalc = 0;
  byte ventData[16] = {0x00};
  byte checksumData[2] = {0x00};  

  packet += String(0x42) + String(flightMinutes()) + String(alt_GPS) + String(PressurePSI) + String(commands);

  packet.getBytes(ventData, 14);
  for (unsigned short i = 0; i < 14; i++){
    checksumCalc += ventData[i];
  }

  memcpy(&checksumData, &checksumCalc, 2);

  ventData[14] = checksumData[0];
  ventData[15] = checksumData[1];

  for (unsigned short i = 0; i < 16; i++){
    BLUETOOTH_SERIAL.write(ventData[i]);
  }

  delay(10);

  if (BLUETOOTH_SERIAL.read() == 0x42){
    ventConnect = true;
    ventTime = BLUETOOTH_SERIAL.read();
    ventStatus = BLUETOOTH_SERIAL.read();
    resistStatus = BLUETOOTH_SERIAL.read();
    checksumData[0] = BLUETOOTH_SERIAL.read();
    checksumData[1] = BLUETOOTH_SERIAL.read();
    
  } else {
    ventConnect = false;
    ventStatus = 0xFF;
    resistStatus = 0xFF;
    while (BLUETOOTH_SERIAL.available() > 0){
      BLUETOOTH_SERIAL.read();
    }
  }
}
