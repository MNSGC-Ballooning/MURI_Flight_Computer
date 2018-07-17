void actHeat(){
  const char ON[] = "on";
  const char OFF[] = "of";
  // Compare digital temp. to critical temp.:  
  if(t3 < t_low) {
    coldBattery = true; // if temperature is below low critical temperature
  }
  if(t3 > t_high) {
    coldBattery = false; // if temperature is above high critical temperature
  }
  if(t4 > t_low){
    coldOPC = true;    
  }
  if(t4 > t_high){
    coldOPC = false;
  }

// turn heater on/off:
  if(coldBattery && strcmp(Bat_heaterStatus.c_str(),OFF)==0){
    digitalWrite(BAT_HEATER_ON, HIGH);
    delay(10);
    digitalWrite(BAT_HEATER_ON, LOW); 
    Bat_heaterStatus = "on";
  }
  else if(!coldBattery && strcmp(Bat_heaterStatus.c_str(),ON)==0){
    digitalWrite(BAT_HEATER_OFF, HIGH);
    delay(10);
    digitalWrite(BAT_HEATER_OFF, LOW);
    Bat_heaterStatus = "off";
  }
  if(coldOPC && strcmp(OPC_heaterStatus.c_str(),OFF)==0){
    digitalWrite(OPC_HEATER_ON, HIGH);
    delay(10);
    digitalWrite(OPC_HEATER_ON, LOW);
    OPC_heaterStatus = "on"; 
  }
  else if(!coldOPC && strcmp(OPC_heaterStatus.c_str(),ON)==0){
    digitalWrite(OPC_HEATER_OFF, HIGH);
    delay(10);
    digitalWrite(OPC_HEATER_OFF, LOW);
    OPC_heaterStatus = "off";
  }
}



