//void actHeat(){  
//  short batTemp;
//  short sensTemp;
//  
//  if (t3 < -100) {                                                      // t3 is the battery temp sensor. t2 is the internal temp sensor
//    batTemp = t2;
//  } 
//  else {
//    batTemp = t3;
//  }
//
//  if (t2 < -100) {
//    sensTemp = t3;
//  } 
//  else {
//    sensTemp = t2;
//  }
//  
//  if(-100 < batTemp && batTemp < LOW_TEMP) {                            //Compare digital temp. to critical temp.
//    coldBattery = true;                                                 //If temperature is below low critical temperature
//  }
//  if(batTemp > HIGH_TEMP) {
//    coldBattery = false;                                                //If temperature is above high critical temperature
//  }
//  if(-100 < sensTemp && sensTemp < LOW_TEMP){
//    coldSensor = true;    
//  }
//  if(sensTemp > HIGH_TEMP){
//    coldSensor = false;
//  }
//
//  if(coldBattery && batHeatRelay.getState()==false){                     //Turn heater on/off:
//    batHeatRelay.setState(true);
//  }
//  else if(!coldBattery && batHeatRelay.getState()==true){
//    batHeatRelay.setState(false);
//  }
//  if(coldSensor && sensorHeatRelay.getState()==false){
//    sensorHeatRelay.setState(true);
//  }
//  else if(!coldSensor && sensorHeatRelay.getState()==true){
//    sensorHeatRelay.setState(false);
//  }
//}
