//============================================================================================================================================
//               MURI Flight Computer
//               Written by PJ Collins (PJ) - coll0792 Fall 2019
//               Edited by Asif Ally (AA)  - allyx004 Fall 2019
//               OPC Library and OLED Written by Nathan Pharis (NP) phari009 and Jacob Meiners (JM) meine042 Summer 2019   
//               SMART Library Written by Vinchenzo Nguyen (VN) Summer 2019
//               In Memory of Garrett Ailts (GA) - ailts008 Summer of '69 (nice)
//============================================================================================================================================

//Version Description: MURI Flight Computer for single or double balloon configuration. Controls balloon flight using a finite state machine and logs payload/atmospheric data.
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
     
     XBee Radio                   | RX1,TX1 (1,2)         | UART bus 1 (Serial1)
     Logging LED (RED)            | 22                    | Blinks everytime flight log is opened to log data
     Onboard SD Reader            | None                  | On board SD card reader uses a dedicated SPI bus
     Temperature Sensors (3)      | 28-30                 | DS18B20 temp sensors uses one wire digital communication
     OPC Heater Relay             | 5,6                   | Digital pins that serve as the on and off pins for opc heater relay
     Battery Heater Relay         | 3,4                   | Digital pins that serve as the on and off pins for the battery heater relay
     Ublox GPS                    | RX2,TX2 (9,10)        | UART bus 2 (Serial2)
     PMS5003 Particle Sensor      | RX3,TX3 (7,8)         | UART bus 3 (Serial3)
     SPS30 Particle Sensor        | RX4,TX4 (31,32)       | UART bus 4 (Serial4)
     OLED                         | I2C0 (18,19)          | I2C bus 1

     
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
#include <Adafruit_MAX31856.h>                                         //Adafruit Library
#include <RelayXBee.h>                                                 //Library for RFD900

////////////////////////////////////
//////////Pin Definitions///////////
////////////////////////////////////
#define LED_SD 22                                                      //Pin which controls the SD LED
#define ONE_WIRE_BUS 26                                                //Battery Temp
#define TWO_WIRE_BUS 27                                                //Internal Temp
#define THREE_WIRE_BUS 28                                              //External Temp
#define FOUR_WIRE_BUS 29
#define SENSOR_HEATER_ON 3                                             //Latching Relay pins for heaters
#define SENSOR_HEATER_OFF 4
#define BAT_HEATER_ON 5
#define BAT_HEATER_OFF 6
#define HONEYWELL_PRESSURE A0                                          //Analog Honeywell Pressure Sensor
#define N3A_SLAVE_PIN 15                                               //Chip Select pin for SPI for the R1
#define SPSA_SERIAL Serial1                                            //Serial Pins
#define UBLOX_SERIAL Serial2
#define SPSB_SERIAL Serial5                                           
#define PMSB_SERIAL Serial4
#define PMSA_SERIAL Serial3   
#define BLUETOOTH_SERIAL Serial6                                         
#define PIN_RESET 17                                                   //The library assumes a reset pin is necessary. The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is not being used
#define RFD_BAUD 38400
#define N3A_SLAVE_PIN 21

/////////////////////////////
//////////Constants//////////
/////////////////////////////
//TIMERS
#define CONTROL_LOOP_TIME 1000                                         //Control loop runs at 1.0 Hz
#define STATE_LOG_TIMER 4000                                           //Log timer runs at 0.25 Hz
#define TEST_LOG_TIMER 15000                                           
#define LONG_TEST_TIMER 300000
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
#define TERMINATE 0x54
#define OPEN_VENT 0x4F
#define CLOSE_VENT  0x43

//Dimensional Boundaries
#define EASTERN_BOUNDARY -93.22                                        //Longitude of Owatonna, MN
#define WESTERN_BOUNDARY -94.63                                        //Longitude of St. James, MN
#define NORTHERN_BOUNDARY 44.43                                        //Latitude of Montgomery, MN
#define SOUTHERN_BOUNDARY 43.76                                        //Latitude of Clark's Grove, MN 
#define MAX_ALTITUDE  100000
#define MIN_ALTITUDE  80000                                            //Minimum altitude of slow descent

static bool SwitchedState = false;
static boolean FlightlogOpen = false;                                   //SD for Flight Computer

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
float t1,t2,t3 = -127.00;                                                  //Temperature values

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
//Venting
bool ventConnect = false;
byte ventStatus = 0xFF;
byte resistStatus = 0xFF;
uint16_t ventTime = 0;
bool resistorCut = false;
String cutReason = "";
bool ventOpen = false;
byte ventCommand = 0x00;

//Control Telemetry
float ascent_rate = 0;                                                 //Ascent rate of payload in feet per minute
float avg_ascent_rate = 0;                                             //Average ascent rate of payload
float Control_Altitude = 0;                                            //Final altitude used between alt_GPS, alt_pressure_library, and time predicted altitude depending on if we have a GPS lock
float Begin_Altitude = 0;                                              //Altitude intitlialized when the state machine initializes
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
Plantower PlanA(&PMSA_SERIAL, STATE_LOG_TIMER);                        //Establish objects and logging string for the OPCs 
Plantower PlanB(&PMSB_SERIAL, STATE_LOG_TIMER);
SPS SpsA(&SPSA_SERIAL);  
SPS SpsB(&SPSB_SERIAL);   
R1 N3A(N3A_SLAVE_PIN);
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
  BLUETOOTH_SERIAL.begin(9600);
  oledPrintAdd(oled, "XB Init");


  initGPS();                                                           //Initialize GPS
  oledPrintAdd(oled, "GPSInit");
  delay(1000);

  initRelays();                                                        //Initialize Relays
  oledPrintAdd(oled, "RlyInit");

  initOPCs();                                                          //Initialize OPCs
  oledPrintAdd(oled, "OPCInit");
  delay(1000);

  initTemp();                                                          //Initialize Temp Sensors
  oledPrintNew(oled, "TmpInit");
  
  Serial.println("Setup Complete");
  oledPrintNew(oled, " Setup Complet");
}

void loop(){
  GPS.update();                                                        //Update GPS and plantower on private loops
  PlanA.readData();
  PlanB.readData();
     
  if (millis()-ControlCounter>=CONTROL_LOOP_TIME){                     //Control loop, runs slower, to ease stress on certain tasks
    ControlCounter = millis();
    
    FixCheck();                                                        //Provide logic for GPS fix
  }
    
  if (millis() - StateLogCounter >= STATE_LOG_TIMER) {
      StateLogCounter = millis();

      stateMachine();                                                    //Update state machine
      updateSensors();                                                   //Updates and logs all sensor data
      actHeat();                                                         //Controls active heating
      oledUpdate();                                                      //Update screen
  }   
}
