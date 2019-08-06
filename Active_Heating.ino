void actHeat(){
  // Compare digital temp. to critical temp.:  
  if(-123 < t3 && t3 < LOW_TEMP) {
    coldBattery = true; // if temperature is below low critical temperature
  }
  if(t3 > HIGH_TEMP) {
    coldBattery = false; // if temperature is above high critical temperature
  }
  if(-123 < t2 && t2 < LOW_TEMP){
    coldSensor = true;    
  }
  if(t2 > HIGH_TEMP){
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
