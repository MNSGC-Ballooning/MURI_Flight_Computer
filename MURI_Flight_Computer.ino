//Libraries
//this requires a special modded version of the TinyGPS library because for whatever
//reason the TinyGPS library does not include a "Fix" variable. the library can be found here:
//https://github.com/simonpeterson/TinyGPS/tree/master 
#include <SPI.h>
#include <SD.h>
#include <LatchRelay.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <UbloxGPS.h>              //Needs TinyGPS++ in order to function
#include <i2c_t3.h>                //Required for usage of MS5607 with Teensy 3.5/3.6
#include <Arduino.h>               //"Microcontroller stuff" - Garrett Ailts 
#include "Salus_Baro.h"            //Library for MS5607
#include <SmartController.h>       //Library for smart units using xbees to send commands
#include <OPCSensor.h>             //Library for OPCs
//#include <SoftwareSerial.h>        //Software Serial library for Plan Tower

//==============================================================
//               MURI Flight Computer
//               Written by Asif Ally  - allyx004 Summer 2019
//               Edited by Patrick James Collins (PJ) coll0792 Summer 2019   
//==============================================================

//Version Description: MURI Flight Computer for double balloon configuration. Controls balloon flight using a finite state machine and logs payload/atmospheric data.
//Switches states based on ascent rate
//
// Use: There are three switches to activate the payload fully. Switches should be flipped in order from right to left. Switch one powers the motherboard, switch two
//powers the microcontroller, and switch three initializes the recovery siren (does not turn it on, this is done by the microcontroller in the recovery state).
//
//=============================================================================================================================================
//=============================================================================================================================================


//                  /$$$$$$  /$$$$$$  /$$$$$$  /$$   /$$  /$$$$$$        /$$      /$$  /$$$$$$  /$$$$$$$  /$$$$$$$$      
//                 /$$__  $$|_  $$_/ /$$__  $$| $$  /$$/ /$$__  $$      | $$$    /$$$ /$$__  $$| $$__  $$| $$_____/      
//                | $$  \__/  | $$  | $$  \__/| $$ /$$/ | $$  \ $$      | $$$$  /$$$$| $$  \ $$| $$  \ $$| $$            
//                |  $$$$$$   | $$  | $$      | $$$$$/  | $$  | $$      | $$ $$/$$ $$| $$  | $$| $$  | $$| $$$$$         
//                 \____  $$  | $$  | $$      | $$  $$  | $$  | $$      | $$  $$$| $$| $$  | $$| $$  | $$| $$__/         
//                 /$$  \ $$  | $$  | $$    $$| $$\  $$ | $$  | $$      | $$\  $ | $$| $$  | $$| $$  | $$| $$            
//                |  $$$$$$/ /$$$$$$|  $$$$$$/| $$ \  $$|  $$$$$$/      | $$ \/  | $$|  $$$$$$/| $$$$$$$/| $$$$$$$$      
//                 \______/ |______/ \______/ |__/  \__/ \______/       |__/     |__/ \______/ |_______/ |________/      
                                                                                                       
                                                                                                       
                                                                                                       

                                                                                                                                                                         
//=============================================================================================================================================
//=============================================================================================================================================

