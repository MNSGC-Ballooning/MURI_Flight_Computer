//============================================================================================================================================
//               MURI Flight Computer
//               Written by Patrick James Collins (PJ) - coll0792 Summer 2019
//               Edited by Asif Ally (AA)  - allyx004 Summer 2019
//               OPC Library and OLED Written by Nathan Pharis (NP) phari009 and Jacob Meiners (JM) meine042 Summer 2019   
//               SMART Library Written by Vinchenzo Nguyen (VN) Summer 2019
//               In Memory of Garrett Ailts (GA) - ailts008 Summer of '69 (nice)
//============================================================================================================================================

//Version Description: MURI Flight Computer for a testing configuration. Controls fake balloon flight using a finite state machine and logs payload/atmospheric data.
//Switches states based on ascent rate

//Use: There are two switches and a plug to fully activate the payload. Switches should be flipped in order from right to left. Switch one powers the motherboard, switch two
//powers the microcontroller, and the plug initializes the recovery siren.

//=============================================================================================================================================
//=============================================================================================================================================
/*
                                                                                  
 ad88888ba                                                                        
d8"     "8b                                         ,d                            
Y8,                                                 88                            
`Y8aaaaa,    8b,dPPYba,    ,adPPYba,   ,adPPYba,  MM88MMM  ,adPPYba,  8b,dPPYba,  
  `"""""8b,  88P'    "8a  a8P_____88  a8"     ""    88    a8P_____88  88P'   "Y8  
        `8b  88       d8  8PP"""""""  8b            88    8PP"""""""  88          
Y8a     a8P  88b,   ,a8"  "8b,   ,aa  "8a,   ,aa    88,   "8b,   ,aa  88          
 "Y88888P"   88`YbbdP"'    `"Ybbd8"'   `"Ybbd8"'    "Y888  `"Ybbd8"'  88          
             88                                                                   
             88                                                                   
                                                                                                 
*/                                                                                                                                                                                                                                                                           
//=============================================================================================================================================
//=============================================================================================================================================

