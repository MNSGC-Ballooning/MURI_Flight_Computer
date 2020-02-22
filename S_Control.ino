//Controller that looks at the derivative of altitude and the current altitude state
#define STATE_MURI_INIT           0x00                                  //0000 0000
#define STATE_MURI_ASCENT         0x01                                 //0000 0001
#define STATE_MURI_FAST_DESCENT   0x02                                 //0000 0010
#define STATE_MURI_SLOW_DESCENT   0x04                                 //0000 0100
#define STATE_MURI_SLOW_ASCENT    0x08                                 //0000 1000
#define STATE_MURI_CAST_AWAY      0x10                                 //0001 0000
#define STATE_MURI_RECOVERY       0x20                                 //0010 0000

#define Lock                      0xAA                                 //10101010
#define NoLock                    0xBB                                 //10111011
boolean usingGPS = false;
uint8_t muriState;
uint8_t GPSstatus = NoLock;
boolean hdotInit = false; 

void stateMachine(){
  static unsigned long castAway = 0;                                   //Counter for cast away state
  static byte initCounter = 0;                                         //Counter for initialization
  static byte skyCheck = 0;                                            //Counter for max ascent
  static byte floorCheck = 0;                                          //Counter for max slow descent
  static byte snail = 0;                                               //Counter for slow ascent
  static int lockcounter;                                              //Counter for getting GPS Lock
  static bool init = false;
  static bool fast = false;
  static bool cast = false;

  if(!init)
  {
    muriState = STATE_MURI_INIT;
    stateString = "INITIALIZATION";
    init=true;                                                         //Initalize state machine
    Serial.println("Initializing...");
  }

  if(!FirstAlt) {
    SetFirstAlt();
  }

  
               
  if(FixStatus == Fix)                                                 //Determine GPSstatus (lock or no lock)
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
    Control_Altitude = GPS.getAlt_feet();                              //Altitude equals the alitude recorded by the Ublox
                                                                       //Calculates ascent rate in ft/sec if GPS has a lock
    ascent_rate = ((Control_Altitude - prev_Control_Altitude)/(millis() - prev_time)) * 1000; 
    avg_ascent_rate = ((Control_Altitude - Begin_Altitude)/(millis() - masterClock)) * 1000;
    prev_time = millis();                                              //prev_time will equal the current time for the next loop
    prev_Control_Altitude = Control_Altitude;                          //Populate the previous altitude variabel with the current altitude for the next loop
  }                                 
  else if(GPSstatus == NoLock)
  {
     Control_Altitude = (avg_ascent_rate*((millis()-prev_time)/1000))+Control_Altitude;
     prev_Control_Altitude = Control_Altitude;
     prev_time = millis();                                             //prev_time still calculated in seconds in case GPS gets a lock on the next loop
  }
 
  stateSwitch();                                                       //Controller that changes State based on derivative of altitude
  AbortControl();                                                      //Abort porcedures for bad situations

  if (SwitchedState) {                                                 //Set these counters to zero so thay the payload must remain in the same state
    initCounter = 0;                                                   //in order to fully fill a counter
    skyCheck = 0;
    floorCheck = 0;
    snail = 0;
    SwitchedState = false;    
  }

