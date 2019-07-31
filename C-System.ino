////////////Measurement Update////////////

//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
  
  UpdateRelays();                                           // Update the relay states

  GetTemperature();                                         // Update the temperatures from the four sensors

  GetPressure();                                            // Update and calculate pressure from analog Honeywell sensor

  GetGPSAltitude();                                         // Update the GPS altitude in alt_GPS

  OPCdata = Plan.logUpdate();                               // Update the plantower

  OPCdata += ",=," + Sps.logUpdate();                       // Update the sensirion

  OPCdata += ",=," + r1.logUpdate();                        // Update the Alphasense

  SDLog();                                                  // Log everything to the SD card
   
  
}

//////////////// LED ////////////////

void actionBlink(){
  digitalWrite(Pin_LED,HIGH);
  delay(10);
  digitalWrite(Pin_LED,LOW);
}


void fixBlink(){
  static unsigned long prevTime = 0;
  if (GPS.getFixAge()<4000){
    if(millis()-prevTime>=15000){
      prevTime = millis();
      digitalWrite(Fix_LED,HIGH);
      delay(10);
      digitalWrite(Fix_LED,LOW);
    }
  }
  else{
      digitalWrite(Fix_LED,HIGH);
      delay(10);
      digitalWrite(Fix_LED,LOW);
  }
   
}


void logBlink(){
  digitalWrite(SD_LED,HIGH);
  delay(10);
  digitalWrite(SD_LED,LOW);
}



/////////////// Relays ///////////////

void UpdateRelays() {
  
  if(batHeatRelay.getState()==true){
    batHeat_Status = "ON";
  }
  else if(batHeatRelay.getState()==false){
    batHeat_Status = "OFF";
  }
  
  if(opcHeatRelay.getState()==true){
    opcHeat_Status = "ON";
  }
  else if(opcHeatRelay.getState()==false){
    opcHeat_Status = "OFF";
  }
}



/////////////Flight Time/////////////

//returns the above flight time as usable strings for print statements
String FlightTimeStr() {
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


String FlightTimeMinutes(){
  unsigned long minutes = millis() / 1000;
  minutes = minutes / 60;
  return minutes;
}


/////////////// SMART ///////////////

void SMARTControl() { 
   if (ChangeData){
      SmartLog="";
      SOCO.RequestTemp(1);
      smartTimer=millis();
      while(millis()-smartTimer<150 && SmartLog == "")
      {
        SmartLog=SOCO.Response();
      }

      ChangeData=false;
   }
    
}
