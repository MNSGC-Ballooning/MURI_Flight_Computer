//Libraries
//this requires a special modded version of the TinyGPS library because for whatever
//reason the TinyGPS library does not include a "Fix" variable. the library can be found here:
//https://github.com/simonpeterson/TinyGPS/tree/master 
#include <SPI.h>
#include <SD.h>
#include <LatchRelay.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Smart.h>
#include <UbloxGPS.h>              //Needs TinyGPS++ in order to function
#include <i2c_t3.h>                //Required for usage of MS5607 
#include <Arduino.h>               //"Microcontroller stuff" - Garret Ailts 
#include "Salus_Baro.h"            //Library for MS5607
#include <SmartController.h>       //Library for smart units using xbees to send commands

//==============================================================
//               MURI Flight Computer
//               Written by Garrett Ailts - ailts008 Summer 2018 
//               Edited by Asif Ally  - allyx004 Summer 2019                  
//==============================================================

//Version Description: MURI Flight Computer for double balloon configuration. Controls balloon flight using a finite state machine and logs payload/atmospheric data.
//Switches states based on ascent rate

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

//=============================================================================================================================================
//=============================================================================================================================================

boolean opcActive = false;

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

////////////////////////////////////////////////////////
/////////////////Class Definitions//////////////////////
////////////////////////////////////////////////////////
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

/////////////////////////////////////////////
///////////////Pin Definitions///////////////
/////////////////////////////////////////////
#define ledPin 21           //Pin which controls the DATA LED, which blinks differently depending on what payload is doing
#define ledSD 23            //Pin which controls the SD LED
#define fix_led 22          //led  which blinks for fix
#define ONE_WIRE_BUS 29     //Internal Temp
#define TWO_WIRE_BUS 30     //External Temp
#define THREE_WIRE_BUS 31   //Battery Temp
#define FOUR_WIRE_BUS 32    //OPC Temp
#define OPC_ON 5            //Relay switches
#define OPC_OFF 6
#define OPC_HEATER_ON 24
#define OPC_HEATER_OFF 25
#define BAT_HEATER_ON 7
#define BAT_HEATER_OFF 8
#define XBEE_SERIAL Serial5
#define UBLOX_SERIAL Serial2
#define PMS5003_SERIAL Serial
//#define SIREN_ON 32
//#define SIREN_OFF 33

//////////////////////////////////////////////
/////////////////Constants////////////////////
//////////////////////////////////////////////
#define MAIN_LOOP_TIME 1000          // Main loop runs at 1 Hz
#define CONTROL_LOOP_TIME 100        // Control loop runs at 10 Hz
#define TIMER_RATE (1000) 
#define Baro_Rate (TIMER_RATE / 200)  // Process MS5607 data at 100Hz
#define C2K 273.15 

//////////////On Baord SD Chipselect/////////////
const int chipSelect = BUILTIN_SDCARD; //On board SD card for teensy

///////////////////////////////////////////////
////////////////Power Relays///////////////////
///////////////////////////////////////////////
LatchRelay opcRelay(OPC_ON, OPC_OFF);
LatchRelay opcHeatRelay(OPC_HEATER_ON,OPC_HEATER_OFF);
LatchRelay batHeatRelay(BAT_HEATER_ON,BAT_HEATER_OFF);
//LatchRelay sirenRelay(SIREN_ON, SIREN_OFF);
boolean  opcON = false;
String opcRelay_Status = "";
String opcHeat_Status = "";
String batHeat_Status = "";

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
UbloxGPS GPS(&UBLOX_SERIAL);
boolean fixU = false;

//MS5607 pressure and temperature sensor
Salus_Baro myBaro;
float pressure = 0;
float altitude = 0;
float temperature = 0;
unsigned long prevTime = 0;
float startAlt = 0;

///////////////////////////////////////////
/////////////////Control///////////////////
///////////////////////////////////////////
// SMART
String SmartData; //Just holds temporary copy of Smart data
static String SmartLog; //Log everytime, is just data from smart
static bool CutA=false; //Set to true to cut A SMART
static bool CutB=false; //Set to true to cut B SMART
static bool ChangeData=true; //Just set true after every data log
static bool tempA=false; //Just flip flops temp requests from A and B (Stupid make better later)
SmartController SOCO = SmartController(2,XBEE_SERIAL,200.0); //Smart controller

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
boolean SDcard = true;

void setup() {

  // initialize LEDs
  pinMode(ledPin, OUTPUT);
  pinMode(ledSD, OUTPUT);
  pinMode(fix_led, OUTPUT);
  
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
  sensor4.begin();

  //Initialize Pressure Altimeter
  initMS5607();

  //Initialize Relays
  initRelays();
  
  Serial.println("Setup Complete");
}
void loop(){
  static unsigned long controlCounter = 0;
  static unsigned long mainCounter = 0;

  // Main Thread
  if (millis()-mainCounter>=MAIN_LOOP_TIME){
    mainCounter = millis();
    actionBlink();
    fixBlink();
    GPS.update();
    updateSensors();   //Updates and logs all sensor data
    actHeat();
  }
  
  // Control Thread
  if (millis()-controlCounter>=CONTROL_LOOP_TIME){
    SmartData=SOCO.Response();
    if (ChangeData){
      SmartLog=SmartData;
      ChangeData=false;
    }
    SOCO.Cut(1,CutA);
    SOCO.Cut(2,CutB);
    //stateMachine();
  } 
}
