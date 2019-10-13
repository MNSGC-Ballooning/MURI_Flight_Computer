//============================================================================================================================================
//               MURI Flight Computer
//               Written by Patrick James Collins (PJ) - coll0792 Summer 2019
//               Edited by Asif Ally (AA)  - allyx004 Summer 2019
//               OPC Library and OLED Written by Nathan Pharis (NP) phari009 and Jacob Meiners (JM) meine042 Summer 2019   
//               SMART Library Written by Vinchenzo Nguyen (VN) Summer 2019
//               In Memory of Garrett Ailts (GA) - ailts008 Summer of '69 (nice)
//============================================================================================================================================

//Version Description: MURI Flight Computer for double balloon configuration. Controls balloon flight using a finite state machine and logs payload/atmospheric data.
//Switches states based on ascent rate
//
//Use: There are two switches and a plug to fully activate the payload. Switches should be flipped in order from right to left. Switch one powers the motherboard, switch two
//powers the microcontroller, and the plug initializes the recovery siren.
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
     
     XBee Radio                   | RX3,TX3 (7,8)         | UART bus 3 (Serial3)
     Logging LED (BLUE)           | 23                    | Blinks everytime flight log is opened to log data
     Onboard SD Reader            | None                  | On board SD card reader uses a dedicated SPI bus
     Temperature Sensors (3)      | 28-30                 | DS18B20 temp sensors uses one wire digital communication
     OPC Power Relay              | 5,6                   | Digital pins that serve as the on and off pins for the opc power relay
     OPC Heater Relay             | 24,25                 | Digital pins that serve as the on and off pins for opc heater relay
     Battery Heater Relay         | 7,8                   | Digital pins that serve as the on and off pins for the battery heater relay
     Ublox GPS                    | RX2,TX2 (9,10)        | UART bus 2 (Serial2)
     PMS5003 Particle Sensor      | RX1,TX1 (1,2)         | UART bus 1 (Serial1)
     SPS30 Particle Sensor        | RX4,TX4 (31,32)       | UART bus 4 (Serial4)

     
     ------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
/////////////////////////////
//////////Libraries//////////
/////////////////////////////
#include <SPI.h>                                                       //SPI Library for R1
#include <SD.h>                                                        //SD Library for logging
#include <LatchRelay.h>                                                //Latching Relay Library
#include <OneWire.h>                                                   //Dallas Data Bus Library
#include <DallasTemperature.h>                                         //Dallas Sensor Library
#include <UbloxGPS.h>                                                  //GPS Library
#include <Arduino.h>                                                   //"Microcontroller stuff" - Garrett Ailts 
#include <SmartController.h>                                           //Library for smart units using xbees to send commands
#include <OPCSensor.h>                                                 //Library for OPCs
#include <Wire.h>                                                      //Library for I2C
#include <SFE_MicroOLED.h>                                             //Library for OLED

////////////////////////////////////
//////////Pin Definitions///////////
////////////////////////////////////
#define LED_SD 22                                                      //Pin which controls the SD LED
#define ONE_WIRE_BUS 28                                                //Battery Temp
#define TWO_WIRE_BUS 29                                                //Internal Temp
#define THREE_WIRE_BUS 30                                              //External Temp
#define SENSOR_HEATER_ON 3                                             //Latching Relay pins for heaters
#define SENSOR_HEATER_OFF 4
#define BAT_HEATER_ON 5
#define BAT_HEATER_OFF 6
#define HONEYWELL_PRESSURE A9                                          //Analog Honeywell Pressure Sensor
#define R1A_SLAVE_PIN 15                                               //Chip Select pin for SPI for the R1
#define PMSA_SERIAL Serial1                                            //Serial Pins
#define UBLOX_SERIAL Serial2                                           
#define XBEE_SERIAL Serial3                                            
#define PMSB_SERIAL Serial4
#define HPMA_SERIAL Serial5
#define PIN_RESET 17                                                   //The library assumes a reset pin is necessary. The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is not being used

