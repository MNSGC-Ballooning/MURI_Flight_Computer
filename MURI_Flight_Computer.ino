//============================================================================================================================================
//               MURI Flight Computer
//               Written by Patrick James Collins (PJ) - coll0792 Fall 2019
//               Edited by Asif Ally (AA)  - allyx004 Fall 2019
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
#include <OneWire.h>                                                   //Dallas Data Bus Library
#include <DallasTemperature.h>                                         //Dallas Sensor Library
#include <Arduino.h>                                                   //"Microcontroller stuff" - Garrett Ailts 
#include <OPCSensor.h>                                                 //Library for OPCs
#include <Wire.h>                                                      //Library for I2C

////////////////////////////////////
//////////Pin Definitions///////////
////////////////////////////////////
#define ONE_WIRE_BUS 29                                                //Battery Temp
#define HONEYWELL_PRESSURE A16                                          //Analog Honeywell Pressure Sensor
#//define PMSA_SERIAL Serial3                                            //Serial Pins
#define SPSA_SERIAL Serial1                                                                                
#//define PMSB_SERIAL Serial5
#define SPSB_SERIAL Serial2

#define LED_SD 24
/////////////////////////////
//////////Constants//////////
/////////////////////////////
//TIMERS
#define CONTROL_LOOP_TIME 1000                                         //Control loop runs at 1.0 Hz
#define STATE_LOG_TIMER 4000                                           //Log timer runs at 0.25 Hz

//Constants
#define DC_JUMPER 1                                                    //The DC_JUMPER is the I2C Address Select jumper. Set to 1 if the jumper is open (Default), or set to 0 if it's closed.
#define MINUTES_TO_MILLIS 60000                                        //MATH TIME
#define PSI_TO_ATM  0.068046                                           //Live love conversions   
#define C2K 273.15                                                     //Celsius to Kelvin. What else is there to say?    
#define PMS_TIME 1                                                     //PMS Timer

//On Board SD Chipselect
const int chipSelect = BUILTIN_SDCARD;                                 //On board SD card for teensy

////////////////////////////////
//////////Power Relays//////////
////////////////////////////////

////////////////////////////////////
//////////Sensor Variables//////////
////////////////////////////////////
//Dallas Digital Temp Sensors
OneWire oneWire1(ONE_WIRE_BUS);                                        //Temperature sensor wire busses
DallasTemperature sensor1(&oneWire1);                                  //Temperature sensors
float t1;                                                  //Temperature values

//Honeywell Pressure Sensor
float pressureSensor;                                                  //Analog number given by sensor
float pressureSensorVoltage;                                           //Voltage calculated from analog number
float PressurePSI;                                                     //PSI calculated from voltage
float PressureATM;                                                     //ATM calculated from PSI

//Timers and Counters
unsigned long ControlCounter = 0;
unsigned long StateLogCounter = 0;
static float masterClock = 0;                                          //Needs to be a float as this value is logged

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
//Plantower PlanA(&PMSA_SERIAL, STATE_LOG_TIMER);                        //Establish objects and logging string for the OPCs
//Plantower PlanB(&PMSB_SERIAL, STATE_LOG_TIMER);                        //oops someone didn't use protection      
SPS SPSA(&SPSA_SERIAL);  
SPS SPSB(&SPSB_SERIAL);

String OPCdata = "";



//////////////////////////////////////////////
//////////Initialize Flight Computer//////////
//////////////////////////////////////////////
void setup() {
  analogReadResolution(13);

  
  initSD();                                                            //Initialize SD

  initOPCs();                                                          //Initialize OPCs
  delay(1000);

  initTemp();                                                          //Initialize Temp Sensors
}

void loop(){
//  PlanA.readData();
//  PlanB.readData();


    
  if (millis() - StateLogCounter >= STATE_LOG_TIMER) {
      StateLogCounter = millis();
                                                                        //Update state machine
      updateSensors();                                                   //Updates and logs all sensor data
  }  
}
