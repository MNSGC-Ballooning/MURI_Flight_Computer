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
  named.print("SICKO  MODE   YEET");
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

  String FHeader = "Flight Time, Minutes, Lat, Long, Altitude (ft), Date, Hour:Min:Sec, Satellites, Fix, IntTemp (C), ExtTemp (C), BatTemp (C), Pressure (PSI), Pressure (ATM),";
  FHeader += "Battery Heater Status, Sensor Heater Status, Control Altitude, Smart A, Smart A Cut Reason, Smart B, Smart B Cut Reason, Ascent Rate, System State,";
  FHeader += "PTA, nhits, pt1_bin1, pt1_bin2, pt1_bin3, pt1_bin4, pt1_bin5, pt1_bin6,";
  FHeader += "SPSA, nhits, MC1.0, MC2.5, MC4.0, MC10.0, NC0.5, NC1.0, NC2.5, NC4.0, NC10.0, Average Particle Size,";
  FHeader += "R1A, nhits, 0.4u, 0.7u, 1.1u, 1.5u, 1.9u, 2.4u, 3.0u, 4.0u, 5.0u, 6.0u, 7.0u, 8.0u, 9.0u, 10.0u, 11.0u, 12.0u, 12.4u";
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
    Serial.println("Airbrone mode set!");
  }
  Serial.println("GPS configured");
}

void initTemp(){
  sensor1.begin();                                                    //Initialize Temp Sensors
  sensor2.begin();
  sensor3.begin();
}

void initRelays(){
  sensorHeatRelay.init(false);                                          //Initialize relays
  batHeatRelay.init(false);
  
  sensorHeat_Status = "OFF";
  batHeat_Status = "OFF";
}

void initOPCs() {                                                       //Sets up serial and initializes the OPCs
  PMS_SERIAL.begin(9600);
//SPS_SERIAL.begin(115200);

  PlanA.initOPC();
//SPSA.initOPC();
  R1A.initOPC(); 
}
