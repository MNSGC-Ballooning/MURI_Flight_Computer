
//Libraries
//this requires a special modded version of the TinyGPS library because for whatever
//reason the TinyGPS library does not include a "Fix" variable. the library can be found here:
//https://github.com/simonpeterson/TinyGPS/tree/master
#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SparkFun_ADXL345.h>      //accelerometer library
#include <Smart.h>
//#include <NMEAGPS.h>
#include <Relay_XBee.h>
#include <MS5xxx.h>                //library for MS5607 altimeter, temp, pressure sensor
#include <Wire.h>                  //I2C required for the temp sensor
//#include <UbloxGPS.h>


//==============================================================
//               MURI Flight Computer
//               Written by Garrett Ailts - ailts008 Summer 2018                  
//==============================================================

//Version Description: MURI Flight Computer for double balloon configuration. Controls balloon flight using a finite state machine and logs payload/atmospheric data.
//Switches states through the use of a PID controller

// Use: There are three switches to activate the payload fully. Switches should be flipped in order from right to left. Switch one powers the motherboard, switch two
//powers the microcontroller, and switch three initializes the recovery siren (does not turn it on, this is done by the microcontroller in the recovery state).
//
//=============================================================================================================================================
//=============================================================================================================================================


//       _________       __    __     ____                                  __                
//      / ____/ (_)___ _/ /_  / /_   / __ \____ __________ _____ ___  ___  / /____  __________
//     / /_  / / / __ `/ __ \/ __/  / /_/ / __ `/ ___/ __ `/ __ `__ \/ _ \/ __/ _ \/ ___/ ___/
//    / __/ / / / /_/ / / / / /_   / ____/ /_/ / /  / /_/ / / / / / /  __/ /_/  __/ /  (__  ) 
//   /_/   /_/_/\__, /_/ /_/\__/  /_/    \__,_/_/   \__,_/_/ /_/ /_/\___/\__/\___/_/  /____/  
//             /____/                                                                         


//In seconds
long Release_Timer = 21000; //Starting value for active timer that terminates flight when the timer runs out!
long Master_Timer =  36000; //Master cut timer 
long minAlt = 80000; //Default cutdown altitude in feet! Changeable via xBee.
long maxAlt = 120000; //Default max cutdown altitude in feet! Changeable via xBee
int OPC_srate=1400;     //OPC sample rate in milliseconds.

//=============================================================================================================================================
//=============================================================================================================================================

/*  Mega ADK pin connections:
     -------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used             | Notes

     xBee serial                  | D0-1                  | IMPORTANT- is hardware serial (controls xBee and hard serial line), cannot upload with xBee plugged in
     Fireburner                   | D8                    | High to this pin fires tungsten burner
     Data LED (BLUE)              | D3                    | "action" LED (Blue), tells us what the payload is doing
     SD                           | D4, D50-52            | 50-52 do not not have wires but they are used!
     SD LED (RED)                 | D5                    | "SD" LED (Red). Only on when the file is open in SD card
     Continutity Check OUTPUT     | D6                    | Outputs a voltatge for the continuity check
     razorcutter pin              | D7                    | High to this pin spins razor blade
     GPS serial                   | serial 1              | serial for GPS (pins 18 and 19 on the mega
     fix                          | D6                    | whether or not we have a GPS fix, must be used with copernicus GPS unit
     Tempread                     | D9                    | temperature sensor reading
     Accel I2C                    | SDA,SCL               | I2C communication for accelerometer (pins 20 and 21)
     
     -------------------------------------------------------------------------------------------------------------------------
*/

class action {
  protected:
    unsigned long Time;
    String nam;
  public:
    String getName();
};
class Blink: public action {
  protected:
    int ondelay;
    int offdelay;
    int ontimes;
  public:
    friend void blinkMode();
    void BLINK();
    Blink();
    Blink(int on, int off, int times, String NAM, unsigned long tim);
    int getOnTimes();
};
class Relay {
  protected:
    bool isOpen;
    int onPin;
    int offPin;
  public:
    Relay(int on, int off);
    const char* getRelayStatus();
    void init();
    void openRelay();
    void closeRelay();
};
class ACTIVE_TIMER{
  protected:
    int smartUnit;
    unsigned long duration;
    unsigned long starT;
  public:
    ACTIVE_TIMER(int sm,long d,long s);
    String getDuration();
    void hammerTime();
    void updateTimer(float);
    void changeSMART(int);
};
class ASCENT_RATE{
  protected:
    float rate;
    float h_dot;
    float prevh;
    unsigned long prevt;
    float h_dotArr[5];
    float hQ[5];
    unsigned long tQ[5];
    float h_dotQ[5];
    float sum;
  public:
    ASCENT_RATE();
    void updateRate();
    void addHit();
    void checkHit();
    float getRate();
    float geth_dot();
    float getPrevh();
    float getPrevt();
    float getPrevh_dot();
    String getHDot();
    
};


