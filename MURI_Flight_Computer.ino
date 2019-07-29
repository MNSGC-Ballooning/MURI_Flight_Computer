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
#include <Arduino.h>               //"Microcontroller stuff" - Garret Ailts 
#include <SmartController.h>       //Library for smart units using xbees to send commands
//#include <SoftwareSerial.h>        //Software Serial library for Plan Tower

//==============================================================
//               MURI Flight Computer
//               Written by Patrick James Collins (PJ) - coll0792 Summer 2019
//               Edited by Asif Ally  - allyx004 Summer 2019                  
//==============================================================

//Version Description: MURI ALBERT Flight Computer for double balloon configuration. Controls balloon flight using a finite state machine and logs payload/atmospheric data.
//Switches states based on ascent rate
//
// Use: There are three switches to activate the payload fully. Switches should be flipped in order from right to left. Switch one powers the motherboard, switch two
//powers the microcontroller, and switch three initializes the recovery siren (does not turn it on, this is done by the microcontroller in the recovery state).
//
//=============================================================================================================================================
//=============================================================================================================================================

//
//                         ___      _        ______    _____   ______   _____  
//                        / _ \    | |       | ___ \  |  ___|  | ___ \ |_   _| 
//                       / /_\ \   | |       | |_/ /  | |__    | |_/ /   | |   
//                       |  _  |   | |       | ___ \  |  __|   |    /    | |   
//                       | | | | _ | |____ _ | |_/ /_ | |___ _ | |\ \  _ | | _ 
//                       \_| |_/(_)\_____/(_)\____/(_)\____/(_)\_| \_|(_)\_/(_)
                                                       
                                                       


//=============================================================================================================================================
//=============================================================================================================================================

//In seconds
long RELEASER_TIMER = 15000; //Starting value for active timer that terminates flight when the timer runs out!
long MASTER_TIMER =  20000; //Master cut timer 

//boolean opcActive = true;

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
#define Pin_LED 21          //Pin which controls the DATA LED, which blinks differently depending on what payload is doing
#define Fix_LED 22          //led  which blinks for fix
#define SD_LED 23           //Pin which controls the SD LED
#define ONE_WIRE_BUS 29     //Internal Temp
#define TWO_WIRE_BUS 30     //External Temp
#define THREE_WIRE_BUS 31   //Battery Temp
#define FOUR_WIRE_BUS 32    //OPC Temp
//#define OPC_ON 5            //Relay switches
//#define OPC_OFF 6
#define OPC_HEATER_ON 24
#define OPC_HEATER_OFF 25
#define BAT_HEATER_ON 5
#define BAT_HEATER_OFF 6
#define HONEYWELL_PRESSURE A16
#define SPS_SERIAL Serial5
#define XBEE_SERIAL Serial3
#define UBLOX_SERIAL Serial2
#define PMSserial Serial1

//////////////////////////////////////////////
/////////////////Constants////////////////////
//////////////////////////////////////////////
#define MAIN_LOOP_TIME 1000          // Main loop runs at 1 Hz
#define CONTROL_LOOP_TIME 1000       // Control loop runs at 0.5 Hz
#define SPS_30_TIME 1500
#define TIMER_RATE (1000) 
#define C2K 273.15 
#define PMS_TIME 1 //PMS Timer
#define SPECIFIC_GAS_CONSTANT  287   //(J/kg*K)
#define SEA_LEVEL_PRESSURE  101325   //(Pa)
#define GRAVITY_ACCEL  9.81
#define METERS_TO_FEET  3.28084
#define PSI_TO_ATM  0.068046                         


/////////////Geographic Boundaries/////////////
#define EASTERN_BOUNDARY 0
#define WESTERN_BOUNDARY 0
#define NORTHERN_BOUNDARY 0
#define SOUTHERN_BOUNDARY 0
#define MAX_ALTITUDE 0


/////////////////Release Timers/////////////////
#define RELEASER_TIMER 15000
#define MASTER_TIMER 20000


//////////////On Baord SD Chipselect/////////////
const int chipSelect = BUILTIN_SDCARD; //On board SD card for teensy

///////////////////////////////////////////////
////////////////Power Relays///////////////////
//////////////////////////////////////////////
//LatchRelay opcRelay(OPC_ON, OPC_OFF);
LatchRelay opcHeatRelay(OPC_HEATER_ON,OPC_HEATER_OFF);
LatchRelay batHeatRelay(BAT_HEATER_ON,BAT_HEATER_OFF);
//LatchRelay sirenRelay(SIREN_ON, SIREN_OFF);
//boolean  opcON = false;
//String opcRelay_Status = "";
String opcHeat_Status = "";
String batHeat_Status = "";

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


