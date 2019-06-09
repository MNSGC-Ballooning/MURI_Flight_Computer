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
  if(coldBattery && batHeatRelay.getState()==false){
    batHeatRelay.setState(true);
  }
  else if(!coldBattery && batHeatRelay.getState()==true){
    batHeatRelay.setState(false);
  }
  if(coldOPC && opcHeatRelay.getState()==false){
    opcHeatRelay.setState(true);
  }
  else if(!coldOPC && opcHeatRelay.getState()==true){
    opcHeatRelay.setState(false);
  }
}