////////////////////////////////////////  
//////////Finite State Machine//////////
////////////////////////////////////////  
    if(muriState == STATE_MURI_INIT && !hdotInit)                                   //A boolean checking to see if we initialized
    {                                                                               //You need an initialization state cause the ascent rate during the very
      if(Initial_Altitude != 0  && (Control_Altitude - Initial_Altitude) > 750)     //beginning of the flight is non-linear and you may not have a GPS fix.
      {
        initCounter++;                                                              //To change states, multiple consecutive confirmations from the change requirements must occur
        if(initCounter>5)
        {
          hdotInit=true;
          masterClock = millis();                                                   //Master clock is set to begin once the balloon gets 500 feet off the ground from where the flight began
          Begin_Altitude = GPS.getAlt_feet();                                       //Bottom altitude used for the average ascent rate calulation
          Serial.println("h_dot initialized!");
          initCounter = 0;
        }
      }
      else {
        initCounter = 0; 
      }
    }

    switch(muriState)
    {
//////////Ascent/////////     
      case 0x01:
        Serial.println("STATE_MURI_ASCENT");
        if(Control_Altitude>MAX_ALTITUDE && Control_Altitude!=0)       //Checks to see if payload has hit the max mission alt
        {
          skyCheck++;
          Serial.println("Max alt hits: " + String(skyCheck));
          if(skyCheck>5)
          {
             cutResistor();                                                           
//           CutSMARTA();                                                //If the payload is consistenly above max mission alt., the first balloon is released
//           smartTwoCut = "Reached Altitude Ceiling";
           skyCheck = 0;
          }
        }
        else 
        {
          skyCheck = 0;
        }
        break;
     
//////////Fast Descent//////////
      case 0x02:
        Serial.println("STATE_MURI_FAST_DESCENT");
        if(!fast) {
            sensorHeatRelay.setState(false);
            batHeatRelay.setState(false);
            fast=true;
        }
        break;

//////////Slow Descent//////////
      case 0x04:
        Serial.println("STATE_MURI_SLOW_DESCENT");                     //Checks to see if it is below data collection range...
        if(Control_Altitude<MIN_ALTITUDE && Control_Altitude!=0 && !LowMaxAltitude) 
        {
          floorCheck++;
          Serial.println("Min alt hits: " + String(floorCheck));
          if(floorCheck>5)                                             //If consistently below data collection range then release all balloons again just in case
          {
              cutResistor();
//            CutSMARTA();
//            smartOneCut = "Reached Slow Descent Floor";
//            CutSMARTB();
//            smartTwoCut = "Reached Slow Descent Floor";
//            floorCheck = 0;
          }
        }
        else
        {
          floorCheck = 0;
        }

        if (LowMaxAltitude && ((millis() - LowAltitudeReleaseTimer) > (LOW_MAX_ALTITUDE_CUTDOWN_TIMER*MINUTES_TO_MILLIS))) {
            cutResistor();
//          CutSMARTA();                                                 //Timer Backup system- comes standard on your balloon today!
//          smartOneCut = "Slow Descent Timer Below 80k";
//          CutSMARTB();
//          smartTwoCut = "Slow Descent Timer Below 80k";
        }
        
        break;
        
//////////Slow Ascent//////////        
        case 0x08:
        Serial.println("STATE_MURI_SLOW_ASCENT");

        if (((millis() - masterClock) > SLOW_ASCENT_ABORT_DELAY*MINUTES_TO_MILLIS) && GPSstatus == Lock) {
          snail++;
          if (snail > 20) {                                            //If your ascent rate is too slow consistently, cut!
             cutResistor();
//           CutSMARTA();
//           smartOneCut = "Too Slow Of An Ascent Rate";
//           CutSMARTB();
//           smartTwoCut = "Too Slow Of An Ascent Rate";
//           snail = 0;
          }  
        }
        else 
        {
          snail = 0;
        } 
         break;

//////////Cast Away//////////         
      case 0x10:
        Serial.println("STATE_MURI_CAST_AWAY");                        //If the balloon is stuck, it is in cast away
        if(!cast)
        {
          castAway = millis();
          cast = true;
        }
        if(millis()-castAway >= 600000)
        {
            cutResistor();
//          CutSMARTA();
//          smartOneCut = "Castaway";
//          CutSMARTB();
//          smartTwoCut = "Castaway";
        }
        break;

//////////Recovery//////////
      case 0x20:
        Serial.println("STATE_MURI_RECOVERY");
        if(!recovery)
        {
          recovery = true;
        }
        break;
    }
}

/////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////////////////////
void stateSwitch(){
  
  static byte ascent_counter = 0;
  static byte slow_ascent_counter = 0;
  static byte slow_descent_counter = 0;
  static byte fast_descent_counter = 0;
  static byte recovery_counter = 0;
  static byte wilson = 0;                                              //Counter for castaway
  if(hdotInit&&Control_Altitude!=0&&!recovery&&GPSstatus==Lock){       //If it has been initialized, it is above sea level, and it is not in recovery
    if(ascent_rate>=(5000/60) || ascent_rate<=(-5000/60)){
      Serial.println("GPS Jump Detected");
    }
    
    if(ascent_rate > (300/60) && muriState != STATE_MURI_ASCENT){      //Ascent rate trigger if the balloon is rising
      ascent_counter++;
      if (ascent_counter >= 5) {
        muriState = STATE_MURI_ASCENT;
        stateString = "ASCENT";
        ascentTimer = millis();
        ascent_counter = 0;
        SwitchedState = true;
      }
    }
    else{
      ascent_counter = 0;
    }
    
    if(ascent_rate>(50/60) && ascent_rate<=(450/60) && muriState != STATE_MURI_SLOW_ASCENT){
      slow_ascent_counter++;                                           //Slow ascent trigger if the balloon is rising too slowly
      if (slow_ascent_counter >= 5) {
        muriState = STATE_MURI_SLOW_ASCENT;
        stateString = "SLOW ASCENT";
        slow_ascent_counter = 0;
        SwitchedState = true;
      }
    }
    else {
      slow_ascent_counter = 0;
    }
    
    if(ascent_rate >= (-1500/60) && ascent_rate < (-50/60) && muriState != STATE_MURI_SLOW_DESCENT){
      slow_descent_counter++;                                          //Slow descent if one balloon cuts away
      if (slow_descent_counter >= 5) {
        muriState = STATE_MURI_SLOW_DESCENT;
        stateString = "SLOW DESCENT";
        descentTimer = millis();
        slow_descent_counter = 0;
        SwitchedState = true;

        if (Control_Altitude < MIN_ALTITUDE) {
         LowAltitudeReleaseTimer = millis();
         LowMaxAltitude = true;                                        //Preps second balloon for cutaway
        }
      }
    }
    else {
      slow_descent_counter = 0;   
    }
    
    if(ascent_rate<(-1500/60) && Control_Altitude>7000 && muriState != STATE_MURI_FAST_DESCENT){
      fast_descent_counter++;
      if (fast_descent_counter >= 5) {
        muriState = STATE_MURI_FAST_DESCENT;                           //Fast descent once both balloons cut away
        stateString = "FAST DESCENT";
        fast_descent_counter = 0;
        SwitchedState = true;
      }
    }
    else {
      fast_descent_counter = 0;
    }
    
    if(ascent_rate>=(-50/60) && ascent_rate<=(50/60) && muriState != STATE_MURI_CAST_AWAY){
      wilson++;                                                        //If the payload gets stuck and cannot rise or fall quickly
      if(wilson>50){
        muriState = STATE_MURI_CAST_AWAY;
        stateString = "CAST AWAY";
        wilson=0;
        SwitchedState = true;
      }
    }
    else {
      wilson = 0;
    }

    
    if((muriState == STATE_MURI_FAST_DESCENT || muriState == STATE_MURI_SLOW_DESCENT) && Control_Altitude<7000){
      recovery_counter++;                                              //If the payload is nearing the ground
      if (recovery_counter >= 5) {
        muriState = STATE_MURI_RECOVERY;
        stateString = "RECOVERY"; 
        SwitchedState = true;
      }
    }
    else {
      recovery_counter = 0; 
    }
  } 
}

