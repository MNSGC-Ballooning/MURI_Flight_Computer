//PID controller that looks at the derivative of altitude and the current altitude state
#define STATE_MURI_INIT 0x00            //0000 0000
#define STATE_MURI_ASCENT 0x01          //0000 0001
#define STATE_MURI_FAST_DESCENT 0x02    //0000 0010
#define STATE_MURI_SLOW_DESCENT 0x04    //0000 0100
#define STATE_MURI_SLOW_ASCENT 0x08     //0000 1000
#define STATE_MURI_CAST_AWAY 0x10       //0001 0000
#define STATE_MURI_RECOVERY 0x20        //0010 0000

uint8_t muriState;
String stateString = "";


void stateMachine(){
  unsigned long castAway = 0;
  static byte initCounter = 0;
  static byte skyCheck = 0;
  static byte floorCheck = 0;
  static byte snail = 0;
  static bool init = false;
  static bool first = false;
  static bool fast = false;
  static bool cast =false;
  if(!init){
    muriState = STATE_MURI_INIT;
    stateString = "INITIALIZATION";
    init=true;
  }
  if(millis() >= masterTimer){
    smartOne.release();
    smartTwo.release();
    muriState = STATE_MURI_RECOVERY;
    stateString = "RECOVERY";
  }
  blinkMode();                          //Controls Data LED that shows payload state
  Fixblink();                           //Controls LED that gives GPS fix information
  //opcControl();                       //Turns on OPC at the start of the desired altitude range
  if(muriState!=STATE_MURI_FAST_DESCENT || !recovery){
   actHeat(); 
  }
  PID();                                //Controller that changes State based on derivative of altitude
  
///////////Finite State Machine/////////////
  if(muriState == STATE_MURI_INIT && !hdotInit){
    Serial.println("STATE_MURI_INIT");
    if(GPS.Fix && GPS.altitude.feet()!=0){
      if(GPS.altitude.feet()>2000){
        initCounter++;
        if(initCounter>10){
          hdotInit=true;
          Serial.println("h_dot initialized!");
        }
      }
    }
  }
  if(muriState == STATE_MURI_ASCENT){
    Serial.println("STATE_MURI_ASCENT");
    if(GPS.Fix && GPS.altitude.feet()!=0){
      if(GPS.altitude.feet()>maxAlt){
        skyCheck++;
        Serial.println("Max alt hits: " + String(skyCheck));
        if(skyCheck>10){
          smartOne.release();
          smartOneString = "RELEASED";
          if(!first){
            if(GPS.altitude.feet()<minAlt){
              minAlt=minAlt-10000;
            }
            first=true;
          }
          skyCheck = 0;
        }
      }
    }
  }
  else if(muriState == STATE_MURI_SLOW_DESCENT){
    Serial.println("STATE_MURI_SLOW_DESCENT");
    smarty = &smartTwo;
    if(GPS.Fix && GPS.altitude.feet()!=0){
      if(GPS.altitude.feet()<minAlt){
        floorCheck++;
        Serial.println("Min alt hits: " + String(floorCheck));
        if(floorCheck>10){
          smartTwo.release();
          smartTwoString = "RELEASED";
          smartOne.release();
          smartOneString = "RELEASED";
          floorCheck = 0;
        }
      }
    }
  }
  else if(muriState == STATE_MURI_FAST_DESCENT){
    Serial.println("STATE_MURI_FAST_DESCENT");
    if(!fast){
      smartTwo.release();
      smartTwoString = "RELEASED";
      smartOne.release();
      smartOneString = "RELEASED";
      fast=true;
    }
    opcRelay.closeRelay();
    opcHeatRelay.closeRelay();
    batHeatRelay.closeRelay();
  }
  else if(muriState == STATE_MURI_SLOW_ASCENT){
    Serial.println("STATE_MURI_SLOW_ASCENT");
    if(GPS.Fix && GPS.altitude.feet()!=0){
      if(GPS.altitude.feet()<minAlt){
        snail++;
        if(snail>10){
          smartOne.release();
          smartOneString = "RELEASED";
          //smartTwo.release();
          snail = 0;
        }
      }
    }
  }
  else if(muriState == STATE_MURI_CAST_AWAY){
    Serial.println("STATE_MURI_CAST_AWAY");
    if(!cast){
      castAway = millis();
      cast = true;
    }
    if(millis()-castAway >= 600000){
      smartOne.release();
      smartOneString = "RELEASED";
      smartTwo.release();
      smartTwoString = "RELEASED";
    }
  }
  else if(muriState == STATE_MURI_RECOVERY){
    Serial.println("STATE_MURI_RECOVERY");
    recovery = true;
    //siren on, add when Asif makes relay cicuit
    sirenRelay.openRelay();
  }
  else{
    
  }
 
}

