//Controller that looks at the derivative of altitude and the current altitude state
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
  static unsigned long prevTimes = 0;


  //keep these variables as static? 
  static float alt_feet = 0;              // final altitude used between alt_GPS and alt_pressure depending on if we have a GPS lock
  static float alt_GPS = 0;               // altitude calculated by the GPS in feet
  static float alt_pressure = 0;          // altitude calculated by the pressure sensor in feet
  static float alt_pressure_library = 0;  // altitiude calculated by the pressure sensor library
  static float prev_alt_feet = 0;         // previous calculated altitude
  static float prev_time = 0;             // previous calculated time (in seconds)
  static float prev_time_millis = 0;      // previous calculated time (in milliseconds)
  static float ascent_rate = 0;           // ascent rate of payload in feet per minute
  static int i; // counter for getting GPS Lock
  static float alt_pressure;
  static float alt_GPS;

  
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

  alt_GPS = Ublox.getAlt_feet();                                // altitude calulated by the Ublox GPS
  alt_pressure_library = mybaro.getaltitude()*METERS_TO_FEET;   // altitude calcuated by the pressure sensor library
  alt_pressure = Pressure_Alt_Calc(kpa*1000, t2);               // altitude calculated by the Hypsometric formula using pressure sensor data


  // determine the best altitude to use based on lock or no lock
  if(GPSstatus == Lock)
  {
    alt_feet = alt_GPS;                                                         // altitude equals the alitude recorded by the Ublox
    ascent_rate = ((alt_feet - prev_alt_feet)/(getLastGPS() - prev_time)) * 60; // calculates ascent rate in ft/min if GPS has a lock
    prev_time = getLastGPS();                                                   // prev_time will equal the current time for the next loop
    prev_time_millis = millis();                                                // same idea as prev_time. millis() used if GPS loses fix and a different method for time-keeping is needed
    prev_alt_feet = alt_feet;                                                   // same idea for prev_time but applied to prev_alt_feet
  }                                 
  else if(GPSstatus == NoLock)
  {
     if (alt_pressure_library != 0) {  
       alt_feet = alt_pressure_library;                          // alt_feet calculated by pressure sensor library function if GPS has no lock
     }
     else {
       alt_feet = alt_pressure;                                  // alt_feet calculated by Hypsometric forumla if pressure sensor library function doesn't work
     }

    ascent_rate = ((alt_feet - prev_alt_feet)/(millis() - prev_time_millis)) * 60000; // ascent rate calcutlated the same way as before, but delta t determined by millis() as GPS won't return good time data
    prev_time = prev_time + (millis() - prev_time_millis)/1000;                       // prev_time still calculated in seconds in case GPS gets a lock on the next loop
    prev_time_millis = millis();

    prev_alt_feet = alt_feet;     

  }
  
  stateSwitch();                                //Controller that changes State based on derivative of altitude
  
///////////Finite State Machine///////////////
//Serial.println("GLGPS: " + String(getLastGPS()));
//Serial.println("Prev time: " + String(prevTimes));
  if(alt_feet!=0 && millis()-prevTimes > 1000 && alt_feet!=prev_alt_feet) 
  {
    if(GPS.Fix && float(GPS.location.lng()) > termination_longitude && GPS.location.lng() != 0) // if payload drifts outide of longitude bounds and longitude is not 0 (gps has fix)
    {
      termination_longitude_check++; // and 1 to # of times outside mission area
      Serial.println("Termination Longitude check: " + String(termination_longitude_check));
      if (termination_longitude_check>5)
      {
        // stateSwtich function takes care of that since we falling fast.
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

    Serial.println("Ascent Rate: " + String(ascent_rate);



///////////////////////////////////////////////////////////////////////////////////////////////////////    
    if(muriState == STATE_MURI_INIT && !hdotInit) // its a boolean checking to see if we initialized
    {
      Serial.println("STATE_MURI_INIT"); // records current state
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
    // You need an initialization state cause the ascent rate during the very beginning of the flight is non-linear and you may not have a GPS fix.


    switch(muriState)
    {
      case 0x01 // Ascent
        Serial.println("STATE_MURI_ASCENT");
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
        break;
      ////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x02 // Fast Descent
        Serial.println("STATE_MURI_FAST_DESCENT");
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
        break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x04 // Slow Descent
        Serial.println("STATE_MURI_SLOW_DESCENT");
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
        break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x08 // Slow Ascent
        Serial.println("STATE_MURI_SLOW_ASCENT");
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
         break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x10 // Cast Away
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
        break;
      /////////////////////////////////////////////////////////////////////////////////////////////////////
      case 0x20 // Recovery
        Serial.println("STATE_MURI_RECOVERY");
        if(!recovery)
        {
          sirenRelay.openRelay();
          opcRelay.closeRelay();
          recovery = true;
        }
        break;
    }   
  }
}

/////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////////////////////




void stateSwitch()
{
  static byte wilson = 0; // counter for castaway
  if(hdotInit && alt_feet!=0 && !recovery) // if it has been initialized, it is above sea level, and it is not in recovery
  {
    if(ascent_rate!=0)
    {
      tickTock.updateTimer(hDOT.geth_dot());
    }
    tickTock.hammerTime();
    if(ascent_rate>=5000 || ascent_rate<=-5000)
    {
      Serial.println("GPS Jump Detected");
    }
    else if(ascent_rate > 250)
    {
      muriState = STATE_MURI_ASCENT;
      stateString = "ASCENT";
    }
    else if(ascent_rate>50 && ascent_rate<=250)
    {
      muriState = STATE_MURI_SLOW_ASCENT;
      stateString = "SLOW ASCENT";
    }
    else if(ascent_rate >= -1500 && ascent_rate < -50)
    {
      static bool first = false;
      muriState = STATE_MURI_SLOW_DESCENT;
      stateString = "SLOW DESCENT";
      if(!first)
      {
        smarty = &smartTwo; // what does this do? Sets a pointer pointing to a given SMART unit on an active timer to be releases
        if(alt_feet<minAlt) // determine minimum altitude
        {
          minAlt=alt_feet-10000;
        }
        first = true;
      }
    }

    else if(ascent_rate<=-2000 && alt_feet>7000)
    {
      muriState = STATE_MURI_FAST_DESCENT;
      stateString = "FAST DESCENT";
    }
    else if(ascent_rate=-50 && ascent_rate<=50)
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
