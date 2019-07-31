//Autonomous LDS Based Extrapolation and Recording Tool (ALBERT)


//==========================================================================
//           MURI Flight Computer
//           Written by Patrick (PJ) Collins - coll0792 July 2019
//           Edited by Asif Ally  - allyx004 Summer 2019                  
//=========================================================================


//Version Description: MURI Flight Computer for single balloon configuration. Controls balloon flight using a finite state machine and logs payload/atmospheric data.
//The system carries an external SMART to artificially switch states and end flight based on GPS fencing, altitude, or a timer.

//Use: There are three switches to activate the payload fully. Switches should be flipped in order from right to left. Switch one powers the motherboard, switch two
//powers the microcontroller, and switch three initializes the recovery siren (technically not a switch, but a plug).

//=============================================================================================================================================
//=============================================================================================================================================

//
//                         ___      _        ______    _____   ______   _____  
//                        / _ \    | |       | ___ \  |  ___|  | ___ \ |_   _| 
//                       / /_\ \   | |       | |_/ /  | |__    | |_/ /   | |   
//                       |  _  |   | |       | ___ \  |  __|   |    /    | |   
//                       | | | | _ | |____ _ | |_/ /_ | |___ _ | |\ \  _ | | _ 
//                       \_| |_/(_)\_____/(_)\____/(_)\____/(_)\_| \_|(_)\_/(_)
//
//
                                                      
//=============================================================================================================================================
//=============================================================================================================================================

