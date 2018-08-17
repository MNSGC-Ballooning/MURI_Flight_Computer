//This file houses functions which control various LED blinking patterns
//The only reason it's in its own file is to cut down on scrolling in the main TungstenCut

String action::getName(){
  return nam;
}

Blink::Blink(){
  ondelay = 0;
  offdelay = 0;
  ontimes = 0;
  Time = 0;
}

Blink::Blink(int on, int off, int times, String NAM, unsigned long tim){
  ondelay = on;
  offdelay = off;
  ontimes = times;
  Time= tim;
  nam = NAM;
}

void Blink::BLINK(){
  if((millis()-Time>=offdelay)&&!LEDon){
    switchLED();
    Time= millis();
    if(ontimes>0){
      ontimes--;
    }
  }
  if((millis()-Time>=ondelay)&&LEDon){
    switchLED();
    Time = millis();
  }
}

int Blink::getOnTimes(){
  return ontimes;
}

void switchLED(){
  if(LEDon){
    LEDon = false;
    digitalWrite(ledPin, LOW);
  }
  else{
    LEDon= true;
    digitalWrite(ledPin, HIGH);
  }
}

void blinkMode(){
  if(currentBlink->getOnTimes()==0){
    delete currentBlink;
    if(recovery){
      currentBlink = &recoveryBlink;
    }
    else{
      currentBlink = &countdownBlink;
    }
  }
  currentBlink -> BLINK();
  
}

void testBlink(){
  currentBlink = new Blink(100,300,7, "testBlink", millis());
};
void Fixblink(){
  static unsigned long timer = 0;
  static bool fixON = false;
  static int Delay = noFixDelay;
  
  if(Copernicus.getFixAge()<2000 && Copernicus.getAlt()!= 0 ){
      Delay = noFixDelay;  
  }
  else{
    Delay = FixDelay;
  }
  if((millis()-timer>=Delay)&&!fixON){
    fixON = true;
    digitalWrite(fix_led, HIGH);
    timer= millis();
  }
  if((millis()-timer>=300)&&fixON){
    fixON = false;
    digitalWrite(fix_led, LOW);
    timer = millis();
  }
}

  

