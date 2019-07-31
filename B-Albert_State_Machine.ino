//Controller that looks at the derivative of altitude and the current altitude state
//Use this dude for merge conflict
#define STATE_ALBERT_INITIALIZATION  0x01    //0000 0001
#define STATE_ALBERT_ASCENT          0x02    //0000 0010
#define STATE_ALBERT_DESCENT         0x04    //0000 0100
#define STATE_ALBERT_RECOVERY        0x08    //0000 1000


#define Lock    0xAA   //10101010
#define NoLock  0xBB   //10111011

uint8_t AlbertState;
uint8_t GPSstatus = NoLock;
bool BalloonBurst = false;                           // Indicates if the balloon burst instead of being released



void stateMachine(){
  static byte skyCheck = 0;
  static byte termination_longitude_check = 0;
  static byte termination_latitude_check = 0;
  static int lockcounter;                            // counter for getting GPS Lock
  static bool state_init = false;


  if(!state_init) {
    AlbertState = STATE_ALBERT_INITIALIZATION;
    stateString = "Initialization";
    state_init = true;
    StateInitTimer = millis();
    Serial.println("Initializing state machine...");
  }

  
  
  if(millis() >= masterTimer) // if mission time is exceeded without recovery, it cuts the balloons and just enters the descent state
  {
    releaseSMART();           // Function is in System
  }

  

  // determine GPSstatus (lock or no lock)
  if(GPSfix)
  {
    lockcounter++;
    if(lockcounter>=2)
    {
      GPSstatus = Lock;
    }  
  } 
  else if(!GPSfix)
  {
    GPSstatus = NoLock;
    lockcounter = 0;
  }

                              


  if(GPSstatus == Lock)
  {
    Control_Altitude = GPS.getAlt_feet();       // altitude equals the alitude recorded by the Ublox
    
    ascent_rate = (((Control_Altitude - prev_Control_Altitude)/(millis() - prev_time))) * 1000*60; // calculates ascent rate in ft/min if GPS has a lock
    prev_time = millis(); 
    prev_Control_Altitude=Control_Altitude;           // prev_time will equal the current time for the next loop
                                                      // same idea as prev_time. millis() used if GPS loses fix and a different method for time-keeping is needed
  }                                 
  
  else if(GPSstatus == NoLock)
  {
     Control_Altitude += (ascent_rate*((millis()-prev_time)/(1000*60)));
     prev_Control_Altitude = Control_Altitude;
     prev_time = millis();                       // prev_time still calculated in seconds in case GPS gets a lock on the next loop
  }


  StateSwitch();
  
////////////////////////Finite State Machine/////////////////////////  

    switch(AlbertState)
    {
      case 0x01: //Initialization

        if (millis() - StateInitTimer >= STATE_INIT_TIME) {
          AlbertState = STATE_ALBERT_ASCENT;
          stateString = "ASCENT";
        }
      
        break;


     /////////////////////////////////////////////////////////////////////////////////////////////////////
      
      case 0x02: // Ascent

        Serial.println("STATE_ALBERT_ASCENT");
      
        if(Control_Altitude != 0) {
          if(Control_Altitude > MAX_ALTITUDE){ // checks to see if payload has hit the max mission alt
            skyCheck++;
            Serial.println("Max alt hits: " + String(skyCheck));
            if(skyCheck>5){
              // if the payload is consistenly above max mission alt. , the first balloon is released
              releaseSMART();
              skyCheck = 0;  
            }
          }
          else 
          {
            skyCheck = 0;
          }

          if(GPSfix) {
            Serial.println("Check fences");
            if(float(GPS.getLon() != 0) && ((float(GPS.getLon()) < WESTERN_BOUNDARY) || (float(GPS.getLon()) > EASTERN_BOUNDARY)) ) //Checks to see if payload is outside of longitudinal boundaries
            {
              termination_longitude_check++;
              Serial.println("Termination Longitude check: " + String(termination_longitude_check));
              if (termination_longitude_check>5)
              {
                releaseSMART();
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
              Serial.println("Termination Latitude check: " + String(termination_latitude_check));
              if (termination_latitude_check>5)
              {
                releaseSMART();
                termination_latitude_check = 0;
              }
            }
            else
            {                                                       // if latitude is still in acceptable range, then reset check
              termination_latitude_check = 0;
            }
          }

        }
        
        break;
        
      /////////////////////////////////////////////////////////////////////////////////////////////////////

      case 0x04: // Descent
        
        break;

      /////////////////////////////////////////////////////////////////////////////////////////////////////
      
      case 0x08: // Recovery
   
        if(!recovery)
        {
          recovery = true;
        }
        
        break;
      
      /////////////////////////////////////////////////////////////////////////////////////////////////////

  }
}

/////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////////////////////
void StateSwitch(){
  static byte balloon_burst_check = 0;
  static byte state_init_check = 0;
  static byte recovery_check = 0;
  
  if(AlbertState == STATE_ALBERT_INITIALIZATION && GPSfix) {
    if(Control_Altitude != 0 && Control_Altitude > 5000) {
      state_init_check++;
      if (state_init_check >= 5) {
        AlbertState = STATE_ALBERT_ASCENT;
        stateString = "ASCENT";
        state_init_check = 0;
      }
    }
    else {
      state_init_check = 0;
    }
  }
  
  else if(AlbertState == STATE_ALBERT_ASCENT && GPSfix) {            //Detects if the payload started to descend because of a balloon burst
    if(ascent_rate < -2000) {
      balloon_burst_check++;
      if(balloon_burst_check >= 5) {
        releaseSMART();
        balloon_burst_check = 0;
        BalloonBurst = true;
      }
    }
    else {
      balloon_burst_check = 0;
    }
  }

  
  else if(AlbertState == STATE_ALBERT_DESCENT && GPSfix) {      //Detects if the payload is nearing the ground 
    if (Control_Altitude != 0 && Control_Altitude < 7000) {
      recovery_check++;
      if (recovery_check >= 5) {
        AlbertState = STATE_ALBERT_RECOVERY;
        stateString = "RECOVERY";
        recovery_check = 0;
      }
    }
    else {
      recovery_check = 0;      
    }
  }    

} 


void releaseSMART() {
  Cut=true;
  smartOneString = "RELEASED";
  AlbertState = STATE_ALBERT_DESCENT;
  stateString = "DESCENT";
}
