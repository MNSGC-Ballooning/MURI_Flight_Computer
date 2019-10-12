//Initializations
void initOLED(MicroOLED& named){                                        //Initializes the OLED Screen
  delay(100);
  Wire.begin();
  named.begin();                                                        //Initialize the OLED
  named.clear(ALL);                                                     //Clear the display's internal memory
  named.display();                                                      //Display what's in the buffer (splashscreen)
  delay(1000);                                                          //Delay 1000 ms
  named.clear(PAGE);                                                    //Clear the buffer.
  
  named.setFontType(1);
  named.clear(PAGE);
  named.setCursor(0,0);                                                 //Reset
  named.print("ALBERT RunningSpectre");
  named.display();
  delay(2000);
}

void initSD(){
  pinMode(chipSelect, OUTPUT);                                          //initialize SD card
  
  while (!SD.begin(chipSelect)) {                                       //power LED will blink if no card is inserted
    Serial.println("No SD");
    digitalWrite(LED_SD, HIGH);
    delay(500);
    digitalWrite(LED_SD, LOW);
    delay(500);
    SDcard = false;
  }
  SDcard = true;

  for (int i = 0; i < 100; i++) {                                       //Flight Log Name Cration
    Fname = String("FLog" + String(i / 10) + String(i % 10) + ".csv");
    if (!SD.exists(Fname.c_str())) {
      openFlightlog();
      break;
    }
  }
  
  Serial.println("Flight log created: " + Fname);

  String FHeader = "Flight Time, Minutes, Master Clock Minutes, Lat, Long, Altitude (ft), Date, Hour:Min:Sec, Satellites, Fix, BatTemp (C), IntTemp (C),ExtTemp (C), ThermoTempAdj, ThermoTemp, ";
  FHeader += "Pressure (PSI), Pressure (ATM), Battery Heater Status, Sensor Heater Status, Control Altitude, Ascent Rate, System State, Smart A, Smart B, RFDString,";
  FHeader += "SPSA" + ',' + SPSA.CSVHeader() + ',' + "SPSB" + ',' + SPSB.CSVHeader() + ',' + "HPMA" + ',' + HPMA.CSVHeader();
  Flog.println(FHeader);                                                //Set up Flight log format
  Serial.println("Flight log header added");                            

  closeFlightlog();
}

void initGPS(){
  UBLOX_SERIAL.begin(UBLOX_BAUD);                                       //initiate GPS
  GPS.init();                                                           //Initiate GPS Data lines

  Serial.println("GPS begin");
  delay(50);
  if(GPS.setAirborne()){
    Serial.println("Airborne mode set!");
  }
  Serial.println("GPS configured");
}

void initTemp(){
  sensor1.begin();                                                    //Initialize Temp Sensors
  sensor2.begin();
  sensor3.begin();

  thermocouple.begin();
  thermocouple.setThermocoupleType(MAX31856_TCTYPE_K);
}

void initRelays(){
  sensorHeatRelay.init(false);                                          //Initialize relays
  batHeatRelay.init(false);
  
  sensorHeat_Status = "OFF";
  batHeat_Status = "OFF";
}

void initOPCs() {                                                       //Sets up serial and initializes the OPCs
  SPS_SERIALA.begin(115200);
  SPS_SERIALB.begin(115200);
  HPM_SERIAL.begin(9600);

  SPSA.initOPC();
  SPSB.initOPC();
  HPMA.initOPC();
}