/*  Teensy 3.5/3.6 pin connections:
     ------------------------------------------------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used             | Notes
     
     XBee Radio                   | RX3,TX3 (##,##)       | UART bus 3 (Serial3)
     Action LED (BLUE)            | 21                    | LED is activated by 
     Logging LED (RED)            | 23                    | Blinks everytime flight log is opened to log data
     Onboard SD Reader            | None                  | On board SD card reader uses a dedicated on board chip
     Fix LED (GREEN)              | 22                    | "SD" LED (Red). Only on when the file is open in SD card
     Temperature Sensors (4)      | 29-31                 | DS18B20 temp sensors uses one wire digital communication
     OPC Heater Relay             | 24,25                 | Digital pins that serve as the on and off pins for opc heater relay
     Battery Heater Relay         | 5,6                   | Digital pins that serve as the on and off pins for the battery heater relay
     Ublox GPS                    | RX2,TX2 (9,10)        | UART bus 2 (Serial2)
     PMS5003 Particle Sensor      | RX1,TX1 (1,2)         | UART bus 1 (Serial1)
     SPS30 Particle Sensor        | RX5,TX5 (34,35)       | UART bus 5 (Serial5)
     
     ------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

/////////////////////////////////////////////
/////////////////Libraries///////////////////
/////////////////////////////////////////////
#include <SPI.h>                                                           //R1 communication (to be implemented)
#include <SD.h>                                                            //SD logging
#include <LatchRelay.h>                                                    //Latching relays
#include <OneWire.h>                                                       //Dallas data flow control
#include <DallasTemperature.h>                                             //Dallas temperature measurements
#include <UbloxGPS.h>                                                      //GPS System
#include <Arduino.h>                                                       //"Microcontroller stuff" - Garret Ailts 
#include <SmartController.h>                                               //External smart units using xbees to send commands
#include <OPCSensor.h>                                                     //Optical Particle Counters (OPCs)

/////////////////////////////////////////////
///////////////Pin Definitions///////////////
/////////////////////////////////////////////
#define Pin_LED 21                                                         //Pin which controls the DATA LED, which blinks differently depending on what payload is doing
#define Fix_LED 22                                                         //led  which blinks for fix
#define SD_LED 23                                                          //Pin which controls the SD LED
#define ONE_WIRE_BUS 29                                                    //External Temp
#define TWO_WIRE_BUS 30                                                    //Internal Temp
#define THREE_WIRE_BUS 31                                                  //Battery Temp
#define OPC_HEATER_ON 24                                                   //Laching relay pins for the OPC Heater
#define OPC_HEATER_OFF 25
#define BAT_HEATER_ON 5                                                    //Latching relay pins for the battery heater
#define BAT_HEATER_OFF 6
#define HONEYWELL_PRESSURE A16                                             //Pressure sensor, on an analog pin
#define SPS_SERIAL Serial5                                                 //SPS30 
#define XBEE_SERIAL Serial3                                                //XBEE IO
#define UBLOX_SERIAL Serial2                                               //UBLOX IO
#define PMS_SERIAL Serial1                                                 //PLANTOWER IO

/////////////////////////////////////////////
/////////////////Constants///////////////////
/////////////////////////////////////////////
#define MAIN_LOOP_TIME 5000                                                //Main loop runs at .025 Hz
#define CONTROL_LOOP_TIME 1000                                             //Control loop runs at 1 Hz
#define TIMER_RATE (1000)                                                  //Does this have a purpose? Probably not.
#define STATE_INIT_TIME 3600000                                           //Forces state machine into ascent state if it hasn't already happened
#define C2K 273.15                                                         //Celcius to Kelvin adjustment (CHANGE CODE TO CELCIUS)
#define SPECIFIC_GAS_CONSTANT  287                                         //(J/kg*K)
#define SEA_LEVEL_PRESSURE  101325                                         //(Pa)
#define GRAVITY_ACCEL  9.81                                                //(m/s^2)
#define METERS_TO_FEET  3.28084                                            //WOO math
#define PSI_TO_ATM  0.068046                                               //Live love conversions                

/////////////Geographic Boundaries///////////
#define EASTERN_BOUNDARY -92.3                                             //Longitude of Waterloo, IA
#define WESTERN_BOUNDARY -97.4                                             //Longitude of Yankton, SD
#define NORTHERN_BOUNDARY 45.6                                             //Latitude of St. Cloud, MN
#define SOUTHERN_BOUNDARY 41.6                                             //Latitude of Des Moines, IA
#define MAX_ALTITUDE 120000                                                //Altitude (feet) (MN is about 800ft)

//////////////Release Timers/////////////////
#define RELEASER_TIMER 15000                                               //Starting value for active timer that terminates flight when the timer runs out!
#define MASTER_TIMER 20000                                                 //Master cutting value

///////////On Baord SD Chipselect////////////
const int chipSelect = BUILTIN_SDCARD;                                     //On board SD card for teensy

/////////////////////////////////////////////
////////////////Power Relays/////////////////
/////////////////////////////////////////////
LatchRelay opcHeatRelay(OPC_HEATER_ON,OPC_HEATER_OFF);                     //OPC heater relay object
LatchRelay batHeatRelay(BAT_HEATER_ON,BAT_HEATER_OFF);                     //Battery heater relay object
String opcHeat_Status = "";                                                //Status logs
String batHeat_Status = "";

/////////////////////////////////////////////
/////////////Sensor Variables////////////////
/////////////////////////////////////////////
OneWire oneWire1(ONE_WIRE_BUS);                                            //Dallas digital temp sensor data busses
OneWire oneWire2(TWO_WIRE_BUS);
OneWire oneWire3(THREE_WIRE_BUS);
DallasTemperature sensor1(&oneWire1);                                      //Dallas digital temp sensors
DallasTemperature sensor2(&oneWire2);
DallasTemperature sensor3(&oneWire3);

float t1;                                                                  //Temperatures
float t2;
float t3;

////////////////////GPS//////////////////////
UbloxGPS GPS(&UBLOX_SERIAL);                                               //GPS Object
float alt_GPS = 0;                                                         //Altitude calculated by the GPS in feet
float prev_alt_feet = 0;                                                   //Previous calculated altitude

/////////Honeywell Pressure Sensor///////////
int pressureSensor;                                                        //Analog number given by sensor
float pressureSensorVoltage;                                               //Voltage calculated from analog number
float PressurePSI;                                                         //PSI calculated from voltage
float PressureATM;                                                         //ATM calculated from PSI

////////Optical Particle Counters////////////
Plantower Plan(&PMS_SERIAL, 7000);                                         //Particle counter objects
SPS Sps(&SPS_SERIAL);
String OPCdata = "";                                                       //OPC data string

/////////////////////////////////////////////
/////////////////Control/////////////////////
/////////////////////////////////////////////
/////SMART/////
String SmartData;                                                          //Just holds temporary copy of Smart data
static String SmartLog;                                                    //Log everytime, is just data from smart
static bool Cut=false;                                                     //Set to true to cut A SMART
static bool ChangeData=true;                                               //Just set true after every data log
SmartController SOCO = SmartController(1,XBEE_SERIAL,200.0);               //Smart controller
String smartOneString = "Primed";

float ascent_rate = 0;                                                     //Ascent rate of payload in feet per minute
float Control_Altitude = 0;                                                //Final altitude used between alt_GPS, alt_pressure_library, and time predicted altitude depending on if we have a GPS lock
static float prev_time = 0;                                                //Prev time for S_Control
static float prev_Control_Altitude = 0;                                    //Records the most recent altitude given by GPS when it had lock
static bool GPSfix = false;                                                //Determines if the GPS has a fix or not

/////Timers/////
unsigned long releaseTimer = RELEASER_TIMER * 1000;                        //Master Timers                  
unsigned long masterTimer = MASTER_TIMER * 1000;
boolean recovery = false;
unsigned long smartTimer = 0;                                              //Loop speed timers
unsigned long controlCounter = 0;
unsigned long mainCounter = 0;
unsigned long StateInitTimer = 0;                                          //State Machine timer

/////Heating/////
float t_low = 283;                                                         //Active heating boundaries and booleans
float t_high = 289;
boolean coldBattery = false;
boolean coldOPC = false;

//////////////////////////////////////////
///////////////Data Logging///////////////
//////////////////////////////////////////
String stateString = "";                                                   //Data strings and the flight log                            
File Flog;
static String data;
String Fname = "";
boolean SDcard = true;

//////////////////////////////////////////////
/////////Initialize Flight Computer///////////
//////////////////////////////////////////////
void setup() {
  initLEDs();                                                              //Initialize LEDs
  initSD();                                                                //Initialize SD
  initSerial();                                                            //Initialize Serial
  initRadio();                                                             //Initialize Radio
  initGPS();                                                               //Initialize GPS
  initTempSensors();                                                       //Initialize Temp Sensors
  initRelays();                                                            //Initialize Relays
  initOPCs();                                                              //Init OPCs
  
  Serial.println("Setup Complete");
}
                                                                           

void loop(){
  UpdateGPS();                                                             //Update the GPS with new values
  Plan.readData();                                                         //Update the plantower Data
  
  if (millis()-mainCounter>=MAIN_LOOP_TIME){                               //Logging Loop
    mainCounter = millis();
    actionBlink();                                                         //Indicates loop activity
    fixBlink();                                                            //Indicates if a fix is present
    updateSensors();                                                       //Updates and logs all sensor data
    actHeat();                                                             //Controls active heating
  }
  
  if (millis()-controlCounter>=CONTROL_LOOP_TIME){                         //Control loop
    controlCounter = millis();
    SMARTControl();                                                        //SMART system
    SOCO.Cut(1,Cut);                                                       //Sends cut command if Cut=true, else nothing
    stateMachine();                                                        //State machine update
  }   
}
