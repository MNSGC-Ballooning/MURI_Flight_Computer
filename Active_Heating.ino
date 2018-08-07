void actHeat(){
  const char ON[] = "on";
  const char OFF[] = "off";
  // Compare digital temp. to critical temp.:  
  if(150 < t3 < t_low) {
    coldBattery = true; // if temperature is below low critical temperature
  }
  if(t3 > t_high) {
    coldBattery = false; // if temperature is above high critical temperature
  }
  if(150 < t4 < t_low){
    coldOPC = true;    
  }
  if(t4 > t_high){
    coldOPC = false;
  }

//turn heater on/off:
  if(coldBattery && strcmp(Bat_heaterStatus.c_str(),OFF)==0){
    batHeatRelay.openRelay();
    Bat_heaterStatus = "on";
  }
  else if(!coldBattery && strcmp(Bat_heaterStatus.c_str(),ON)==0){
    batHeatRelay.closeRelay();
    Bat_heaterStatus = "off";
  }
  if(coldOPC && strcmp(OPC_heaterStatus.c_str(),OFF)==0){
    opcHeatRelay.openRelay();
    OPC_heaterStatus = "on"; 
  }
  else if(!coldOPC && strcmp(OPC_heaterStatus.c_str(),ON)==0){
    opcHeatRelay.closeRelay();
    OPC_heaterStatus = "off";
  }
}



