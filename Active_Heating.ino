void actHeat(){
  const char ON[] = "OPEN";
  const char OFF[] = "CLOSED";
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
  if(coldBattery && strcmp(batHeatRelay.getRelayStatus().c_str(),OFF)==0){
    batHeatRelay.openRelay();
  }
  else if(!coldBattery && strcmp(batHeatRelay.getRelayStatus().c_str(),ON)==0){
    batHeatRelay.closeRelay();
  }
  if(coldOPC && strcmp(opcHeatRelay.getRelayStatus().c_str(),OFF)==0){
    opcHeatRelay.openRelay();
  }
  else if(!coldOPC && strcmp(opcHeatRelay.getRelayStatus().c_str(),ON)==0){
    opcHeatRelay.closeRelay();
  }
}



