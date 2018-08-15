//PID controller that looks at the derivative of altitude and the current altitude state
#define STATE_MURI_INIT 0x00            //0000 0000
#define STATE_MURI_ASCENT 0x01          //0000 0001
#define STATE_MURI_FAST_DESCENT 0x02    //0000 0010
#define STATE_MURI_SLOW_DESCENT 0x04    //0000 0100
#define STATE_MURI_SLOW_ASCENT 0x08     //0000 1000
#define STATE_MURI_CAST_AWAY 0x10       //0001 0000
#define STATE_MURI_RECOVERY 0x20        //0010 0000

uint8_t muriState;


void stateMachine(){
  unsigned long castAway = 0;
  static byte initCounter = 0;
  static byte skyCheck = 0;
  static byte floorCheck = 0;
  static byte snail = 0;
  static bool init = false;
  static bool fast = false;
  static bool cast =false;
  static int prevTime = 0;
  if(!init){
    muriState = STATE_MURI_INIT;
    init=true;
  }
  if(millis() >= masterTimer){
    smartOne.release();
    smartTwo.release();
    muriState = STATE_MURI_RECOVERY;
  }
  blinkMode();                          //Controls Data LED that shows payload state
  Fixblink();                           //Controls LED that gives GPS fix information
  //opcControl();                       //Turns on OPC at the start of the desired altitude range
  if(muriState!=STATE_MURI_FAST_DESCENT || !recovery){
   actHeat(); 
  }
  PID();                                //Controller that changes State based on derivative of altitude
  
///////////Finite State Machine/////////////
  if(GPS.Fix && GPS.altitude.feet()!=0 && getLastGPS()-prevTime>=1){
    if(muriState == STATE_MURI_INIT && !hdotInit){
      Serial.println("STATE_MURI_INIT");
      if((float(millis())/1000)>hDOT.getPrevt()&&GPS.altitude.feet()>hDOT.getPrevh()&&hDOT.geth_dot()>250){
        hDOT.updateRate();
      }
      else{
        hDOT.checkHit();
      }
      if(GPS.altitude.feet()>5000){
        initCounter++;
        if(initCounter>5){
          hdotInit=true;
          Serial.println("h_dot initialized!");
        }
      } 
    }
    if(muriState == STATE_MURI_ASCENT){
      Serial.println("STATE_MURI_ASCENT");
      if((float(millis())/1000)>hDOT.getPrevt()&&GPS.altitude.feet()>hDOT.getPrevh()&&hDOT.geth_dot()>250){
        hDOT.updateRate();
      }
      else{
        hDOT.checkHit();
      }
      if(GPS.altitude.feet()>maxAlt){
        skyCheck++;
        Serial.println("Max alt hits: " + String(skyCheck));
        if(skyCheck>5){
          smartOne.release();
          skyCheck = 0;
        }
      } 
    }
    else if(muriState == STATE_MURI_SLOW_DESCENT){
      Serial.println("STATE_MURI_SLOW_DESCENT");
      if((float(millis())/1000)>hDOT.getPrevt() && GPS.altitude.feet()<hDOT.getPrevh() && hDOT.geth_dot()>-2000){
        hDOT.updateRate();
      }
      else{
        hDOT.checkHit();
      }
      if(GPS.altitude.feet()<minAlt){
        floorCheck++;
        Serial.println("Min alt hits: " + String(floorCheck));
        if(floorCheck>5){
          smartTwo.release();
          smartOne.release();
          floorCheck = 0;
        }
      }
    }
    else if(muriState == STATE_MURI_FAST_DESCENT){
      Serial.println("STATE_MURI_FAST_DESCENT");
      if((float(millis())/1000)>hDOT.getPrevt() && GPS.altitude.feet()<hDOT.getPrevh() && hDOT.geth_dot()>-2000){
        hDOT.updateRate();
      }
      else{
        hDOT.checkHit();
      }
      if(!fast){
        smartTwo.release();
        smartOne.release();
        opcHeatRelay.closeRelay();
        batHeatRelay.closeRelay();
        fast=true;
      }
      
    }
    else if(muriState == STATE_MURI_SLOW_ASCENT){
      Serial.println("STATE_MURI_SLOW_ASCENT");
      if((float(millis())/1000)>hDOT.getPrevt() && GPS.altitude.feet()>hDOT.getPrevh() && hDOT.geth_dot()<=250){
        hDOT.updateRate();
      }
      else{
        hDOT.checkHit();
      }
      snail++;
      if(snail>10){
        if(GPS.altitude.feet()<30000){
          smartTwo.release();
        }
        else if(GPS.altitude.feet()>=30000){
          smartOne.release();
          smartTwo.release();
        }
        snail = 0;
      }
    }
    else if(muriState == STATE_MURI_CAST_AWAY){
      if((float(millis())/1000) > hDOT.getPrevt() && hDOT.geth_dot()>-50 && hDOT.geth_dot()<50){
        hDOT.updateRate();
      }
      else{
        hDOT.checkHit();
      }
      Serial.println("STATE_MURI_CAST_AWAY");
      if(!cast){
        castAway = millis();
        cast = true;
      }
      if(millis()-castAway >= 600000){
        smartOne.release();
        smartTwo.release();
      }
    }
    else if(muriState == STATE_MURI_RECOVERY){
      Serial.println("STATE_MURI_RECOVERY");
      if(!recovery){
        sirenRelay.openRelay();
        opcRelay.closeRelay();
        recovery = true;
      }
    }
    else{
      
    }
    prevTime=getLastGPS();
  }  
}

