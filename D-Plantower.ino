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
    if (badLog >= 5){
      goodLog = false;
      dataPMS += '%' + ',' + 'Q' + ',' + '=' + ',' + '!' + ',' + '@' + ',' + '$';
    }
  }
}

/////////////////////User Defined Functions////////////////////////////

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
