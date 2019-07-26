//bool DataUpdate=true;
//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
 static unsigned long prevTime = 0;
 if(millis()-prevTime>=3000){

  prevTime = millis();
  
  UpdateRelays();                                           // Update the relay states

  GetTemperature();                                         // Update the temperatures from the four sensors

  GetPressure();                                            // Update and calculate pressure from analog Honeywell sensor

  GetGPSAltitude();                                         // Update the GPS altitude in alt_GPS

  SDLog();                                                  // Log everything to the SD card
  
  }  
  
}
  
  
