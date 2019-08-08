void actHeat(){  
  if(-123 < t3 && t3 < LOW_TEMP) {                                      //Compare digital temp. to critical temp.
    coldBattery = true;                                                 //If temperature is below low critical temperature
  }
  if(t3 > HIGH_TEMP) {
    coldBattery = false;                                                //If temperature is above high critical temperature
  }
  if(-123 < t2 && t2 < LOW_TEMP){
    coldSensor = true;    
  }
  if(t2 > HIGH_TEMP){
    coldSensor = false;
  }

  if(coldBattery && batHeatRelay.getState()==false){                     //Turn heater on/off:
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
