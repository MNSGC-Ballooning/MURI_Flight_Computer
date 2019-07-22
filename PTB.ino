void pmsUpdateB() {
    dataPMSB = "";                                                                  //Log sample number, in flight time
    dataPMSB += ntotB;
    dataPMSB += ",";  
  if (readPMSdataB(&PMSBserial)) {
    dataPMSB += PMSBdata.particles_03um;                                            //If data is in the buffer, log it
    dataPMSB += ",";
    dataPMSB += PMSBdata.particles_05um;
    dataPMSB += ",";
    dataPMSB += PMSBdata.particles_10um;
    dataPMSB += ",";
    dataPMSB += PMSBdata.particles_25um;
    dataPMSB += ",";
    dataPMSB += PMSBdata.particles_50um;
    dataPMSB += ",";
    dataPMSB += PMSBdata.particles_100um;
    
    nhitsB=nhitsB+1;                                                                 //Log sample number, in flight time
    
    ntotB = ntotB+1;                                                                 //Total samples

    goodLogB = true;                                                                //If data is successfully collected, note the state;
    badLogB = 0;

  } else {
    badLogB++;                                                                      //If there are five consecutive bad logs, not the state;
    if (badLogB == 5){
      goodLogB = false;
      dataPMSB += '%' + ',' + 'Q' + ',' + '=' + ',' + '!' + ',' + '@' + ',' + '$';
    }
  }
}


/////////////////////User Defined Functions////////////////////////////

boolean readPMSdataB(Stream *s) {
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
  memcpy((void *)&PMSBdata, (void *)buffer_u16, 30);
 
  if (sum != PMSBdata.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
