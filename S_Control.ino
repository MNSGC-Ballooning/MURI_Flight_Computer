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
  static byte float_longitude_check = 0;   //how many times we have exceed the given longitude
  static byte termination_longitude_check = 0;
  static byte floorCheck = 0;
  static byte snail = 0;
  static bool init = false;
  static bool fast = false;
  static bool cast = false;
  static float prev_time = 0;             // previous calculated time (in milliseconds)
  static int lockcounter;                 // counter for getting GPS Lock
  static float prev_Control_Altitude = 0;     // records the most recent altitude given by GPS when it had lock
  
  

  
  if(!init)
  {
    muriState = STATE_MURI_INIT;
    stateString = "INITIALIZATION";
    init=true; // initalize state machine
    Serial.println("Initializing...");
  }
  if(millis() >= masterTimer) // if mission time is exceeded without recovery, it cuts the balloons and just enters the recovery state
  {
    CutA=true;
    CutB=true;
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

                              
  alt_pressure_library = myBaro.getAltitude()*METERS_TO_FEET;   // altitude calcuated by the pressure sensor library

  if(GPSstatus == Lock)
  {
    Control_Altitude = alt_GPS;       // altitude equals the alitude recorded by the Ublox
    ascent_rate = (((Control_Altitude - prev_Control_Altitude)/(millis() - prev_time))) * 60000; // calculates ascent rate in ft/min if GPS has a lock
    prev_time = millis(); 
    prev_Control_Altitude=Control_Altitude;// prev_time will equal the current time for the next loop
     // same idea as prev_time. millis() used if GPS loses fix and a different method for time-keeping is needed
    // prev_Control_Altitude = Control_Altitude;                                                // Is this being used anywhere?
    prev_Control_Altitude = Control_Altitude;      //Only used when determining appropiate range to use data from barometer library for altitude
    
  }                                 
  else if(GPSstatus == NoLock)
  {
     if ((alt_pressure_library < prev_Control_Altitude + 1000) && (alt_pressure_library > prev_Control_Altitude - 1000)) {    
        Control_Altitude = alt_pressure_library;                           // Control_Altitude only updated by barometer library if given altitude is within 1000ft of last GPS altitude with a lock
     }
     else {
        Control_Altitude = (ascent_rate*((millis()-prev_time)/1000))+Control_Altitude;
     }

     ascent_rate = (((Control_Altitude - prev_Control_Altitude)/(millis() - prev_time))) * 60000; // ascent rate calcutlated the same way as before, but delta t determined by millis() as GPS won't return good time data
     prev_Control_Altitude = Control_Altitude;
     prev_time = millis();                       // prev_time still calculated in seconds in case GPS gets a lock on the next loop
         

  }

 
  stateSwitch();                                //Controller that changes State based on derivative of altitude
  
////////////////////////Finite State Machine/////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
  if(Control_Altitude!=0 && Control_Altitude!=prev_Control_Altitude){
    if(FixStatus==Fix && float(GPS.getLon()) > float_longitude && GPS.getLon() != 0) // if payload drifts outide of longitude bounds and longitude is not 0 (gps has fix) * Add termination longitude check
    {
      float_longitude_check++; // and 1 to # of times outside mission area
      Serial.println("Float Longitude check: " + String(termination_longitude_check));
      if (float_longitude_check>5)
      {
        // stateSwtich function takes care of that since we falling fast.
        CutA=true;
        smartOneString = "RELEASED";
        float_longitude_check = 0;
      }
    }
    else
    {
      // if longitude is still in acceptable range, then reset check
      float_longitude_check = 0;
    }

    Serial.println("Ascent Rate: " + String(ascent_rate));

    
///////////////////////////////////////////////////////////////////////////////////////////////////////
    if(FixStatus==Fix && float(GPS.getLon()) > termination_longitude && GPS.getLon() != 0) // if payload drifts outide of longitude bounds and longitude is not 0 (gps has fix) * Add termination longitude check
    {
      termination_longitude_check++; // and 1 to # of times outside mission area
      Serial.println("Termination Longitude check: " + String(termination_longitude_check));
      if (termination_longitude_check>5)
      {
        // stateSwtich function takes care of that since we falling fast.
        CutA=true;
        CutB=true;
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

    Serial.println("Ascent Rate: " + String(ascent_rate));

///////////////////////////////////////////////////////////////////////////////////////////////////////    
    if(muriState == STATE_MURI_INIT && !hdotInit) // its a boolean checking to see if we initialized
    {
      Serial.println("STATE_MURI_INIT"); // records current state
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
        if(Control_Altitude>maxAlt && Control_Altitude!=0)// checks to see if payload has hit the max mission alt
        {
          skyCheck++;
          Serial.println("Max alt hits: " + String(skyCheck));
          if(skyCheck>5)
          {
            // if the payload is consistenly above max mission alt. , the first balloon is released
            CutA=true;
            smartOneString = "RELEASED";
            skyCheck = 0;
          }
        }
        break;
      ////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x02: // Fast Descent
        Serial.println("STATE_MURI_FAST_DESCENT");
        if(!fast)
        {
          CutB=true;
          smartTwoString = "RELEASED";
          CutA=true;
          smartOneString = "RELEASED";
//          opcHeatRelay.setState(false);
          batHeatRelay.setState(false);
          fast=true;
        }
        break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x04: // Slow Descent
        Serial.println("STATE_MURI_SLOW_DESCENT");
        if(Control_Altitude<minAlt && Control_Altitude!=0) // checks to see if it is below data collection range...
        {
          floorCheck++;
          Serial.println("Min alt hits: " + String(floorCheck));
          if(floorCheck>5)
          {
            // if consistently below data collection range then release all balloons again just in case
            CutB=true;
            smartTwoString = "RELEASED";
            CutA=true;
            smartOneString = "RELEASED";
            floorCheck = 0;
          }
        }
        else
        {
          floorCheck = 0;
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
            CutB=true;
          }
          else if(Control_Altitude>=30000)
          {
            CutA=true;
            smartOneString = "RELEASED";
            CutB=true;
            smartTwoString = "RELEASED";
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
          CutA=true;
          CutB=true;
        }
        break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x20: // Recovery
        Serial.println("STATE_MURI_RECOVERY");
        if(!recovery)
        {
//          opcRelay.setState(false);
          recovery = true;
        }
        break;
    }   
  }
}

/////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////////////////////
void stateSwitch(){
  static byte wilson = 0; // counter for castaway
  if(hdotInit && Control_Altitude!=0 && !recovery){ // if it has been initialized, it is above sea level, and it is not in recovery
    if(ascent_rate>=5000 || ascent_rate<=-5000){
      Serial.println("GPS Jump Detected");
    }
    else if(ascent_rate > 250){
      muriState = STATE_MURI_ASCENT;
      stateString = "ASCENT";
    }
    else if(ascent_rate>50 && ascent_rate<=250){
      muriState = STATE_MURI_SLOW_ASCENT;
      stateString = "SLOW ASCENT";
    }
    else if(ascent_rate >= -1500 && ascent_rate < -50){
      muriState = STATE_MURI_SLOW_DESCENT;
      stateString = "SLOW DESCENT";
      if(Control_Altitude<minAlt){ // determine minimum altitude
        minAlt=Control_Altitude-10000;
      }
    }
    else if(ascent_rate<=-2000 && Control_Altitude>7000){
      muriState = STATE_MURI_FAST_DESCENT;
      stateString = "FAST DESCENT";
    }
    else if(ascent_rate>=-50 && ascent_rate<=50){
      wilson++;
      if(wilson>100){
        muriState = STATE_MURI_CAST_AWAY;
        stateString = "CAST AWAY";
        wilson=0;
      }
    }
    else if(muriState == STATE_MURI_FAST_DESCENT && Control_Altitude<7000){
      muriState = STATE_MURI_RECOVERY;
      stateString = "RECOVERY";
    } 
  } 
}
