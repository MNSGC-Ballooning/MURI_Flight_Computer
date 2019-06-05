// Author: Asif ALly
#include <MS5xxx.h>

MS5xxx MS5(&Wire);
int ms_temp=0;
int ms_pressure = 0;


void setup() {
  Serial.begin(9600);
  MS5.connect();
  delay(500);

}

//String create8bit(String binval){
//  int len = binval.length();
//  int idx=0;int idx2=0;
//  String largebinval = "0000000"+binval;
//  int fullLen = largebinval.length();
//  String bin8bitval = "00000000";
//  for(idx=fullLen-7;idx<fullLen;idx++){
//    bin8bitval[idx2] = largebinval[idx];idx2++;
//  }
//  return bin8bitval;
//}

void ms5Update() {
  MS5.ReadProm();
  MS5.Readout();
  ms_temp = (MS5.GetTemp()/100); //because temp is given in .01 C
  ms_pressure =(MS5.GetPres());
  String data = "";
  data += (String(ms_temp)+ "," + String(ms_pressure) + ",");
  Serial.println(data);
}

void loop() {
  ms5Update();

}

//float MS5607::getPressure() {
//  
//    if(rawTemperature && rawPressure != 0) {
//        int64_t dT = rawTemperature - (calData[4] << 8);
//        int64_t OFF  = (calData[1] << 17) + ((dT * calData[3]) >> 6);
//        int64_t SENS = (calData[0] << 16) + ((dT * calData[2]) >> 7);
//        return ((((((rawPressure * SENS) >> 21) - OFF) >> 15) / 100.0) + 20) * 0.02953;
//    }
//    else {
//        return -1;
//    }
//}
//
//float MS5607::getTemperature() {
//
//    if(rawTemperature && rawPressure != 0) {
//        int64_t dT = rawTemperature - (calData[4] << 8);
//        return (((2000 + ((dT * calData[5]) >> 23)) / 100.0) * 1.8) + 32;
//    }
//    else {
//        return NULL;
//    }
//}