void PID(){
  static byte wilson = 0;
  if(hdotInit && GPS.Fix && GPS.altitude.feet()!=0 && !recovery){
    Serial.println("In control loop");
    tickTock.updateTimer(hDOT.geth_dot());
    tickTock.hammerTime();
    if(hDOT.geth_dot()>=3000 || hDOT.geth_dot()<=-3000){
      Serial.println("GPS Jump Detected");
    }
    else if(hDOT.geth_dot() > 250){
      muriState = STATE_MURI_ASCENT;
    }
    else if(hDOT.geth_dot()>50 && hDOT.geth_dot()<=250){
      muriState = STATE_MURI_SLOW_ASCENT;
    }
    else if(hDOT.geth_dot() > -1000 && hDOT.geth_dot() < -50){
      static bool first = false;
      muriState = STATE_MURI_SLOW_DESCENT;
      if(!first){
        smarty = &smartTwo;
        if(GPS.altitude.feet()<minAlt){
          minAlt=GPS.altitude.feet()-10000;
        }
        first = true;
      }
    }
    else if(hDOT.geth_dot()<=-2000){
      muriState = STATE_MURI_FAST_DESCENT;
    }
    else if(hDOT.geth_dot()>=-50 && hDOT.geth_dot()<=50){
      wilson++;
      if(wilson>20){
        muriState = STATE_MURI_CAST_AWAY;
        wilson=0;
      }
    }
    else if(muriState == STATE_MURI_FAST_DESCENT && GPS.altitude.feet()<7000){
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
  if(GPS.Fix && GPS.altitude.feet()!=0 && muriState==STATE_MURI_ASCENT && GPS.altitude.feet()<30000){
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
//ASCENT_RATE class
ASCENT_RATE::ASCENT_RATE(){
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
 for(int i=0;i<5;i++){
  if(h_dotArr[i]==0){
    h_dotArr[i]=((GPS.altitude.feet()-prevh)/((float(millis())/1000)-prevt))*60; //h_dot in feet per minute
    prevh=GPS.altitude.feet();
    prevt=(float(millis())/1000);
    break;
  }
 }
 if(h_dotArr[5]!=0){
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
  for(int i=0;i<5;i++){
    if(h_dotQ[i]==0){
      h_dotQ[i]=((GPS.altitude.feet()-prevh)/((float(millis())/1000)-prevt))*60; //h_dot in feet per minute
      prevh=GPS.altitude.feet();
      prevt=(float(millis())/1000);
      break;
    }
  }
  if(h_dotQ[5]!=0){
   for(int i=0;i<5;i++){
     sum+=h_dotQ[i];
     h_dotQ[i]=0;      //set normal and questionable array to zero after questionable array is determined to be correct
     h_dotArr[i]=0;
   }
   h_dot=sum/5;
   sum=0;
 }
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




