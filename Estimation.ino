// Control Systems For New State Machine
#define SPECIFIC_GAS_CONSTANT      287 //(J/kg*K)
#define SEA_LEVEL_PRESSURE         101325 //(Pa)
#define GRAVITY_ACCEL              9.81
#define METERS_TO_FEET             3.28084
#define Fix        0x00 //0000 0000
#define NoFix      0x01  //0000 0001

//Variables
uint8_t FixStatus= NoFix;
//long Pressure_Alt; // altitude in feet based on pressure and temp

///// Function Declarations /////


// check if gps fix is good
void MeasurementCheck() {
  if (GPS.getFixAge() < 4000)// && (GPS.getAlt_feet() > ((myBaro.getAltitude()*METERS_TO_FEET)-3000)) && (GPS.getAlt_feet() < ((myBaro.getAltitude()*METERS_TO_FEET)+3000)) && ((myBaro.getAltitude()*METERS_TO_FEET)>0)) 
  {
    FixStatus = Fix;
  }
  else if(GPS.getFixAge() > 4000)
  {
    FixStatus = NoFix;
  }
  else
  {
    FixStatus = NoFix;
  }
}