/////////////////////////////
//////////Constants//////////
/////////////////////////////
//TIMERS
#define CONTROL_LOOP_TIME 1000                                         //Control loop runs at 1.0 Hz
#define STATE_LOG_TIMER 4000                                           //Log timer runs at 0.25 Hz
#define SCREEN_UPDATE_RATE 1000                                        //Rate of the screen updates
#define MASTER_TIMER 270                                               //Ultimate Release Timer
#define LOW_MAX_ALTITUDE_CUTDOWN_TIMER 10                              //Release SMARTs after 10 minutes if max alt is less than 80000ft
#define LONG_ASCENT_TIMER 210                                          //SMARTs release if ascent takes longer than 4 hours
#define LONG_DESCENT_TIMER 60                                          //SMARTS release if descent takes longer than 1.5 hours
#define SLOW_ASCENT_ABORT_DELAY 20                                     //Time needed to wait until abort procedures b/c of slow ascent are activiated

//Constants
#define DC_JUMPER 1                                                    //The DC_JUMPER is the I2C Address Select jumper. Set to 1 if the jumper is open (Default), or set to 0 if it's closed.
#define MINUTES_TO_MILLIS 60000                                        //MATH TIME
#define PSI_TO_ATM  0.068046                                           //Live love conversions   
#define C2K 273.15                                                     //Celsius to Kelvin. What else is there to say?    
#define PMS_TIME 1                                                     //PMS Timer
#define SPECIFIC_GAS_CONSTANT      287                                 //(J/kg*K)
#define SEA_LEVEL_PRESSURE         101325                              //(Pa)
#define GRAVITY_ACCEL              9.81                                //(m/s^2)
#define METERS_TO_FEET             3.28084
#define Fix        0x00                                                //0000 0000
#define NoFix      0x01                                                //0000 0001

//Control Constants
#define HIGH_TEMP 16                                                   //Heater temperatures
#define LOW_TEMP  10

//Dimensional Boundaries
#define EASTERN_BOUNDARY -91.9                                         //Longitude of Independence, IA
#define WESTERN_BOUNDARY -96.45                                        //Longitude of MN-SD Border
#define NORTHERN_BOUNDARY 45.6                                         //Latitude of St. Cloud, MN
#define SOUTHERN_BOUNDARY 42.5                                         //Latitude of Waterloo, IA 
#define MAX_ALTITUDE  110000
#define MIN_ALTITUDE  80000                                            //Minimum altitude of slow descent

static bool SwitchedState = false;

//On Board SD Chipselect
const int chipSelect = BUILTIN_SDCARD;                                 //On board SD card for teensy

////////////////////////////////
//////////Power Relays//////////
////////////////////////////////
LatchRelay sensorHeatRelay(SENSOR_HEATER_ON,SENSOR_HEATER_OFF);        //Declare latching relay objects and related logging variables
LatchRelay batHeatRelay(BAT_HEATER_ON,BAT_HEATER_OFF);
String sensorHeat_Status = "";
String batHeat_Status = "";

////////////////////////////////////
//////////Sensor Variables//////////
////////////////////////////////////
//Dallas Digital Temp Sensors
OneWire oneWire1(ONE_WIRE_BUS);                                        //Temperature sensor wire busses
OneWire oneWire2(TWO_WIRE_BUS);
OneWire oneWire3(THREE_WIRE_BUS);
DallasTemperature sensor1(&oneWire1);                                  //Temperature sensors
DallasTemperature sensor2(&oneWire2);
DallasTemperature sensor3(&oneWire3);
float t1;                                                              //Temperature vvalues
float t2;
float t3;

//Honeywell Pressure Sensor
float pressureSensor;                                                  //Analog number given by sensor
float pressureSensorVoltage;                                           //Voltage calculated from analog number
float PressurePSI;                                                     //PSI calculated from voltage
float PressureATM;                                                     //ATM calculated from PSI

//GPS
UbloxGPS GPS(&UBLOX_SERIAL);
uint8_t FixStatus= NoFix;
float alt_GPS = 0;                                                     //Altitude calculated by the GPS in feet
float prev_alt_feet = 0;                                               //Previous calculated altitude

///////////////////////////////////////////
/////////////////Control///////////////////
///////////////////////////////////////////
//SMART
String SmartData;                                                      //Just holds temporary copy of Smart data
static String SmartLogA = "";                                          //Log everytime, is just data from smart
static String SmartLogB = "";
static bool CutA=false;                                                //Set to true to cut A SMART
static bool CutB=false;                                                //Set to true to cut B SMART
static bool ChangeData=true;                                           //Just set true after every data log
SmartController SOCO = SmartController(2,XBEE_SERIAL,200.0);           //Smart controller
String smartOneString = "Primed";
String smartTwoString = "Primed";
String smartOneCut = "";
String smartTwoCut = "";