/////////////////////////////////////////////
/////////////////Define Pins/////////////////
/////////////////////////////////////////////
#define ledPin 3          //Pin which controls the DATA LED, which blinks differently depending on what payload is doing
#define chipSelect 4      //SD Card pin
#define ledSD 5           //Pin which controls the SD LED
#define fix_led 6         //led  which blinks for fix
#define ONE_WIRE_BUS 28   //Internal Temp
#define TWO_WIRE_BUS 29   //External Temp
#define THREE_WIRE_BUS 30 //Battery Temp
#define FOUR_WIRE_BUS 31  //OPC Temp
#define OPC_ON 22         //Relay switches
#define OPC_OFF 23
#define OPC_HEATER_ON 24
#define OPC_HEATER_OFF 25
#define BAT_HEATER_ON 26
#define BAT_HEATER_OFF 27
#define SIREN_ON 32
#define SIREN_OFF 33
//#define gps_Serial Serial2
#define xBee_Serial Serial
#define ID "B"
//#define test 33
///////////////////////////////////////////////
////////////////Power Relays///////////////////
///////////////////////////////////////////////
Relay opcRelay(OPC_ON, OPC_OFF);
Relay opcHeatRelay(OPC_HEATER_ON,OPC_HEATER_OFF);
Relay batHeatRelay(BAT_HEATER_ON,BAT_HEATER_OFF);
Relay sirenRelay(SIREN_ON, SIREN_OFF);
boolean  opcON = false;
//////////////////////////////////////////
//////////////Communication///////////////
//////////////////////////////////////////
boolean LEDon = false;
//variables for LED fix blinking time
#define FixDelay 1000
#define noFixDelay 15000
Blink recoveryBlink = Blink(200, 2000, -1, "recoveryBlink", 0);
Blink countdownBlink = Blink(200, 850, -1, "countdownBlink", 0);
Blink* currentBlink = &countdownBlink;
//XBee constructor
XBee xBee = XBee(&xBee_Serial, ID);

/////////////////////////////////////////////
/////////////Sensor Variables////////////////
/////////////////////////////////////////////

//Dallas Digital Temp Sensors
OneWire oneWire1(ONE_WIRE_BUS);
OneWire oneWire2(TWO_WIRE_BUS);
OneWire oneWire3(THREE_WIRE_BUS);
OneWire oneWire4(FOUR_WIRE_BUS);
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);
DallasTemperature sensor3(&oneWire3);
DallasTemperature sensor4(&oneWire4);
float t1;
float t2;
float t3;
float t4;


//GPS
TinyGPSPlus GPS;
//NMEAGPS UBlox;

//Accelerometer
ADXL345 adxl = ADXL345();
boolean shift = false;
int x,y,z;

//HoneyWell Pressure Sensor
//is this necessary or being used?- Simon 
int pressure = 0;
float pressureV = 0;
float psi = 0;
float kpa = 0;

//MS5607 pressure and temperature sensor
MS5xxx MS5(&Wire);
float ms_temp = 0;
float ms_pressure = 0;

///////////////////////////////////////////
//////////////Control System///////////////
///////////////////////////////////////////
//See State_Machine.ino for control logic
//2SMART
ASCENT_RATE hDOT = ASCENT_RATE();
unsigned long beaconTimer= 0;
boolean burnerON = false;
long releaseTimer = Release_Timer * 1000;
long masterTimer = Master_Timer * 1000;
long starty = 0;
boolean recovery = false;
boolean hdotInit=false;
ACTIVE_TIMER tickTock = ACTIVE_TIMER(1,releaseTimer,starty);

//Heating
float t_low = 283;
float t_high = 289;
boolean coldBattery = false;
boolean coldOPC = false;

//////////////////////////////////////////
///////////////Data Logging///////////////
//////////////////////////////////////////
String stateString = "";
File Flog;
String data;
String Fname = "";
File eventLog;
String Ename = "";
//File eDatLog;
//String Dname = "";
String smartOneString = "";
String smartTwoString = "";
boolean SDcard = true;
  

