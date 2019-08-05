//Controller that looks at the derivative of altitude and the current altitude state
#define STATE_MURI_INIT           0x00    //0000 0000
#define STATE_MURI_ASCENT         0x01    //0000 0001
#define STATE_MURI_FAST_DESCENT   0x02    //0000 0010
#define STATE_MURI_SLOW_DESCENT   0x04    //0000 0100
#define STATE_MURI_SLOW_ASCENT    0x08    //0000 1000
#define STATE_MURI_CAST_AWAY      0x10    //0001 0000
#define STATE_MURI_RECOVERY       0x20    //0010 0000

#define Lock    0xAA   //10101010
#define NoLock  0xBB   //10111011
boolean usingGPS = false;
uint8_t muriState;
uint8_t GPSstatus = NoLock;
//float ascent_rate = 0;     // ascent rate of payload in feet per minute
boolean hdotInit = false; 



void stateMachine(){
  static unsigned long castAway = 0;
  static byte initCounter = 0;
  static byte skyCheck = 0;
  static byte termination_longitude_check = 0;
  static byte termination_latitude_check = 0;
  static byte floorCheck = 0;
  static byte snail = 0;
  static bool init = false;
  static bool fast = false;
  static bool cast = false;
//  static float prev_time = 0;             // previous calculated time (in milliseconds)
  static int lockcounter;                 // counter for getting GPS Lock
//  static float prev_Control_Altitude = 0;     // records the most recent altitude given by GPS when it had lock

  
  

  
  if(!init)
  {
    muriState = STATE_MURI_INIT;
    stateString = "INITIALIZATION";
    init=true; // initalize state machine
    Serial.println("Initializing...");
  }
  if(millis() >= masterTimer) // if mission time is exceeded without recovery, it cuts the balloons and just enters the recovery state
  {
    CutSMARTA();
    CutSMARTB();
    muriState = STATE_MURI_RECOVERY;
    stateString = "RECOVERY";
  }
  if(muriState!=STATE_MURI_FAST_DESCENT || !recovery)
  {
   actHeat(); 
  }
  

  // determine GPSstatus (lock or no lock)
  if(FixStatus == Fix)
  {
    lockcounter++;
    if(lockcounter>=2)
    {
      GPSstatus = Lock;
    }  
  } 
  else if(FixStatus == NoFix)
  {
    GPSstatus = NoLock;
    lockcounter = 0;
  }



  if(GPSstatus == Lock)
  {
    Control_Altitude = GPS.getAlt_feet();       // altitude equals the alitude recorded by the Ublox
    
    ascent_rate = (((Control_Altitude - prev_Control_Altitude)/(millis() - prev_time))) * 1000; // calculates ascent rate in ft/sec if GPS has a lock
    prev_time = millis(); 
    prev_Control_Altitude=Control_Altitude;// prev_time will equal the current time for the next loop
     // same idea as prev_time. millis() used if GPS loses fix and a different method for time-keeping is needed
    // prev_Control_Altitude = Control_Altitude;                                                // Is this being used anywhere?
    prev_Control_Altitude = Control_Altitude;      //Only used when determining appropiate range to use data from barometer library for altitude
  }                                 
  else if(GPSstatus == NoLock)
  {
//     if ((alt_pressure_library < prev_Control_Altitude + 1000) && (alt_pressure_library > prev_Control_Altitude - 1000)) {    
//        Control_Altitude = alt_pressure_library;                           // Control_Altitude only updated by barometer library if given altitude is within 1000ft of last GPS altitude with a lock
//     }
//     if {
        Control_Altitude = (ascent_rate*((millis()-prev_time)/1000))+Control_Altitude;
//     }

     ascent_rate = (((Control_Altitude - prev_Control_Altitude)/(millis() - prev_time))) * 1000; // ascent rate calcutlated the same way as before, but delta t determined by millis() as GPS won't return good time data
     prev_Control_Altitude = Control_Altitude;
     prev_time = millis();                       // prev_time still calculated in seconds in case GPS gets a lock on the next loop
  }


 
  stateSwitch();                                //Controller that changes State based on derivative of altitude
  AbortControl();
  
////////////////////////Finite State Machine/////////////////////////

   if(GPSstatus == Lock) {
           if(float(GPS.getLon() != 0) && ((float(GPS.getLon()) < WESTERN_BOUNDARY) || (float(GPS.getLon()) > EASTERN_BOUNDARY)) ) //Checks to see if payload is outside of longitudinal boundaries
           {
             termination_longitude_check++;
             Serial.println("Termination Longitude check: " + String(termination_longitude_check));
             if (termination_longitude_check>5)
             {
               CutSMARTA();
               CutSMARTB();
               termination_longitude_check = 0;
             }
           }
           else
           {                                                       // if longitude is still in acceptable range, then reset check
             termination_longitude_check = 0;
           }


           if(float(GPS.getLat() != 0) && ((float(GPS.getLat()) > NORTHERN_BOUNDARY) || (float(GPS.getLat()) < SOUTHERN_BOUNDARY)) ) //Checks to see if payload is outside of latitudinal boundaries
           {
             termination_latitude_check++;
             if (termination_latitude_check>5)
             {
               CutSMARTA();
               CutSMARTB();
               termination_latitude_check = 0;
             }
           }
           else
           {                                                       // if latitude is still in acceptable range, then reset check
             termination_latitude_check = 0;
           }
       }



///////////////////////////////////////////////////////////////////////////////////////////////////////    
    if(muriState == STATE_MURI_INIT && !hdotInit) // its a boolean checking to see if we initialized
    {
//      Serial.println("STATE_MURI_INIT"); // records current state
      if(Control_Altitude>5000)
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
    // You need an initialization state cause the ascent rate during the very beginning of the flight is non-linear and you may not have a GPS fix.


    switch(muriState)
    {
      case 0x01: // Ascent
        Serial.println("STATE_MURI_ASCENT");
        if(Control_Altitude>MAX_ALTITUDE && Control_Altitude!=0)// checks to see if payload has hit the max mission alt
        {
          skyCheck++;
          Serial.println("Max alt hits: " + String(skyCheck));
          if(skyCheck>5)
          {
            // if the payload is consistenly above max mission alt. , the first balloon is released
           CutSMARTA();
           skyCheck = 0;
          }
        }
        break;
      ////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x02: // Fast Descent
        Serial.println("STATE_MURI_FAST_DESCENT");
        if(!fast) {
            opcHeatRelay.setState(false);
            batHeatRelay.setState(false);
            fast=true;
        break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x04: // Slow Descent
        Serial.println("STATE_MURI_SLOW_DESCENT");
        if(Control_Altitude<MIN_ALTITUDE && Control_Altitude!=0 && !LowMaxAltitude) // checks to see if it is below data collection range...
        {
          floorCheck++;
          Serial.println("Min alt hits: " + String(floorCheck));
          if(floorCheck>5)
          {
            // if consistently below data collection range then release all balloons again just in case
            CutSMARTA();
            CutSMARTB();
            floorCheck = 0;
          }
        }
        else
        {
          floorCheck = 0;
        }

        if (LowMaxAltitude && ((millis() - LowAltitudeReleaseTimer) > LOW_MAX_ALTITUDE_CUTDOWN_TIMER)) {
          CutSMARTA();
          CutSMARTB();
        }
        break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x08: // Slow Ascent
        Serial.println("STATE_MURI_SLOW_ASCENT");
        snail++;
        if(snail>10)
        {
          if(Control_Altitude<30000)
          {
            CutSMARTB();
          }
          else if(Control_Altitude>=30000)
          {
            CutSMARTA();
            CutSMARTB();
          }
          snail = 0;
         }
         break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x10: // Cast Away
        Serial.println("STATE_MURI_CAST_AWAY");
        if(!cast)
        {
          castAway = millis();
          cast = true;
        }
        if(millis()-castAway >= 600000)
        {
          CutSMARTA();
          CutSMARTB();
        }
        break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x20: // Recovery
        Serial.println("STATE_MURI_RECOVERY");
        if(!recovery)
        {
          recovery = true;
        }
        break;
    }
  }
}   



/////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////////////////////
void stateSwitch(){
  static byte ascent_counter = 0;
  static byte slow_ascent_counter = 0;
  static byte slow_descent_counter = 0;
  static byte fast_descent_counter = 0;
  static byte recovery_counter = 0;
  static byte wilson = 0; // counter for castaway
  if(hdotInit && Control_Altitude!=0 && !recovery){ // if it has been initialized, it is above sea level, and it is not in recovery
    if(ascent_rate>=(5000/60) || ascent_rate<=(-5000/60)){
      Serial.println("GPS Jump Detected");
    }
    
    if(ascent_rate > (300/60) && muriState != STATE_MURI_ASCENT){
      ascent_counter++;
      if (ascent_counter >= 5) {
        muriState = STATE_MURI_ASCENT;
        stateString = "ASCENT";
        ascentTimer = millis();
        ascent_counter = 0;
      }
    }
    else{
      ascent_counter = 0;
    }
    
    if(ascent_rate>(50/60) && ascent_rate<=(300/60) && muriState != STATE_MURI_SLOW_ASCENT){
      slow_ascent_counter++;
      if (slow_ascent_counter >= 5) {
        muriState = STATE_MURI_SLOW_ASCENT;
        stateString = "SLOW ASCENT";
        slow_ascent_counter = 0;
      }
    }
    else {
      slow_ascent_counter = 0;
    }
    
    if(ascent_rate >= (-1500/60) && ascent_rate < (-50/60) && muriState != STATE_MURI_SLOW_DESCENT){
      slow_descent_counter++;
      if (slow_descent_counter >= 5) {
        muriState = STATE_MURI_SLOW_DESCENT;
        stateString = "SLOW DESCENT";
        descentTimer = 0;
        slow_descent_counter = 0;

        if (Control_Altitude < MIN_ALTITUDE) {
         LowAltitudeReleaseTimer = millis();
         LowMaxAltitude = true; 
        }
      }
    }
    else {
      slow_descent_counter = 0;   
    }
    
    if(ascent_rate<=(-2000/60) && Control_Altitude>7000 && muriState != STATE_MURI_FAST_DESCENT){
      fast_descent_counter++;
      if (fast_descent_counter >= 5) {
        muriState = STATE_MURI_FAST_DESCENT;
        stateString = "FAST DESCENT";
        fast_descent_counter = 0;
      }
    }
    else {
      fast_descent_counter = 0;
    }
    
    if(ascent_rate>=(-50/60) && ascent_rate<=(50/60) && muriState != STATE_MURI_CAST_AWAY){
      wilson++;
      if(wilson>50){
        muriState = STATE_MURI_CAST_AWAY;
        stateString = "CAST AWAY";
        wilson=0;
      }
    }
    else {
      wilson = 0;
    }

    
    if((muriState == STATE_MURI_FAST_DESCENT || muriState == STATE_MURI_SLOW_DESCENT) && Control_Altitude<7000){
      muriState = STATE_MURI_RECOVERY;
      stateString = "RECOVERY";
    } 
  } 
}



void CutSMARTA() {
  CutA=true;
  smartOneString = "RELEASED";
}

void CutSMARTB() {
  CutB=true;
  smartTwoString = "RELEASED";
}


void AbortControl() {

   //Cut down if ascent takes too long 
   if (muriState == STATE_MURI_ASCENT && ((ascentTimer - millis()) > LONG_ASCENT_TIMER)) {
     CutSMARTA();
     CutSMARTB();
   }


   //Cut down if slow descent takes too long
   if (muriState == STATE_MURI_SLOW_DESCENT && ((descentTimer - millis()) > LONG_DESCENT_TIMER)) {
      CutSMARTA();
      CutSMARTB();
   }

}