/*  Teensy 3.5/3.6 pin connections:
     ------------------------------------------------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used             | Notes
     
     XBee Radio                   | RX5,TX5 (34,35)       | UART bus 5 (Serial5)
     Action LED (BLUE)            | 21                    | LED is activated by 
     Logging LED (RED)            | 23                    | Blinks everytime flight log is opened to log data
     Onboard SD Reader            | None                  | On board SD card reader uses a dedicated SPI bus
     Fix LED (GREEN)              | 22                    | "SD" LED (Red). Only on when the file is open in SD card
     Temperature Sensors (4)      | 29-32                 | DS18B20 temp sensors uses one wire digital communication
     OPC Power Relay              | 5,6                   | Digital pins that serve as the on and off pins for the opc power relay
     OPC Heater Relay             | 24,25                 | Digital pins that serve as the on and off pins for opc heater relay
     Battery Heater Relay         | 7,8                   | Digital pins that serve as the on and off pins for the battery heater relay
     Ublox GPS                    | RX2,TX2 (9,10)        | UART bus 2 (Serial2)
     PMS5003 Particle Sensor      | RX1,TX1 (1,2)         | UART bus 1 (Serial1)
     MS5607                       | SCL0,SDA0 (18,19)     | I2C communication for MS5607
     
     ------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

/////////////////////////////////////////////
///////////////Pin Definitions///////////////
/////////////////////////////////////////////
#define LED_PIN 20                  //Pin which controls the DATA LED, which blinks differently depending on what payload is doing
#define LED_FIX 21                  //led  which blinks for fix
#define LED_SD 22                   //Pin which controls the SD LED
#define ONE_WIRE_BUS 28             //Battery Temp
#define TWO_WIRE_BUS 29             //Internal Temp
#define THREE_WIRE_BUS 30           //External Temp
#define SENSOR_HEATER_ON 3
#define SENSOR_HEATER_OFF 4
#define BAT_HEATER_ON 5
#define BAT_HEATER_OFF 6
#define HONEYWELL_PRESSURE A9       //Analog Honeywell Pressure Sensor
#define R1A_SLAVE_PIN 15
#define PMS_SERIAL Serial1
#define UBLOX_SERIAL Serial2
#define XBEE_SERIAL Serial3
#define SPS_SERIAL Serial4



//////////////////////////////////////////////
/////////////////Constants////////////////////
//////////////////////////////////////////////

//TIMERS
#define MAIN_LOOP_TIME 1000                         // Main loop runs at 1 Hz
#define CONTROL_LOOP_TIME 1000                      // Control loop runs at 1.0 Hz
#define LOG_TIMER 4000                              // Log timer runs at 0.25 Hz
#define MASTER_TIMER 300                            // Ultimate Release Timer
#define LOW_MAX_ALTITUDE_CUTDOWN_TIMER 10           // Release SMARTs after 10 minutes if max alt is less than 80000ft
#define LONG_ASCENT_TIMER 240                       // SMARTs release if ascent takes longer than 4 hours
#define LONG_DESCENT_TIMER 90                       // SMARTS release if descent takes longer than 1.5 hours

//Constants
#define MINUTES_TO_MILLIS 60000
#define PSI_TO_ATM  0.068046                                //Live love conversions   
#define C2K 273.15                                          //Celsius to Kelvin. What else is there to say?    
#define PMS_TIME 1                                          //PMS Timer

//Control Constants
#define HIGH_TEMP 16
#define LOW_TEMP  10


//Dimensional Boundaries
#define EASTERN_BOUNDARY -92.3                              //Longitude of Waterloo, IA
#define WESTERN_BOUNDARY -97.4                              //Longitude of Yankton, SD
#define NORTHERN_BOUNDARY 45.6                              //Latitude of St. Cloud, MN
#define SOUTHERN_BOUNDARY 41.6                              //Latitude of Des Moines, IA 
#define MAX_ALTITUDE  110000
#define MIN_ALTITUDE  80000

//////////////On Board SD Chipselect/////////////
const int chipSelect = BUILTIN_SDCARD; //On board SD card for teensy

///////////////////////////////////////////////
////////////////Power Relays///////////////////
//////////////////////////////////////////////
//LatchRelay opcRelay(OPC_ON, OPC_OFF);
LatchRelay sensorHeatRelay(SENSOR_HEATER_ON,SENSOR_HEATER_OFF);
LatchRelay batHeatRelay(BAT_HEATER_ON,BAT_HEATER_OFF);
//LatchRelay sirenRelay(SIREN_ON, SIREN_OFF);
//boolean  opcON = false;
//String opcRelay_Status = "";
String sensorHeat_Status = "";
String batHeat_Status = "";

/////////////////////////////////////////////
/////////////Sensor Variables////////////////
/////////////////////////////////////////////
//Dallas Digital Temp Sensors
OneWire oneWire1(ONE_WIRE_BUS);
OneWire oneWire2(TWO_WIRE_BUS);
OneWire oneWire3(THREE_WIRE_BUS);
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);
DallasTemperature sensor3(&oneWire3);
float t1;
float t2;
float t3;


//Honeywell Pressure Sensor
int pressureSensor;                                                        //Analog number given by sensor
float pressureSensorVoltage;                                               //Voltage calculated from analog number
float PressurePSI;                                                         //PSI calculated from voltage
float PressureATM;                                                         //ATM calculated from PSI


//GPS
UbloxGPS GPS(&UBLOX_SERIAL);
//boolean fixU = false;
float alt_GPS = 0;               // altitude calculated by the GPS in feet
float prev_alt_feet = 0;         // previous calculated altitude

///////////////////////////////////////////
/////////////////Control///////////////////
///////////////////////////////////////////
//SMART
String SmartData;                                                       //Just holds temporary copy of Smart data
static String SmartLogA;                                                //Log everytime, is just data from smart
static String SmartLogB;
static bool CutA=false;                                                 //Set to true to cut A SMART
static bool CutB=false;                                                 //Set to true to cut B SMART
static bool ChangeData=true;                                            //Just set true after every data log
SmartController SOCO = SmartController(2,XBEE_SERIAL,200.0);            //Smart controller
String smartOneString = "Primed";
String smartTwoString = "Primed";
String smartOneCut = "";
String smartTwoCut = "";

//Control Telemetry
float ascent_rate = 0;                                                  // ascent rate of payload in feet per minute
float Control_Altitude = 0;                                             // final altitude used between alt_GPS, alt_pressure_library, and time predicted altitude depending on if we have a GPS lock
static float prev_time = 0;                                             // prev time for S_Control
static float prev_Control_Altitude = 0;                                 // records the most recent altitude given by GPS when it had lock
int test =0;

//Timers
boolean recovery = false;
unsigned long smartTimer = 0;
unsigned long LowAltitudeReleaseTimer = 0;
unsigned long ascentTimer = 0;
unsigned long descentTimer = 0;
unsigned long masterClock = 0;


//Timer Booleans
boolean LowMaxAltitude = false;                                          // Set to true if balloon releases before 80,000 feet


//Heating
boolean coldBattery = false;
boolean coldSensor = false;

//////////////////////////////////////////
///////////////Data Logging///////////////
//////////////////////////////////////////
String stateString = "";
File Flog;
static String data;
String Fname = "";
boolean SDcard = true;


//////////OPCs//////////
Plantower PlanA(&PMS_SERIAL, LOG_TIMER);
//SPS SPSA(&SPS_SERIAL);
R1 R1A(R1A_SLAVE_PIN);

String OPCdata = "";


//////////////////////////////////////////////
/////////Initialize Flight Computer///////////
//////////////////////////////////////////////
void setup() {

  // initialize LEDs
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_SD, OUTPUT);
  pinMode(LED_FIX, OUTPUT);

  //Initialize SD
  initSD();

  //Initialize Serial
  Serial.begin(9600); //USB Serial for debugging
 
  
  //Initialize Radio
  XBEE_SERIAL.begin(9600); //For smart xBee


  //Initialize GPS
  initGPS();
  
  //Initialize Temp Sensors
  sensor1.begin();
  sensor2.begin();
  sensor3.begin();


  //Initialize Relays
  initRelays();

  //Initialize OPCs
  initOPCs();
  
  Serial.println("Setup Complete");

}

void loop(){
  static unsigned long controlCounter = 0;
  static unsigned long mainCounter = 0;
  GPS.update();
  PlanA.readData();
  // Main Thread
  if (millis()-mainCounter>=MAIN_LOOP_TIME){
    mainCounter = millis();
    actionBlink();
    fixBlink();
    updateSensors();   //Updates and logs all sensor data
    actHeat();
    
  }
  
  
  // Control Thread

    if (ChangeData){
      SmartLogA="";
      SOCO.RequestTemp(1);
      smartTimer=millis();
      while(millis()-smartTimer<150 && SmartLogA == "")
      {
        SmartLogA=SOCO.Response();
      }

      
      SmartLogB="";
      SOCO.RequestTemp(2);
      smartTimer=millis();
      while(millis()-smartTimer<150 && SmartLogB == "")
      {
        SmartLogB=SOCO.Response();
      }

      ChangeData=false;
      }
    
    
  if (millis()-controlCounter>=CONTROL_LOOP_TIME){
    controlCounter = millis();
    SOCO.Cut(1,CutA);
    SOCO.Cut(2,CutB);
    FixCheck();
    stateMachine();
  } 

}
