void initLEDs() {
  pinMode(Pin_LED, OUTPUT);
  pinMode(SD_LED, OUTPUT);
  pinMode(Fix_LED, OUTPUT);
}

void initSD(){
  //initialize SD card
  pinMode(chipSelect, OUTPUT);
  
  while (!SD.begin(chipSelect)) {//power LED will blink if no card is inserted
    Serial.println("No SD");
    digitalWrite(SD_LED, HIGH);
    delay(500);
    digitalWrite(SD_LED, LOW);
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

  String FHeader = "Flight Time, Lat, Long, Altitude (ft), Date, Hour:Min:Sec, Satellites, Fix, External Ambient (K), Internal Ambient (K), Battery (K), Pressure(PSI), Pressure(ATM), Battery Heater Status, PMSB Heater Status, MS5607 temperature (K), MS5607 pressure (kPa), MS5607 altitude (ft), Control Altitude, Smart A, Smart B, Ascent Rate, System State";
  FHeader+= "PT, nhits, 0.3um, 0.5um, 1.0um, 2.5um, 5.0um, 10.0um,SPS,nhits,MC 1um,MC 2.5um,MC 4um,MC 10um,NC 0.5um,NC 1.0um,NC 2.5um,NC 4.0um,NC 10.0um,APS";
  Flog.println(FHeader);//set up Flight log format
  Serial.println("Flight log header added");

  closeFlightlog();
}

void initSerial() {
  Serial.begin(9600); //USB Serial for debugging
 
  PMS_SERIAL.begin(9600);   delay(100);
  SPS_SERIAL.begin(115200);                                                     //Serial initializations for SPS and computer.
}

void initRadio() {
  XBEE_SERIAL.begin(9600); //For smart xBee
  
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
  opcHeatRelay.init(false);
  batHeatRelay.init(false);
  
  opcHeat_Status = "OFF";
  batHeat_Status = "OFF";
}

void initTempSensors() {
  sensor1.begin();
  sensor2.begin();
  sensor3.begin();
}

void initOPCs() {
  Plan.initOPC();
  Sps.initOPC();
}
