void actHeat(){
                       // set flight-time variable to Arduino internal clock
  
  ////////// Temperature monitoring ////////// 
  
  
  TempSensors2.requestTemperatures();            // request temp from digital temp sensor...
  t2 = TempSensors2.getTempCByIndex(0);          // digital temp in celcius
  t2 = t2 + 273.15;                         // digital temp in Kelvin

  ////////// Heater operation //////////

  // "test-fire" heater for 3 minutes after Arduino clock has started; NOTE heater does not depend on temperature values during this time!
  if (t<(60*3)){
   digitalWrite(heatBlanket, HIGH); 
   heaterStatus = "on";
  }
  
  // Compare digital temp. to critical temp.:  
  else{
    if (t2 < t_low) {
      hold = 1; // if temperature is below low critical temperature
    }
    if (t2 > t_high) {
      hold = 0; // if temperature is above high critical temperature
     }    

  // turn heater on/off:
    if (hold==1){
    digitalWrite(heatBlanket, HIGH); 
    heaterStatus = "on";
    }
   else {
    digitalWrite(heatBlanket, LOW);
    heaterStatus = "off";
    }  
  }

  ////////// Datalogging //////////
  
  data = "";           
  data += t2;               // log value of digital temp.
  data += ",";
  data += heaterStatus;     // log heater status (either "on" or "off")
  data += ",";
  data += flightTimeStr();    // log flight time; flightTime is a user-defined function

 ////////// Data Writing //////////

 openTemplog();    // open file

 if (tempLog) {
    //Serial.println("tempLog.csv opened...");    // file open successfully 
    tempLog.println(data);
    closeTemplog();
    
  }
  else {
             // file open failed
    return;
  }

 delay(samp_freq); // delay 1 second i.e. do all that every 1 second 

}



