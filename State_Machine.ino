//PID controller that looks at the derivative of altitude and the current altitude state
#define STATE_MURI_INIT 0x00            //0000 0000
#define STATE_MURI_ASCENT 0x01          //0000 0001
#define STATE_MURI_FAST_DESCENT 0x02    //0000 0010
#define STATE_MURI_SLOW_DESCENT 0x04    //0000 0100
#define STATE_MURI_SLOW_ASCENT 0x08     //0000 1000
#define STATE_MURI_CAST_AWAY 0x10       //0001 0000
#define STATE_MURI_RECOVERY 0x20        //0010 0000

#define Lock 0xAA;
#define NoLock 0xBB;

uint8_t muriState;
uint8_t GPSstatus = NoLock;

void stateMachine()
{
  unsigned long castAway = 0;
  static byte initCounter = 0;
  static byte skyCheck = 0;
  static byte float_longitude_check = 0;   //how many times we have exceed the given longitude
  static byte termination_longitude_check = 0;
  static byte floorCheck = 0;
  static byte snail = 0;
  static bool init = false;
  static bool fast = false;
  static bool cast =false;
  static unsigned long prevTimes = 0;

  float alt_feet; // altitude in feet
  int i; // counter for getting GPS Lock
  
  if(!init)
  {
    muriState = STATE_MURI_INIT;
    stateString = "INITIALIZATION";
    init=true; // initalize state machine
    Serial.println("Initializing...");
  }
  if(millis() >= masterTimer) // if mission time is exceeded without recovery, it cuts the balloons and just enters the recovery state
  {
    smartOne.release();
    smartTwo.release();
    muriState = STATE_MURI_RECOVERY;
    stateString = "RECOVERY";
  }
  blinkMode();                          //Controls Data LED that shows payload state
  Fixblink();                           //Controls LED that gives GPS fix information
  //opcControl();                       //Turns on OPC at the start of the desired altitude range
  if(muriState!=STATE_MURI_FAST_DESCENT || !recovery)
  {
   actHeat(); 
  }


  // determine GPSstatus (lock or no lock)
  if(FixStatus == Fix)
  {
    i++;
    if(i>=5)
    {
      GPSstatus = Lock;
    }  
  }
  if(FixStatus == NoFix)
  {
    GPSstatus = Nolock;
    i = 0;
  }

  // determine the best altitude to use based on lock or no lock)
  if(GPSstatus == Lock)
  {
    alt_feet = GPS.getAlt_feet();
  }
  else if(GPSstatus == NoLock)
  {
    alt_feet = Pressure_Alt_Calc(pressure,temperature); // not sure what the pressure and temp variables are called
  }
  
  PID();                                //Controller that changes State based on derivative of altitude
  
///////////Finite State Machine///////////////
//Serial.println("GLGPS: " + String(getLastGPS()));
//Serial.println("Prev time: " + String(prevTimes));
if(alt_feet!=0 && millis()-prevTimes>1000 && alt_feet!=hDOT.getPrevh()) 
{
  // enter when alt >0, it has been more than 1 sec since last update, and the altitudde is changing 

///////////////////////////////////////////////////////////////////////////////////
    // can this only be done if there is a fix?  
    if(FixStatus == Fix && float(GPS.location.lng()) > termination_longitude && GPS.location.lng() != 0) // if paload drifts outide of longitude bounds and longitude is not 0 (gps has fix)
    {
      termination_longitude_check++; // and 1 to # of times outside mission area
      Serial.println("Termination Longitude check: " + String(termination_longitude_check));
      if (termination_longitude_check>5)
      {
        // if it is outside mission area for 5 consecutive checks, then balloons are released. Probably should enter recovery state here
        smartOne.release();
        smartTwo.release();
        smartOneString = "RELEASED";
        smartTwoString = "RELEASED";
        termination_longitude_check = 0;
      }
    }
    else
    {
      // if longitude is still in acceptable range, then reset check
      termination_longitude_check = 0;
    }
    
    prevTimes=millis(); // set time
    hDOT.updateRate(); // get ascent rate
    Serial.println("h dot: " + String(hDOT.geth_dot()));


///////////////////////////////////////////////////////////////////////////////////////////////////////    
    if(muriState == STATE_MURI_INIT && !hdotInit) // checks to see if in initial state. not entirely sure what hdotInit is tho...
    {
      Serial.println("STATE_MURI_INIT"); // records current state
      if(hDOT.getRate()>250 && hDOT.getRate()<1500)
      {
        hDOT.addHit(); // not sure what a hit is... pretty sure it has something to do with finding average ascent rate maybe?
      }
      else
      {
        hDOT.checkHit(); // not sure what this is either
      }
      if(alt_feet>5000)
      {
        // this looks like it keeps counting while above 5000 ft to initialize hdot, whatever that means
        initCounter++;
        if(initCounter>5)
        {
          hdotInit=true;
          Serial.println("h_dot initialized!");
        }
      } 
    }
    // overall not sure what the point of this state is. maybe the hits are important but i dont know. it just seems like it isnt doing anything of value

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    if(muriState == STATE_MURI_ASCENT) // checks to see if the state is ascent
    {
      Serial.println("STATE_MURI_ASCENT");
      // again not sure what hits are
      if(hDOT.getRate()>250 && hDOT.getRate()<1500)
      {
        hDOT.addHit();
      }
      else
      {
        hDOT.checkHit();
      }
      if(alt_feet>maxAlt && alt_feet!=0)// checks to see if payload has hit the max mission alt
      {
        skyCheck++;
        Serial.println("Max alt hits: " + String(skyCheck));
        if(skyCheck>5)
        {
          // if the payload is consistenly above max mission alt. , the first balloon is released
          smartOne.release();
          smartOneString = "RELEASED";
          skyCheck = 0;
        }
      }
      else
      {
        skyCheck = 0; // if not above max alt consistently, set counter back to 0
      }
      // can this only run with a fix?
      if(FixStatus == Fix && float(GPS.location.lng()) > float_longitude && GPS.location.lng() != 0) // not entirely sure what floating is.... but it seams like a cut off to start descent 
      {
        float_longitude_check++;
        Serial.println("float Longitude check: " + String(float_longitude_check));
        if (float_longitude_check>5)
        {
          smartOne.release();
          smartOneString = "RELEASED";
          float_longitude_check = 0;
        }
      }
    }
    
////////////////////////////////////////////////////////////////////////////////////////////////////////    
    else if(muriState == STATE_MURI_SLOW_DESCENT) // checks if in slow descent (1 balloon)
    {
      Serial.println("STATE_MURI_SLOW_DESCENT");
      if(hDOT.getRate()>-2000 && hDOT.getRate()<-50)
      {
        hDOT.addHit();
      }
      else
      {
        hDOT.checkHit();
      }
      if(alt_feet<minAlt && alt_feet!=0) // checks to see if it is below data collection range...
      {
        floorCheck++;
        Serial.println("Min alt hits: " + String(floorCheck));
        if(floorCheck>5)
        {
          // if consistently below data collection range then release all balloons again just in case
          smartTwo.release();
          smartTwoString = "RELEASED";
          smartOne.release();
          smartOneString = "RELEASED";
          floorCheck = 0;
        }
      }
      else
      {
        floorCheck = 0;
      }
    }

//////////////////////////////////////////////////////////////////////////////////////////////////    
    else if(muriState == STATE_MURI_FAST_DESCENT)
    {
      Serial.println("STATE_MURI_FAST_DESCENT");
      if(hDOT.getRate()<-2000)
      {
        hDOT.addHit();
      }
      else
      {
        hDOT.checkHit();
      }
      if(!fast)
      {
        smartTwo.release();
        smartTwoString = "RELEASED";
        smartOne.release();
        smartOneString = "RELEASED";
        opcHeatRelay.closeRelay();
        batHeatRelay.closeRelay();
        fast=true;
      }
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////    
    else if(muriState == STATE_MURI_SLOW_ASCENT)
    {
      Serial.println("STATE_MURI_SLOW_ASCENT");
      if(50>hDOT.getRate() && hDOT.getRate()<=250)
      {
        hDOT.addHit();
      }
      else
      {
        hDOT.checkHit();
      }
      snail++;
      if(snail>10)
      {
        if(alt_feet<30000)
        {
          smartTwo.release();
        }
        else if(alt_feet>=30000)
        {
          smartOne.release();
          smartOneString = "RELEASED";
          smartTwo.release();
          smartTwoString = "RELEASED";
        }
        snail = 0;
      }
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////    
    else if(muriState == STATE_MURI_CAST_AWAY)
    {
      if(hDOT.getRate()>=-50 && hDOT.getRate()<=50)
      {
        hDOT.addHit();
      }
      else
      {
        hDOT.checkHit();
      }
      Serial.println("STATE_MURI_CAST_AWAY");
      if(!cast)
      {
        castAway = millis();
        cast = true;
      }
      if(millis()-castAway >= 600000)
      {
        smartOne.release();
        smartTwo.release();
      }
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////    
    else if(muriState == STATE_MURI_RECOVERY)
    {
      Serial.println("STATE_MURI_RECOVERY");
      if(!recovery)
      {
        sirenRelay.openRelay();
        opcRelay.closeRelay();
        recovery = true;
      }
    }
    
  }
  
}


/////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////////////////////

void PID()
{
  static byte wilson = 0;
  if(hdotInit && alt_feet!=0 && !recovery)
  {
    if(hDOT.geth_dot()!=0)
    {
      tickTock.updateTimer(hDOT.geth_dot());
    }
    tickTock.hammerTime();
    if(hDOT.geth_dot()>=5000 || hDOT.geth_dot()<=-5000)
    {
      Serial.println("GPS Jump Detected");
    }
    else if(hDOT.geth_dot() > 250)
    {
      muriState = STATE_MURI_ASCENT;
      stateString = "ASCENT";
    }
    else if(hDOT.geth_dot()>50 && hDOT.geth_dot()<=250)
    {
      muriState = STATE_MURI_SLOW_ASCENT;
      stateString = "SLOW ASCENT";
    }
    else if(hDOT.geth_dot() >= -1500 && hDOT.geth_dot() < -50)
    {
      static bool first = false;
      muriState = STATE_MURI_SLOW_DESCENT;
      stateString = "SLOW DESCENT";
      if(!first)
      {
        smarty = &smartTwo;
        if(alt_feet<minAlt)
        {
          minAlt=alt_feet-10000;
        }
        first = true;
      }
    }
    else if(hDOT.geth_dot()<=-2000 && alt_feet>7000)
    {
      muriState = STATE_MURI_FAST_DESCENT;
      stateString = "FAST DESCENT";
    }
    else if(hDOT.geth_dot()>=-50 && hDOT.geth_dot()<=50)
    {
      wilson++;
      if(wilson>100)
      {
        muriState = STATE_MURI_CAST_AWAY;
        stateString = "CAST AWAY";
        wilson=0;
      }
    }
    else if(muriState == STATE_MURI_FAST_DESCENT && alt_feet<7000)
    {
      muriState = STATE_MURI_RECOVERY;
      stateString = "RECOVERY";
    } 
  } 
}


void opcControl(){
  static byte checktimes;
  if(!opcON && alt_feet>=75,000){
    checktimes++;
    if(checktimes>=15){
      opcRelay.openRelay();
      opcON=true;
    }
  }
}


/////////Control Classes Definitions////////////
//Relay class functions
Relay::Relay(int on,int off)
  : onPin(on)
  , offPin(off)
  , isOpen(false)
  {}
const char* Relay::getRelayStatus(){
  const char _open[] = "ON";
  const char _closed[] = "CLOSED";
  if (isOpen){
    return (_open);
  }
  else {
    return (_closed);
  }
}
void Relay::init(){
  pinMode(onPin,OUTPUT);
  pinMode(offPin,OUTPUT);
}
void Relay::openRelay(){
  isOpen = true;
  digitalWrite(onPin,HIGH);
  delay(10);
  digitalWrite(onPin,LOW);
}
void Relay::closeRelay(){
  isOpen = false;
  digitalWrite(offPin,HIGH);
  delay(10);
  digitalWrite(offPin,LOW);
}
//ACTIVE_TIMER class
ACTIVE_TIMER::ACTIVE_TIMER(Smart * smart,long d,long s){
  smartUnit=smart;
  duration=d;
  starT=s;
}
void ACTIVE_TIMER::updateTimer(float r){
  if(GPS.Fix && alt_feet!=0 && muriState==STATE_MURI_ASCENT && GPS.altitude.feet()<30000){
    duration=((maxAlt/fabs(r))*1.2);
  }
  else if(GPS.Fix && alt_feet!=0 && muriState==STATE_MURI_SLOW_DESCENT && GPS.altitude.feet()>(minAlt+30000)){
    duration=((maxAlt-minAlt/fabs(r))*1.2);
  }
}
void ACTIVE_TIMER::hammerTime(){
  if(((millis()-starT)/60000)>=duration){
    smartUnit->release();
  }
}
String ACTIVE_TIMER::getDuration() {return (String(duration));}

//ASCENT_RATE class
ASCENT_RATE::ASCENT_RATE(){
  rate=0;
  h_dot=0;
  prevh=0;
  prevt=0;
  for(int i=0;i<5;i++){
    h_dotArr[i]=0;
    hQ[i]=0;
    tQ[i]=0;
    h_dotQ[i]=0;
  }
  sum=0;
  
}
void ASCENT_RATE::updateRate(){
 rate=((alt_feet-prevh)/(getGPStime()-prevt))*60; //h_dot in feet per minute
 prevh=alt_feet;
 prevt=getGPStime();
 Serial.println("Rate: " + String(rate));
 Serial.println("Alt: " + String(prevh));
 Serial.println("Time: " + String(prevt));
}
void ASCENT_RATE::addHit(){
  Serial.println("Adding hit");
  for(int i=0;i<5;i++)
  {
    if(h_dotArr[i]==0)
    {
      h_dotArr[i]=rate;
      break;
    }
  }
  if(h_dotArr[4]!=0){
   for(int i=0;i<5;i++){
     sum+=h_dotArr[i];
     h_dotArr[i]=0; //set h dot array element equal to zero after adding to sum to prepare for next five value average
   }
   h_dot=sum/5;
   sum=0;
 }
 for(int i=0;i<5;i++){
  h_dotQ[i]=0; //set questionable array equal to zero everytime normal update happens
 } 
}
void ASCENT_RATE::checkHit(){
  Serial.println("Checking hit");
  for(int i=0;i<5;i++){
    if(h_dotQ[i]==0){
      h_dotQ[i]=rate;
      break;
    }
  }
  if(h_dotQ[4]!=0){
   for(int i=0;i<5;i++){
     sum+=h_dotQ[i];
     h_dotQ[i]=0;      //set normal and questionable array to zero after questionable array is determined to be correct
     h_dotArr[i]=0;
   }
   h_dot=sum/5;
   sum=0;
 }
}
float ASCENT_RATE::getRate(){
  return rate;
}
float ASCENT_RATE::geth_dot(){
  return h_dot;
}
float ASCENT_RATE::getPrevh(){
  return prevh;
}
float ASCENT_RATE::getPrevt(){
  return prevt;
}
String ASCENT_RATE::getHDot() {return (String(h_dot));}
