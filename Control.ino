// Control Systems For New State Machine

///// Variables /////
long Pressure_Alt; // altitude in feet based on pressure and temp

///// Function Declarations /////

// check if gps fix is good
bool CheckEstimate()
{
  Pressure_Alt = Pressure_Alt_Calc(pres,temp); // not sure where these come from but pressure should be in 'Pa' and temp should be in 'K'
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
long Pressure_Alt_Calc(long Pressure, long Temperature)
{
  long R = 287; // gas constant (J/kg*K)
  long P0 = 101325; // atmospheric pressure at sea level (Pa)

  long Pressure_Alt_SI = (R*Temperature/9.81)*log(P0/pressure); // returns altitude based on P and T in meters.
  long Pressure_Alt = Pressure_Alt_SI*3.28084; // converts m to ft

  return Pressure_Alt;
}
