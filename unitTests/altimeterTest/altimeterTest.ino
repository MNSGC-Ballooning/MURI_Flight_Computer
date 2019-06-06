
// Author: Asif ALly
#include <Melon_MS5607.h>

Melon_MS5607 MS5=Melon_MS5607();
int32_t tempByte;
int32_t presByte;
unsigned long prevtime = 0;
float temp;
float pres;

#define MS_I2C_ADDR 0xEE // 11101110

void setup() {
  Serial.begin(9600);
  MS5.begin(MS_I2C_ADDR);
  delay(500);

}

void ms5Update() {
  
  if(MS5.readTemperature()){
    
    tempByte=MS5.getTemperature(); 

    }
  if(MS5.readPressure()){
    presByte= MS5.getPressure();
  }

  Serial.println("pressure: " +  String(presByte));
  Serial.println("temp: " + String(tempByte));

  
//  String data = "";
//  data += ("Temp:" + String(ms_temp)+ "," + " Pressure:" + String(ms_pressure) + ",");
//  Serial.println(data);
}

void loop() {
  if (millis()-prevtime>=1){
    prevtime=millis();
  ms5Update();
  }
}
