void pmsUpdateA() {
    dataPMSA = "";                                                                  //Log sample number, in flight time
    dataPMSA += ntotA;
    dataPMSA += ",";  
  if (readPMSdataA(&PMSAserial)) {
    dataPMSA += PMSAdata.particles_03um;                                            //If data is in the buffer, log it
    dataPMSA += ",";
    dataPMSA += PMSAdata.particles_05um;
    dataPMSA += ",";
    dataPMSA += PMSAdata.particles_10um;
    dataPMSA += ",";
    dataPMSA += PMSAdata.particles_25um;
    dataPMSA += ",";
    dataPMSA += PMSAdata.particles_50um;
    dataPMSA += ",";
    dataPMSA += PMSAdata.particles_100um;
    
    nhitsA=nhitsA+1;                                                                 //Log sample number, in flight time
    
    ntotA = ntotA+1;                                                                 //Total samples

    goodLogA = true;                                                                //If data is successfully collected, note the state;
    badLogA = 0;

  } else {
    badLogA++;                                                                      //If there are five consecutive bad logs, not the state;
    if (badLogA = 5){
      goodLogA = false;
      dataPMSA += '%' + ',' + 'Q' + ',' + '=' + ',' + '!' + ',' + '@' + ',' + '$';
    }
  }
}

/////////////////////User Defined Functions////////////////////////////

boolean readPMSdataA(Stream *s) {
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
  memcpy((void *)&PMSAdata, (void *)buffer_u16, 30);
 
  if (sum != PMSAdata.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
