// Control Systems For New State Machine
#define SPECIFIC_GAS_CONSTANT      287 //(J/kg*K)
#define SEA_LEVEL_PRESSURE         101325 //(Pa)
#define GRAVITY_ACCEL              9.81
#define METERS_TO_FEET             3.28084
#define Fix        0x00 //0000 0000
#define NoFix      0x01  //0000 0001

//Variables
uint8_t FixStatus= NoFix;
long Pressure_Alt; // altitude in feet based on pressure and temp

///// Function Declarations /////


// check if gps fix is good
void MeasurementCheck() {
  if (GPS.getFixAge() < 4000 && (GPS.getAlt_feet() > ((myBaro.getAltitude()*METERS_TO_FEET)-3000)) && (GPS.getAlt_feet() < ((myBaro.getAltitude()*METERS_TO_FEET)+3000)) && ((myBaro.getAltitude()*METERS_TO_FEET)>0)) 
  {
    FixStatus = Fix;
  }
  else if(GPS.getFixAge() < 4000 && (myBaro.getAltitude()*METERS_TO_FEET)<=0)
  {
    FixStatus = Fix;
  }
  else
  {
    FixStatus = NoFix;
  }
}
/*
bool CheckEstimate(){

  Pressure_Alt = Pressure_Alt_Calc(pressure*1000,t2); // not sure where these come from but pressure should be in 'Pa' and temp should be in 'K'

  if(GPS.getAlt_feet() > (Pressure_Alt - 3000) && GPS.getAlt_feet() < (Pressure_Alt + 3000))
  {
    return true; 
  }
  else
  {
    return false;
  }
}
*/
 //determine the altitude from Pressure and Temperature using Hypsometric formula
float Pressure_Alt_Calc(float Pressure, float Temperature){
  float Pressure_Alt_SI = (SPECIFIC_GAS_CONSTANT*Temperature/GRAVITY_ACCEL)*log(SEA_LEVEL_PRESSURE/Pressure); // returns altitude based on P and T in meters.
  float Pressure_Alt = Pressure_Alt_SI*METERS_TO_FEET; // converts m to ft 

  return Pressure_Alt;
}
