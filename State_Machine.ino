//PID controller that looks at the derivative of altitude and the current altitude state
#define STATE_MURI_ASCENT 0x00          //0000 0001
#define STATE_MURI_FAST_DESCENT 0x02    //0000 0010
#define STATE_MURI_SLOW_DESCENT 0x04    //0000 0100
#define STATE_MURI_SLOW_ASCENT 0x08     //0000 1000
#define STATE_MURI_CAST_AWAY 0x10       //0001 0000
#define STATE_MURI_RECOVERY 0x20        //0010 0000
uint8_t muriState;
unsigned long castAway = 0;

void stateMachine(){
  static byte skyCheck = 0;
  static byte floorCheck = 0;
  static byte snail = 0;
  static bool init = false;
  if(!init){
    muriState = STATE_MURI_ASCENT;
    init=true;
  }
  if(millis() >= masterTimer){
    smartOne.release();
    smartTwo.release();
    muriState = STATE_MURI_RECOVERY;
  }
  blinkMode();                          //Controls Data LED that shows payload state
  Fixblink();                           //Controls LED that gives GPS fix information
  opcControl();                         //Turns on OPC at the start of the desired altitude range
  PID();                                //Controller that changes State based on derivative of altitude
  
///////////Finite State Machine/////////////
  if(muriState == STATE_MURI_ASCENT){
    if(GPS.Fix && GPS.altitude.feet()!=0){
      if(GPS.altitude.feet()>maxAlt){
        skyCheck++;
        if(skyCheck>5){
          smartOne.release();
          skyCheck = 0;
        }
      }
    }
  }
  else if(muriState == STATE_MURI_SLOW_DESCENT){
    smarty = &smartTwo;
    if(GPS.Fix && GPS.altitude.feet()!=0){
      if(GPS.altitude.feet()<minAlt){
        floorCheck++;
        if(floorCheck>5){
          smartTwo.release();
          smartOne.release();
          floorCheck = 0;
        }
      }
    }
  }
  else if(muriState == STATE_MURI_FAST_DESCENT){
    opcRelay.closeRelay();
    opcHeatRelay.closeRelay();
    batHeatRelay.closeRelay();
  }
  else if(muriState == STATE_MURI_SLOW_ASCENT){
    if(GPS.Fix && GPS.altitude.feet()!=0){
      if(GPS.altitude.feet()<minAlt){
        snail++;
        if(snail>5){
          smartOne.release();
          //smartTwo.release();
          snail = 0;
        }
      }
    }
  }
  else if(muriState == STATE_MURI_CAST_AWAY){
    castAway = millis();
    if(millis()-castAway >= 60000){
      smartOne.release();
      smartTwo.release();
    }
  }
  else if(muriState == STATE_MURI_RECOVERY){
    recovery = true;
    //siren on, add when Asif makes relay cicuit
  }
  else{
    
  }
 
}

void PID(){
  static bool hdotInit = false;
  static unsigned long prevAlt = 0;
  static int prevTime = 200000;
  static byte wilson = 0;
  static float h_dot=0; 
  if(GPS.Fix && GPS.altitude.feet()!=0){
    if(getLastGPS()>prevTime){
      h_dot=((GPS.altitude.feet()-prevAlt)/(getLastGPS()-prevTime))*60; //h_dot in feet per minute
      if(!hdotInit){
        hdotInit = true;
      }
    }
    prevTime = getLastGPS;
    prevAlt = GPS.altitude.feet(); 
  }
  if(hdotInit && GPS.Fix && GPS.altitude.feet()!=0){
    tickTock.updateTimer(h_dot);
    tickTock.hammerTime();
    if(h_dot>5000 || h_dot<-5000){
      Serial.println("GPS Jump Detected");
    }
    else if(h_dot > 250){
      muriState = STATE_MURI_ASCENT;
    }
    else if(h_dot>50 && h_dot<250){
      muriState = STATE_MURI_SLOW_ASCENT;
    }
    else if(h_dot > -500 && h_dot < -50){
      muriState = STATE_MURI_SLOW_DESCENT;
      if(GPS.altitude.feet()<minAlt){
        minAlt = GPS.altitude.feet()-10000;
      }
    }
    else if(h_dot<=-500){
      muriState = STATE_MURI_FAST_DESCENT;
    }
    else if(h_dot>-50 && h_dot<50){
      wilson++;
      if(castAway>15){
        muriState = STATE_MURI_CAST_AWAY;
      }
    }
    else if(muriState == STATE_MURI_FAST_DESCENT && GPS.altitude.feet()<5000){
      muriState = STATE_MURI_RECOVERY;
    }
    else{
      
    }
    
  } 
}
void opcControl(){
  static byte checktimes;
  if(!opcON && GPS.altitude.feet()>=75,000){
    checktimes++;
    if(checktimes>=15){
      opcRelay.openRelay();
      opcON=true;
    }
  }
}

/////////Control Classes Definitions////////////
//Relay class functions
Relay::Relay(int on,int off){
  onPin=on;
  offPin=off;
}
void Relay::init(){
  pinMode(onPin,OUTPUT);
  pinMode(offPin,OUTPUT);
}
void Relay::openRelay(){
  digitalWrite(onPin,HIGH);
  delay(10);
  digitalWrite(onPin,LOW);
}
void Relay::closeRelay(){
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
  if(GPS.Fix && GPS.altitude.feet()!=0 && muriState==STATE_MURI_ASCENT && GPS.altitude.feet()<(maxAlt-30000)){
    duration=(((maxAlt-GPS.altitude.feet())/fabs(r))*1.5);
  }
  else if(GPS.Fix && GPS.altitude.feet()!=0 && muriState==STATE_MURI_SLOW_DESCENT && GPS.altitude.feet()>(minAlt+30000)){
    duration=(((GPS.altitude.feet()-minAlt)/fabs(r))*1.5);
  }
}
void ACTIVE_TIMER::hammerTime(){
  if(millis()-starT>=duration){
    smartUnit->release();
  }
}
