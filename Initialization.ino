//Initializations

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

  // CHANGE THE BELOW HEADER

  String FHeader = "Time, Temp (C), Pressure (PSI), Pressure (ATM),";
//  FHeader += "PTA, " + PlanA.CSVHeader();
//  FHeader += ",PTPump, " + PlanB.CSVHeader();
  FHeader += ",SPSA, " + SPSA.CSVHeader();
  FHeader += ",SPSB, " + SPSB.CSVHeader();
  Flog.println(FHeader);                                                //Set up Flight log format
  Serial.println("Flight log header added");                            

  closeFlightlog();
}


void initTemp(){
  sensor1.begin();                                                    //Initialize Temp Sensors
}


void initOPCs() {                                                       //Sets up serial and initializes the OPCs
//  PMSA_SERIAL.begin(9600);
 // PMSB_SERIAL.begin(9600);
  SPSA_SERIAL.begin(115200);
  SPSB_SERIAL.begin(115200);


//  PlanA.initOPC();
//  Serial.println("PlanA Initialized");
//  PlanB.initOPC();
//  Serial.println("PlanPump Initialized");
  SPSA.initOPC();
  Serial.println("SPSA Initialized");
  SPSB.initOPC();
  Serial.println("SPSB Initialized");

}
