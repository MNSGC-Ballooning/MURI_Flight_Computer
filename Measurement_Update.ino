//bool DataUpdate=true;
//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateSensors() {
 static unsigned long prevTime = 0;
 if(millis()-prevTime>=5000){
  prevTime = millis();
  
  if(opcRelay.getState()==true){
    opcRelay_Status = "ON";
  }
  else if(opcRelay.getState()==false){
    opcRelay_Status = "OFF";
  }
  if(batHeatRelay.getState()==true){
    batHeat_Status = "ON";
  }
  else if(batHeatRelay.getState()==false){
    batHeat_Status = "OFF";
  }
  if(opcHeatRelay.getState()==true){
    opcHeat_Status = "ON";
  }
  else if(opcHeatRelay.getState()==false){
    opcHeat_Status = "OFF";
  }

  sensor1.requestTemperatures();
  sensor2.requestTemperatures();
  sensor3.requestTemperatures();
  sensor4.requestTemperatures();



  //Pressure, Temp, and Altitude

  t1 = sensor1.getTempCByIndex(0) + 273.15;
  t2 = sensor2.getTempCByIndex(0) + 273.15;
  t3 = sensor3.getTempCByIndex(0) + 273.15;
  t4 = sensor4.getTempCByIndex(0) + 273.15;

  myBaro.baroTask();
  pressure = myBaro.getPressure()/10;
  temperature = myBaro.getTemperature()+C2K;

  alt_GPS = GPS.getAlt_feet();                                // altitude calulated by the Ublox GPS
  alt_pressure_library = myBaro.getAltitude()*METERS_TO_FEET;   // altitude calcuated by the pressure sensor library
  alt_pressure = Pressure_Alt_Calc(pressure*1000, t2);               // altitude calculated by the Hypsometric formula using pressure sensor data
  //openFlightlog();
  data = "";
  data = flightTimeStr()+ "," + String(GPS.getLat(), 4) + "," + String(GPS.getLon(), 4) + "," 
  + String(alt_GPS, 1) + ","
  + String(GPS.getMonth()) + "/" + String(GPS.getDay()) + "/" + String(GPS.getYear()) + ","
  + String(GPS.getHour()) + ":" + String(GPS.getMinute()) + ":" + String(GPS.getSecond()) + ","
  + String(GPS.getSats()) + ",";
  
  //GPS should update once per second, if data is more than 2 seconds old, fix was likely lost
  if(GPS.getFixAge() > 4000){
    data += "No Fix,";
    //fixU == false;
  }
  else{
    data += "Fix,";
    //fixU == true;
  }


  
  data += (String(t1) + "," +String(t2) + "," + String(t3) + "," + String(t4) + ",");
  data += (batHeat_Status + "," + opcHeat_Status + ",");
  data += (String(temperature)+ ",");
  data += (String(pressure) + ",");
  data += (String(alt_pressure_library) + ",");
  data += (String(alt_pressure) + ",");
  data += (SmartLog + "," + String(ascent_rate) + "," + stateString + ",");

//    DataUpdate=false;
  openFlightlog();
  Serial.println(data);
  pmsUpdate();
  Serial.println(dataPMS);
  Flog.println(data + dataPMS);
  closeFlightlog();

//SMART 
  ChangeData=true; //Telling SmartController that we have logged the data

  if (!tempA){
    SOCO.RequestTemp(1);
    tempA=true;
  }
  else {
    SOCO.RequestTemp(2);
    tempA=false;
    }
  }  
  
}
  

void pmsUpdate() {
    dataPMS = "";                                                                  //Log sample number, in flight time
    dataPMS += ntot;
    dataPMS += ",";  
  if (readPMSdata(&PMSserial)) {
    dataPMS += PMSdata.particles_03um;                                            //If data is in the buffer, log it
    dataPMS += ",";
    dataPMS += PMSdata.particles_05um;
    dataPMS += ",";
    dataPMS += PMSdata.particles_10um;
    dataPMS += ",";
    dataPMS += PMSdata.particles_25um;
    dataPMS += ",";
    dataPMS += PMSdata.particles_50um;
    dataPMS += ",";
    dataPMS += PMSdata.particles_100um;
    
    nhits=nhits+1;                                                                 //Log sample number, in flight time
    
    ntot = ntot+1;                                                                 //Total samples

    goodLog = true;                                                                //If data is successfully collected, note the state;
    badLog = 0;

  } else {
    badLog++;                                                                      //If there are five consecutive bad logs, not the state;
    if (badLog = 5){
      goodLog = false;
      dataPMS += '%' + ',' + 'Q' + ',' + '=' + ',' + '!' + ',' + '@' + ',' + '$';
    }
  }
}  

boolean readPMSdata(Stream *s) {
  if (! s->available()) {
//    Serial.println("Serial is not available");
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
//    Serial.println("Peek is not available");
    s->read();
    return false;
  }
 
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }

  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
  // put it into a nice struct :)
  memcpy((void *)&PMSdata, (void *)buffer_u16, 30);
 
  if (sum != PMSdata.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
