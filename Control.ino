// Control Systems For New State Machine

///// Variables /////
long Pressure_Alt; // altitude in feet based on pressure and temp
int Lock; // counter for good GPS fixes
int NoLock; // counter for bad GPS fixes


///// Function Declarations /////

// check if gps fix is good
bool CheckEstimate()
{
  Pressure_Alt = Pressure_Alt_Calc(pres,temp); // not sure where these come from but pressure should be in 'Pa' and temp should be in 'K'
  if(GPS.altitude.feet() > (Pressure_Alt - 3000) && GPS.altitude.feet() < (Pressure_Alt + 3000))
  {
    Lock++; // increment counter for Lock
    if( Lock >= 5)
    {
      NoLock = 0; // if we get 5 locks in a row, reset NoLock 
    }

    return true;
    
  }
  else
  {
    NoLock++;
    Lock = 0; // if we get a bad lock, reset Lock

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

// get the current value of Lock
int GetLock()
{
  return Lock;
}

// get the curret value of NoLock
int GetNoLock()
{
  return NoLock
}