//void CutSMARTA() {
//  CutA=true;
//  smartOneString = "RELEASED";
//}
//
//void CutSMARTB() {
//  CutB=true;
//  smartTwoString = "RELEASED";
//}

void cutResistor() {
  resistorCut = true;
  BLUETOOTH_SERIAL.write('T');
}

void AbortControl() {
  static byte termination_longitude_check = 0;
  static byte termination_latitude_check = 0;

                                                                     //Cut if the master timer is reached
  if((millis() - masterClock) >= MASTER_TIMER*MINUTES_TO_MILLIS)     //If mission time is exceeded without recovery, it cuts the balloons and just enters the recovery state
  {
      cutResistor();
      cutReason = "Master Timer";
//    CutSMARTA();
//    smartOneCut = "Master Timer";
//    CutSMARTB();
//    smartTwoCut = "Master Timer";
    muriState = STATE_MURI_RECOVERY;
    stateString = "RECOVERY";
  }

  if(GPSstatus == Lock) {                                              //Cut if the geographic boundaries are breached
       if(float(GPS.getLon() != 0) && ((float(GPS.getLon()) < WESTERN_BOUNDARY) || (float(GPS.getLon()) > EASTERN_BOUNDARY)) )
       {                                                               //Checks to see if payload is outside of longitudinal boundaries
         termination_longitude_check++;
         Serial.println("Termination Longitude check: " + String(termination_longitude_check));
         if (termination_longitude_check>5)
         {
             cutResistor();
             cutReason = "Reached Termination Longitude";
//           CutSMARTA();
//           smartOneCut = "Reached Termination Longitude";
//           CutSMARTB();
//           smartTwoCut = "Reached Termination Longitude";
           termination_longitude_check = 0;
         }
       }
       else
       {                                                               //If longitude is still in acceptable range, then reset check
         termination_longitude_check = 0;
       }

       if(float(GPS.getLat() != 0) && ((float(GPS.getLat()) > NORTHERN_BOUNDARY) || (float(GPS.getLat()) < SOUTHERN_BOUNDARY)) )
       {                                                               //Checks to see if payload is outside of latitudinal boundaries
         termination_latitude_check++;
         if (termination_latitude_check>5)
         {
             cutResistor();
             cutReason = "Reached Termination Latitude";
//           CutSMARTA();
//           smartOneCut = "Reached Termination Latitude";
//           CutSMARTB();
//           smartTwoCut = "Reached Termination Latitude";
           termination_latitude_check = 0;
         }
       }
       else
       {                                                               //If latitude is still in acceptable range, then reset check
         termination_latitude_check = 0;
       }
   }
   
   if (muriState == STATE_MURI_ASCENT && ((millis() - ascentTimer) > (LONG_ASCENT_TIMER*MINUTES_TO_MILLIS))) {
       cutResistor();
       cutReason = "Ascent Timer";
//     CutSMARTA();                                                      //Cut down if ascent takes too long
//     smartOneCut = "Ascent Timer";
//     CutSMARTB();
//     smartTwoCut = "Ascent Timer";
   }

   if (muriState == STATE_MURI_SLOW_DESCENT && ((millis() - descentTimer) > (LONG_DESCENT_TIMER*MINUTES_TO_MILLIS))) {
        cutResistor();
        cutReason = "Descent Timer";
//      CutSMARTA();                                                     //Cut down if slow descent takes too long
//      smartOneCut = "Descent Timer";
//      CutSMARTB();
//      smartTwoCut = "Descent Timer";
   }
}
