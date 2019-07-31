//Initializations
void initLEDs() {
  pinMode(Pin_LED, OUTPUT);                                                         // LED pins defined
  pinMode(SD_LED, OUTPUT);
  pinMode(Fix_LED, OUTPUT);
}

void initSD(){
  pinMode(chipSelect, OUTPUT);                                                      // Initialize SD card
  
  while (!SD.begin(chipSelect)) {                                                   // Power LED will blink if no card is inserted
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
    Fname = String("FLog" + String(i / 10) + String(i % 10) + ".csv");              // Creates a flight log based off of available names
    if (!SD.exists(Fname.c_str())) {
      openFlightlog();
      break;
    }
  }
  
  Serial.println("Flight log created: " + Fname);

  String FHeader = "Flight Time,Minutes,Lat,Long,Altitude (ft),Date,Hour:Min:Sec,Satellites,Fix,External Ambient (K),Internal Ambient (K),Battery (K),";
  FHeader+= "Pressure(PSI),Pressure(ATM),Battery Heater Status,PMS Heater Status,Control Altitude,Smart,Ascent Rate,System State,Balloon Burst,";
  FHeader+= "PT,nhits,0.3um,0.5um,1.0um,2.5um,5.0um,10.0um,SPS,nhits,MC 1um,MC 2.5um,MC 4um,MC 10um,NC 0.5um,NC 1.0um,NC 2.5um,NC 4.0um,NC 10.0um,APS,";
  FHeader+= "R1,nhits,00.4,00.7,01.1,01.5,01.9,02.4,03,04,05,06,07,08,09,10,11,12,12.4";
  Flog.println(FHeader);//set up Flight log format
  Serial.println("Flight log header added");

  closeFlightlog();
}

void initSerial() {
  Serial.begin(9600); //USB Serial for debugging
 
  PMS_SERIAL.begin(9600);   delay(100);
  SPS_SERIAL.begin(115200);                                                         // Serial initializations for SPS and computer.
}


void initRadio() {
  XBEE_SERIAL.begin(9600); //For smart xBee                                         // Initialization for XBEE radio that talks to external SMART box  
}


void initGPS(){                                                                     // Initializes UBLOX GPS system
  UBLOX_SERIAL.begin(UBLOX_BAUD);
  GPS.init();
  delay(50);
  if(GPS.setAirborne()){
    Serial.println("Airbrone mode set!");
  }
}


void initRelays(){                                                                  // Initializes the relays used for turning on the heaating pads
  opcHeatRelay.init(false);
  batHeatRelay.init(false);
  
  opcHeat_Status = "OFF";
  batHeat_Status = "OFF";
}

void initTempSensors() {                                                            // Initilaizes that Dallas temperature sensors 
  sensor1.begin();
  sensor2.begin();
  sensor3.begin();
}

void initOPCs() {                                                                   // Initializes the PLANTOWER and SPS30 OPCs
  Plan.initOPC();
  Sps.initOPC();
  r1.initOPC();
}