void setup() {
  //Initiate Serial
  Serial.begin(9600);
  
  //Initiate Temp Sensors
  sensor1.begin();
  sensor2.begin();
  sensor3.begin();
  sensor4.begin();

  //initialize MS5607

  //should this be in a while loop? (see test example)
  MS5.connect();
  delay(500);


  
  //Initialize Relays
  opcRelay.init();
  opcHeatRelay.init();
  batHeatRelay.init();
  sirenRelay.init();
  
  opcRelay.closeRelay();
  opcHeatRelay.closeRelay();
  batHeatRelay.closeRelay();
  sirenRelay.closeRelay();
  delay(1000);

  opcRelay.openRelay();
  
  // initialize pins
  pinMode(ledPin, OUTPUT);
  pinMode(ledSD, OUTPUT);
  pinMode(chipSelect, OUTPUT);    // this needs to be be declared as output for data logging to work
  pinMode(fix_led, OUTPUT);

  
  //Initialize SMART
//  smartOne.initialize();
//  smartOneString = "CLOSED";
//  smartTwo.initialize();
//  smartTwoString = "CLOSED";
  
  //initiate GPS
  Serial1.begin(4800);
//  gps_Serial.begin(9600);
 

  Serial.println("xBee begin");
  //Initiate GPS Data lines
  Serial.println("GPS begin");

  //GPS setup and config
  Serial.println("GPS configured");

  adxl.powerOn();
  adxl.setRangeSetting(16);
  adxl.setSpiBit(0);

  //initialize SD card
  while (!SD.begin(chipSelect)) {//power LED will blink if no card is inserted
    Serial.println("No SD");
    digitalWrite(ledSD, HIGH);
    delay(500);
    digitalWrite(ledSD, LOW);
    delay(500);
    SDcard = false;
  }
  SDcard = true;
  Serial.println("Checking for existing file");
  //Check for existing event logs and creates a new one
  for (int i = 0; i < 100; i++) {
    if (!SD.exists("Elog" + String(i / 10) + String(i % 10))) {
      Ename = "Elog" + String(i / 10) + String(i % 10);
      openEventlog();
      break;
    }
  }
  Serial.println("Event log created: " + Ename);

  //Same but for Flight Log
  for (int i = 0; i < 100; i++) {
    if (!SD.exists("FLog" + String(i / 10) + String(i % 10) + ".csv")) {
      Fname = "FLog" + String(i / 10) + String(i % 10) + ".csv";
      openFlightlog();
      break;
    }
  }
  
  Serial.println("Flight log created: " + Fname);

//  for (int i = 0; i < 100; i++) {
//    if (!SD.exists("eDatLog" + String(i / 10) + String(i % 10) + ".csv")) {
//      Dname = "eDatLog" + String(i / 10) + String(i % 10) + ".csv";
//      openeDatlog();
//      break;
//    }
//  }
//
//  Serial.println("Extra data log created: " + Dname);

  String FHeader = "Flight Time, Lat, Long, Altitude (ft), Date, Hour:Min:Sec, Fix, Accel x, Accel y, Accel z, Internal Ambient (K), External Ambient (K), Battery (K), OPC (K), OPC Heater Status, Battery Heater Status, External Pressure (PSI), MS5607 temperature (C), MS5607 pressure (PA)";
  Flog.println(FHeader);//set up Flight log format
  Serial.println("Flight log header added");


  String eventLogHeader = "Flight Time, State, hdot, Active Timer, OPC Relay, OPC Heater Relay, Battery Heater Relay, Smart 1, Smart 2";
  eventLog.println(eventLogHeader);
  Serial.println("Eventlog header added");

//  String eDatLogHeader = "Flight Time, Lat, Long, Altitude (ft), Date, Hour:Min:Sec, Fix";
//  eDatLog.println(eDatLogHeader);
//  Serial.println("eDatLog header added");
  
  closeEventlog();
  closeFlightlog();
  //closeeDatlog();


  Serial.println("Setup Complete");
}
void loop(){
  updateGPS();       //Updates Copernicus GPS
//    if (UBlox.available( gps_Serial )){ 
//     ubloxUpdate( UBlox.read() );} //Updates UBlox GPS
  updateSensors();   //Updates and logs all sensor data
  stateMachine();    //Finite state machine that makes in flight decsions based on GPS data
  //writeEvents();     //Writes event to log
}