void PID(){
  static byte ascent = 0;
  static byte ascentS = 0;
  static byte descentS = 0;
  static byte descentF = 0;
  static byte recov = 0;
  static unsigned long prevAlt = 0;
  static int prevTime = 0;
  static unsigned long prevT = 0;
  static byte wilson = 0;
  static float h_dot=0; 
  if(GPS.Fix && GPS.altitude.feet()!=0){
    if(getLastGPS()-prevTime>2){
      h_dot=((GPS.altitude.feet()-prevAlt)/(getLastGPS()-prevTime))*60; //h_dot in feet per minute
      Serial.println("h_dot was set!");
      Serial.println("h_dot = " + String(h_dot));
    }
    prevTime = getLastGPS();
    prevAlt = GPS.altitude.feet();
  }
  if(hdotInit && GPS.Fix && GPS.altitude.feet()!=0 && !recovery){
    Serial.println("In control loop");
    tickTock.updateTimer(h_dot);
    tickTock.hammerTime();
    if(h_dot>=3000 || h_dot<=-3000){
      Serial.println("GPS Jump Detected");
    }
    else if(h_dot > 300){
      ascent++;
      if(ascent>5){
        muriState = STATE_MURI_ASCENT;
        stateString = "ASCENT";
        ascent = 0;
      }
    }
    else if(h_dot>25 && h_dot<=300){
      ascentS++;
      if(ascentS>5){
        muriState = STATE_MURI_SLOW_ASCENT;
        stateString = "SLOW ASCENT";
        ascentS=0;
      }
    }
    else if(h_dot > -800 && h_dot < -25){
      descentS++;
      if(descentS>5){
        muriState = STATE_MURI_SLOW_DESCENT;
        stateString = "SLOW DESCENT";
        descentS=0;
      }
      
    }
    else if(h_dot<=-1500 && GPS.altitude.feet()>=7000){
      descentF++;
      if(descentF>20){
        muriState = STATE_MURI_FAST_DESCENT;
        stateString = "FAST DESCENT";
        descentF = 0;
      }
    }
    else if(muriState == STATE_MURI_ASCENT && h_dot>=-25 && h_dot<=25 && GPS.altitude.feet()>minAlt){
      wilson++;
      if(wilson>100){
        muriState = STATE_MURI_CAST_AWAY;
        stateString = "CAST AWAY";
        wilson=0;
      }
    }
    else if(muriState == STATE_MURI_FAST_DESCENT && GPS.altitude.feet()<7000){
      recov++;
      if(recov>100){
        muriState = STATE_MURI_RECOVERY;
        stateString = "RECOVERY";
        recov=0;
      }
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
String Relay::getRelayStatus(){
  return (relayStatus);
}
void Relay::init(){
  pinMode(onPin,OUTPUT);
  pinMode(offPin,OUTPUT);
}
void Relay::openRelay(){
  relayStatus = "OPEN";
  digitalWrite(onPin,HIGH);
  delay(10);
  digitalWrite(onPin,LOW);
}
void Relay::closeRelay(){
  relayStatus = "CLOSED";
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
  if(((millis()-starT)/60000)>=duration){
    smartUnit->release();
  }
}
String ACTIVE_TIMER::getDuration() {return (String(duration));)}
