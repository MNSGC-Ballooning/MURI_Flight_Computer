// Control Systems For New State Machine

#define SPECIFIC_GAS_CONSTANT = 287; //(J/kg*K)
#define SEA_LEVEL_PRESSURE 101325; //(Pa)
#define GRAVITY_ACCEL 9.81
#define METERS_TO_FEET 3.28084


///// Variables /////
long Pressure_Alt; // altitude in feet based on pressure and temp

///// Function Declarations /////

// check if gps fix is good
bool CheckEstimate()
{
  Pressure_Alt = Pressure_Alt_Calc(kpa*1000,t2); // not sure where these come from but pressure should be in 'Pa' and temp should be in 'K'
  if(Ublox.getAlt_feet() > (Pressure_Alt - 3000) && Ublox.getAlt_feet() < (Pressure_Alt + 3000))
  {
    return true; 
  }
  else
  {
    return false;
  }
}

// determine the altitude from Pressure and Temperature using Hypsometric formula
float Pressure_Alt_Calc(float Pressure, float Temperature)
{
  float Pressure_Alt_SI = (SPECIFIC_GAS_CONSTANT*Temperature/GRAVITY_ACCEL)*log(SEA_LEVEL_PRESSURE/pressure); // returns altitude based on P and T in meters.
  float Pressure_Alt = Pressure_Alt_SI*METERS_TO_FEET; // converts m to ft 

  return Pressure_Alt;
}