//Control Telemetry
float ascent_rate = 0;                                                 //Ascent rate of payload in feet per minute
float Control_Altitude = 0;                                            //Final altitude used between alt_GPS, alt_pressure_library, and time predicted altitude depending on if we have a GPS lock
static float prev_time = 0;                                            //Prev time for S_Control
static float prev_Control_Altitude = 0;                                //Records the most recent altitude given by GPS when it had lock
static float Initial_Altitude = 0;                                     //Initial altitude set when GPS first gets a fix
static bool FirstAlt = false;                                          //Indicates if Initial_Altitude has been set
int test = 0;

//Timers and Counters
unsigned long smartTimer = 0;                                          //To time loop speeds and flight times and Automatic Sonde InFo (ASIF) 
unsigned long LowAltitudeReleaseTimer = 0;
unsigned long ascentTimer = 0;
unsigned long descentTimer = 0;
unsigned long screenUpdateTimer = 0;
unsigned long oledTime = 0;                                            //Tracks the time of the main loop
unsigned long ControlCounter = 0;
unsigned long StateLogCounter = 0;
static float masterClock = 0;                                          //Needs to be a float as this value is logged

//Timer Booleans
boolean LowMaxAltitude = false;                                        //Set to true if balloon releases before 80,000 feet
boolean recovery = false;

//Heating
boolean coldBattery = false;
boolean coldSensor = false;

////////////////////////////////
//////////Data Logging//////////
////////////////////////////////
String stateString = "";                                               //Variables needed to establish the flight log
File Flog;
static String data;
String Fname = "";
boolean SDcard = true;

////////////////////////
//////////OPCs//////////
////////////////////////
Plantower PlanA(&PMSA_SERIAL, STATE_LOG_TIMER);                               //Establish objects and logging string for the OPCs
Plantower PlanB(&PMSB_SERIAL, STATE_LOG_TIMER);
HPM HPMA(&HPMA_SERIAL);
R1 R1A(R1A_SLAVE_PIN);

String OPCdata = "";

////////////////////////////////////////////////
//////////MicroOLED Object Declaration//////////
////////////////////////////////////////////////
MicroOLED oled(PIN_RESET, DC_JUMPER);                                  //Object I2C declaration
bool finalMessage[2] = {false,false};
unsigned short screen = 0;

//////////////////////////////////////////////
//////////Initialize Flight Computer//////////
//////////////////////////////////////////////
void setup() {
  analogReadResolution(13);

  initOLED(oled);                                                      //Initialize OLED Screen
  pinMode(LED_SD, OUTPUT);                                             //Initialize LED
  
  initSD();                                                            //Initialize SD
  oledPrintNew(oled, "SD Init");

  Serial.begin(9600);                                                  //USB Serial for debugging
  XBEE_SERIAL.begin(9600);                                             //Initialize Radio
  oledPrintAdd(oled, "XB Init");

  initGPS();                                                           //Initialize GPS
  oledPrintAdd(oled, "GPSInit");
  delay(1000);
  
  initTemp();                                                          //Initialize Temp Sensors
  oledPrintNew(oled, "TmpInit");

  initRelays();                                                        //Initialize Relays
  oledPrintAdd(oled, "RlyInit");

  initOPCs();                                                          //Initialize OPCs
  oledPrintAdd(oled, "OPCInit");
    delay(1000);
  
  Serial.println("Setup Complete");
  oledPrintNew(oled, " Setup Complet");
}

void loop(){
  GPS.update();                                                        //Update GPS and plantower on private loops
  PlanA.readData();
  PlanB.readData();
  SmartUpdate();                                                       //System to update SMART Units
     
  if (millis()-ControlCounter>=CONTROL_LOOP_TIME){                     //Control loop, runs slower, to ease stress on certain tasks
    ControlCounter = millis();
    
    SOCO.Cut(1,CutA);                                                  //Cut command logic for SMART
    SOCO.Cut(2,CutB);
    FixCheck();                                                        //Provide logic for GPS fix
  }
    
   if (millis() - StateLogCounter >= STATE_LOG_TIMER) {
      StateLogCounter = millis();

      Serial.println(PlanA.logUpdate());
      stateMachine();                                                    //Update state machine
      updateSensors();                                                   //Updates and logs all sensor data
      actHeat();                                                         //Controls active heating
      oledUpdate();                                                      //Update screen
    } 
  
  
}
