void actHeat(){
  // Compare digital temp. to critical temp.:  
  if(150 < t3 && t3 < t_low) {
    coldBattery = true; // if temperature is below low critical temperature
  }
  if(t3 > t_high) {
    coldBattery = false; // if temperature is above high critical temperature
  }
  if(150 < t2 && t2 < t_low){
    coldSensor = true;    
  }
  if(t2 > t_high){
    coldSensor = false;
  }

//turn heater on/off:
  if(coldBattery && batHeatRelay.getState()==false){
    batHeatRelay.setState(true);
  }
  else if(!coldBattery && batHeatRelay.getState()==true){
    batHeatRelay.setState(false);
  }
  if(coldSensor && sensorHeatRelay.getState()==false){
    sensorHeatRelay.setState(true);
  }
  else if(!coldSensor && sensorHeatRelay.getState()==true){
    sensorHeatRelay.setState(false);
  }
}
