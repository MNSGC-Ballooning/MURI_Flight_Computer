#include <i2c_t3.h>
#include <Arduino.h>
#include "Salus_Baro.h"

///////////Defines///////////
#define TIMER_RATE      (1000)                  // Check the timer every 1 millisecond
#define BARO_RATE       (TIMER_RATE / 200)      // Process MS5607 data at 100Hz
#define C2K             273.15

Salus_Baro myBaro;
float pressure = 0;
float altitude = 0;
float temperature = 0;
unsigned long prevTime = 0;
float startAlt = 0;


void setup() {
  Serial.begin(9600);
  delay(500);
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
  // Initialize the Barometer
  Serial.println(F("Initializing Barometer..."));
  myBaro.begin();
  startAlt = myBaro.getAltitude();
  Serial.print(F("Start Alt: "));
  Serial.println(startAlt);
  Serial.println(F("Barometer Initialized.\n"));
  Wire.setRate(I2C_RATE_400);

}

void loop() {
 if(millis()-prevTime>=1000){ 
   prevTime = millis();
   myBaro.baroTask();
   Serial.println(myBaro.getReferencePressure());
   pressure = myBaro.getPressure()/10;
   altitude = myBaro.getAltitude(); //- startAlt;
   temperature = myBaro.getTemperature()+C2K;
   Serial.println("Pressure: " + String(pressure) + " kpa");
   Serial.println("Altitude: " + String(altitude) + " m");
   Serial.println("Temperature: " + String(temperature) + " K");
   
 }

}