////////////////////GPS////////////////////
UbloxGPS GPS(&UBLOX_SERIAL);
float alt_GPS = 0;               // altitude calculated by the GPS in feet
float prev_alt_feet = 0;         // previous calculated altitude


/////////Honeywell Pressure Sensor/////////
int pressureSensor;              // Analog number given by sensor
float pressureSensorVoltage;     // Voltage calculated from analog number
float PressurePSI;               // PSI calculated from voltage
float PressureATM;               // ATM calculated from PSI


///////////////////////////////////////////
/////////////////Control///////////////////
///////////////////////////////////////////
// SMART
String SmartData; //Just holds temporary copy of Smart data
static String SmartLog; //Log everytime, is just data from smart
static bool Cut=false; //Set to true to cut A SMART
static bool ChangeData=true; //Just set true after every data log
SmartController SOCO = SmartController(2,XBEE_SERIAL,200.0); //Smart controller
String smartOneString = "Primed";


float ascent_rate = 0;     // ascent rate of payload in feet per minute
float Control_Altitude = 0;                 // final altitude used between alt_GPS, alt_pressure_library, and time predicted altitude depending on if we have a GPS lock
static float prev_time = 0;                 // prev time for S_Control
static float prev_Control_Altitude = 0;     // records the most recent altitude given by GPS when it had lock
static bool GPSfix = false;                 // determines if the GPS has a fix or not

//Timers
unsigned long releaseTimer = RELEASER_TIMER * 1000;
unsigned long masterTimer = MASTER_TIMER * 1000;
boolean recovery = false;
unsigned long smartTimer = 0;

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
static String data;
String Fname = "";
String FnamePMS = "";
boolean SDcard = true;

//Plantower Definitions

int nhits=1;            //used to count successful data transmissions
int ntot=1;             //used to count total attempted transmitions
int badLog =1;
boolean goodLog = false;
static String dataPMS="";                              
struct PMS5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
struct PMS5003data PMSdata;

//SPS30 Definitions

byte buffers[40] = {0}, systemInfo [5] = {0}, MassC[16] = {0};                  //Byte variables for collection and organization of data from the SPS30.
byte NumC[20] = {0}, AvgS[4] = {0}, dataEmpty = 0, checksum = 0, SPSChecksum = 0;
bool goodLogS = false;
int badLogS = 0;

union mass                                                                      //Defines the union for mass concentration
{
  byte MCA[16];
  float MCF[4];
}m;

union num                                                                       //Defines the union for number concentration
{
  byte NCA[20];
  float NCF[5];
}n;

union avg                                                                       //Defines the union for average sizes
{
  byte ASA[4];
  float ASF;
}a;

//////////////////////////////////////////////
/////////Initialize Flight Computer///////////
//////////////////////////////////////////////
void setup() {

  //Initialize LEDs
  initLEDs();

  //Initialize SD
  initSD();

  //Initialize Serial
  initSerial();
  
  //Initialize Radio
  initRadio();

  //Initialize GPS
  initGPS();
  
  //Initialize Temp Sensors
  initTempSensors();

  //Initialize Relays
  initRelays();

  //Initialize SPS
  SPS_init(&SPS_SERIAL);                                                        //The SPS30 Initialization command will boot and clean the sensor.
  
  Serial.println("Setup Complete");

}


void loop(){
  static unsigned long controlCounter = 0;
  static unsigned long mainCounter = 0;

  UpdateGPS();                                       // Update the GPS with new values
  readPMSdata(&PMSserial);
  
  if (millis()-mainCounter>=MAIN_LOOP_TIME){
    mainCounter = millis();
    actionBlink();
    fixBlink();
    updateSensors();   //Updates and logs all sensor data
    actHeat();  
  }
  
  
  // Control Thread

  SMARTControl();     //Control function for SMART unit
    
  if (millis()-controlCounter>=CONTROL_LOOP_TIME){
    controlCounter = millis();
    SOCO.Cut(1,Cut);
    stateMachine();
  } 

//RETURNS DATA STRING, RUNS AT 1500 MS
  SPS_Update(&SPS_SERIAL);
  
}