/*  Teensy 3.5/3.6 pin connections:
     ------------------------------------------------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used             | Notes
       
     Logging LED (BLUE)           | 22                    | Blinks everytime flight log is opened to log data
     Onboard SD Reader            | None                  | On board SD card reader uses a dedicated SPI bus
     Temperature Sensors (3)      | 28-30                 | DS18B20 temp sensors uses one wire digital communication
     OPC Power Relay              | 3,4                   | Digital pins that serve as the on and off pins for the opc heater relay
     Battery Heater Relay         | 5,6                   | Digital pins that serve as the on and off pins for opc heater relay
     Honeywell Pressure Sensor    | A9                    | Analog pins
 
     UBLOX GPS                    | RX1,TX1 (1,2)         | UART bus 1 (Serial1)   
     RFD900                       | RX2,TX2 (9,10)        | UART bus 2 (Serial2) 
     SPS30 Particle Sensor A      | RX3,TX3 (7,8)         | UART bus 3 (Serial3)
     SPS30 Particle Sensor B      | RX4,TX4 (31,32)       | UART bus 4 (Serial4)
     HPM Particle Sensor          | RX5,TX5 (34,35)       | UART bus 5 (Serial5)
     
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
#include <OPCSensor.h>                                                 //Library for OPCs
#include <Wire.h>                                                      //Library for I2C
#include <SFE_MicroOLED.h>                                             //Library for OLED
#include <RelayXBee.h>                                                 //Library for RFD900
#include <Adafruit_MAX31856.h>                                         //Adafruit Library

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
#define UBLOX_SERIAL Serial1                                           //Serial Pins
#define RFD_SERIAL Serial2
#define SPS_SERIALA Serial3
#define SPS_SERIALB Serial4
#define HPM_SERIAL Serial5
#define PIN_RESET 17                                                   //The library assumes a reset pin is necessary. The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is not being used
#define RFD_BAUD 38400
/////////////////////////////
//////////Constants//////////
/////////////////////////////
//TIMERS
#define CONTROL_LOOP_TIME 1000                                         //Control loop runs at 1.0 Hz
#define STATE_LOG_TIMER 4000                                           //Log timer runs at 0.25 Hz
#define SCREEN_UPDATE_RATE 1000                                        //Rate of the screen updates
#define MASTER_TIMER 300                                               //Ultimate Release Timer
#define LOW_MAX_ALTITUDE_CUTDOWN_TIMER 10                              //Release SMARTs after 10 minutes if max alt is less than 80000ft
#define LONG_ASCENT_TIMER 240                                          //SMARTs release if ascent takes longer than 4 hours
#define LONG_DESCENT_TIMER 90                                          //SMARTS release if descent takes longer than 1.5 hours
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
#define EASTERN_BOUNDARY -92.3                                         //Longitude of Waterloo, IA
#define WESTERN_BOUNDARY -97.4                                         //Longitude of Yankton, SD
#define NORTHERN_BOUNDARY 45.6                                         //Latitude of St. Cloud, MN
#define SOUTHERN_BOUNDARY 41.6                                         //Latitude of Des Moines, IA 
#define MAX_ALTITUDE  110000
#define MIN_ALTITUDE  80000                                            //Minimum altitude of slow descent

static bool SwitchedState = false;

//On Board SD Chipselect
const int chipSelect = BUILTIN_SDCARD;                                 //On board SD card for teensy

//RFD900//
String packet;

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
float t1,t2,t3,t4,t5;                                                  //Temperature vvalues


//Honeywell Pressure Sensor
int pressureSensor;                                                    //Analog number given by sensor
float pressureSensorVoltage;                                           //Voltage calculated from analog number
float PressurePSI;                                                     //PSI calculated from voltage
float PressureATM;                                                     //ATM calculated from PSI

//GPS
UbloxGPS GPS(&UBLOX_SERIAL);
uint8_t FixStatus= NoFix;
float alt_GPS = 0;                                                     //Altitude calculated by the GPS in feet
float prev_alt_feet = 0;                                               //Previous calculated altitude

//Thermocouple
Adafruit_MAX31856 thermocouple = Adafruit_MAX31856(15);                //Thermocouple temperature sensor

///////////////////////////////////////////
/////////////////Control///////////////////
///////////////////////////////////////////
//SMART
static String SmartLogA = "";                                          //Log everytime, is just data from smart
static String SmartLogB = "";


//Control Telemetry
float ascent_rate = 0;                                                 //Ascent rate of payload in feet per minute
float Control_Altitude = 0;                                            //Final altitude used between alt_GPS, alt_pressure_library, and time predicted altitude depending on if we have a GPS lock
static float prev_time = 0;                                            //Prev time for S_Control
static float prev_Control_Altitude = 0;                                //Records the most recent altitude given by GPS when it had lock
static float Initial_Altitude = 0;                                     //Initial altitude set when GPS first gets a fix
static bool FirstAlt = false;                                          //Indicates if Initial_Altitude has been set
int test = 0;

//Timers and Counters
//unsigned long smartTimer = 0;                                          //To time loop speeds and flight times and Automatic Sonde InFo (ASIF) 
unsigned long LowAltitudeReleaseTimer = 0;
unsigned long ascentTimer = 0;
unsigned long descentTimer = 0;
unsigned long masterClock = 0;
unsigned long screenUpdateTimer = 0;
unsigned long oledTime = 0;                                            //Tracks the time of the main loop
unsigned long ControlCounter = 0;
unsigned long StateLogCounter = 0;

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
SPS SPSA(&SPS_SERIALA);                                                 //Establish objects and logging string for the OPCs
SPS SPSB(&SPS_SERIALB);
HPM HPMA(&HPM_SERIAL);

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
  RFD_SERIAL.begin(RFD_BAUD);
  oledPrintAdd(oled, "RFDInit");  
                                           
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
                                                                       //Testing RFD900//
  if(RFD_SERIAL.available()>0){                                        //Checks for any incoming bytes
    delay(5);                                                          //Bytes will be received one at a time unless you add a small delay so the buffer fills with your message
    int incomingBytes = RFD_SERIAL.available();                        //Checks number of total bytes to be read
    Serial.println(incomingBytes);                                     //Just for testing to see if delay is sufficient to receive all bytes.
    for(int i=0; i<incomingBytes; i++)
    {
      packet[i] = RFD_SERIAL.read();                                   //Reads bytes one at a time and stores them in a character array.
    }
    Serial.println(packet);                                            //Prints whole character array
  }
  //////////////////   
  if (millis()-ControlCounter>=CONTROL_LOOP_TIME){                     //Control loop, runs slower, to ease stress on certain tasks
    ControlCounter = millis();

    FixCheck();                                                        //Provide logic for GPS fix
    
   if (millis() - StateLogCounter >= STATE_LOG_TIMER) {
      StateLogCounter = millis();

      stateMachine();                                                    //Update state machine
      updateSensors();                                                   //Updates and logs all sensor data
      actHeat();                                                         //Controls active heating
      oledUpdate();                                                      //Update screen
    } 
  }
}
