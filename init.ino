void initMS5607(){
  //initialize MS5607
  delay(500);
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
  //initialize the baraometer
  Serial.println("Initilizing Barometer...");
  myBaro.begin();
  startAlt=myBaro.getAltitude();
  //Serial.print("Start Alt: " + startAlt);
  Serial.println("Barometer Initialized. \n");
  Wire.setRate(I2C_RATE_400);
}

void initSD(){
  //initialize SD card
  pinMode(chipSelect, OUTPUT);
  
  while (!SD.begin(chipSelect)) {//power LED will blink if no card is inserted
    Serial.println("No SD");
    digitalWrite(ledSD, HIGH);
    delay(500);
    digitalWrite(ledSD, LOW);
    delay(500);
    SDcard = false;
  }
  SDcard = true;

  //Flight Log
  for (int i = 0; i < 100; i++) {
    Fname = String("FLog" + String(i / 10) + String(i % 10) + ".csv");
    if (!SD.exists(Fname.c_str())) {
      openFlightlog();
      break;
    }
  }
  
  Serial.println("Flight log created: " + Fname);

  String FHeader = "Flight Time, Lat, Long, Altitude (ft), Date, Hour:Min:Sec, Satellites, Fix,Internal Ambient (K), External Ambient (K), Battery (K), OPC (K), OPC Heater Status, Battery Heater Status, MS5607 temperature (K), MS5607 pressure (kPa), MS5607 altitude (ft), Custom Pressure Altitude, Smart Unit, Ascent Rate,";
  Flog.println(FHeader);//set up Flight log format
  Serial.println("Flight log header added");

  closeFlightlog();
}

void initGPS(){
    //initiate GPS
  UBLOX_SERIAL.begin(UBLOX_BAUD);
  GPS.init();
  //Initiate GPS Data lines
  Serial.println("GPS begin");
  delay(50);
  if(GPS.setAirborne()){
    Serial.println("Airbrone mode set!");
  }

  //GPS setup and config
  Serial.println("GPS configured");
}

void initRelays(){
  opcRelay.init(false);
  opcHeatRelay.init(false);
  batHeatRelay.init(false);
  //sirenRelay.init(false);
  delay(100);
  if (opcActive){
    opcRelay.setState(true);
    opcRelay_Status = "ON";
  }
  else{
    opcRelay.setState(false);
    opcRelay_Status = "OFF";
  }
  
  opcHeat_Status = "OFF";
  batHeat_Status = "OFF";
}
