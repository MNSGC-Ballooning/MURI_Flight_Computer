//////Contains Active Heating, Temperature Sensor, and Pressure Sensor Code//////


//////////ACTIVE HEATING//////////
void actHeat(){
  // Compare digital temp. to critical temp.:  
  if(150 < t3 && t3 < t_low) {
    coldBattery = true; // if temperature is below low critical temperature
  }
  if(t3 > t_high) {
    coldBattery = false; // if temperature is above high critical temperature
  }
  if(150 < t2 && t2 < t_low){
    coldOPC = true;    
  }
  if(t2 > t_high){
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



//////////PRESSURE//////////

void GetPressure() { 
  
  pressureSensor = analogRead(HONEYWELL_PRESSURE);              //Read the analog pin
  pressureSensorVoltage = pressureSensor * (5.0/1024);          //Convert the analog number to voltage
  PressurePSI = (pressureSensorVoltage - (0.1*5.0))/(4.0/15.0); //Convert the voltage to PSI
  PressureATM = PressurePSI*PSI_TO_ATM;                         //Convert PSI reading to ATM
  
}



//////////TEMPERATURE//////////

void GetTemperature() {
  
  sensor1.requestTemperatures();
  sensor2.requestTemperatures();
  sensor3.requestTemperatures();


  t1 = sensor1.getTempCByIndex(0) + C2K;
  t2 = sensor2.getTempCByIndex(0) + C2K;
  t3 = sensor3.getTempCByIndex(0) + C2K;

}
