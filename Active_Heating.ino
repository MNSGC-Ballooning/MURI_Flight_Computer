void actHeat(){
//  const char ON[] = "OPEN";
//  const char OFF[] = "CLOSED";
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
  if(coldBattery && batHeatRelay.getStatus()==false){
    batHeatRelay.setState(true);
  }
  else if(!coldBattery && batHeatRelay.getStatus()==true){
    batHeatRelay.setState(false);
  }
  if(coldOPC && opcHeatRelay.getStatus()==false){
    opcHeatRelay.setState(true);
  }
  else if(!coldOPC && opcHeatRelay.getStatus()==true){
    opcHeatRelay.setState(false);
  }
}
