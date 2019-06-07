#include "SmartController.h"
//#include <SD.h>
//#define chipSelect_pin 8




//USEFUL VARIABLES FOR SMART IMPLEMENTATION (maybe dont need)
boolean TempA=false; //Just flips requests for A and B temps
unsigned long TempTimer=0;
String SmartData;

int SmartNumber=2;
SmartController SOCO= SmartController(SmartNumber,Serial3,200.0);


void setup() { 
 // SDsetup();
 // logData("SMART LOG START");
  Serial.begin(9600);
  Serial3.begin(9600);





}

void loop() {

if(millis()-TempTimer>1000){
  if(!TempA){
  SOCO.RequestTemp(1);  //Sends request to SMART 1 for temperature data
  TempTimer=millis();
  TempA=true;
}
else
{
  SOCO.RequestTemp(2);  //Sends request to SMART 2 for temperature data
  TempTimer=millis();
  TempA=false;
}

SmartData=SOCO.Response();  //MUST CALL EVERY LOOP
if (SmartData!=""){        //Should log unless "" is returned (no useful data)
Serial.println(SmartData);
}


SOCO.Cut(1,millis()>300000); //Cuts after 5 minutes
SOCO.Cut(2,millis()>600000); //Cuts after 10 minutes
}

}
