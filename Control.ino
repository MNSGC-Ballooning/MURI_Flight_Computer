// Control Systems For New State Machine

///// Variables /////
long Pressure_Alt; // altitude in feet based on pressure and temp

///// Function Declarations /////

// check if gps fix is good
bool CheckEstimate()
{
  Pressure_Alt = Pressure_Alt_Calc(******pres,******temp); // not sure where these come from but pressure should be in 'Pa' and temp should be in 'K'
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
  float R = 287; // gas constant (J/kg*K) * Should be a pre compiler directive
  float P0 = 101325; // atmospheric pressure at sea level (Pa) *These should be defined as pre compiler directives since they are constants

  float Pressure_Alt_SI = (R*Temperature/9.81)*log(P0/pressure); // returns altitude based on P and T in meters. * No random numbers. Gravitational constant should be a pre compiler directive
  float Pressure_Alt = Pressure_Alt_SI*3.28084; // converts m to ft * No random numbers, use precompiler directives. The name of the macro should reflect the constant and be in all caps

  return Pressure_Alt;
}



///// ASCENT_RATE class /////
ASCENT_RATE::ASCENT_RATE()
{
  rate=0;
  prevh=0;
  prevt=0;
}

void ASCENT_RATE::updateRate()
{
 rate=((alt_feet-prevh)/(getGPStime()-prevt))*60; //h_dot in feet per minute
 prevh=alt_feet;
 prevt=getGPStime();
 Serial.println("Rate: " + String(rate));
 Serial.println("Alt: " + String(prevh));
 Serial.println("Time: " + String(prevt));
}

float ASCENT_RATE::getRate(){
  updateRate();
  return rate;
}
float ASCENT_RATE::getPrevh(){
  return prevh;
}
float ASCENT_RATE::getPrevt(){
  return prevt;
}
