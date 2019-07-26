// LED
void actionBlink(){
  digitalWrite(Pin_LED,HIGH);
  delay(10);
  digitalWrite(Pin_LED,LOW);
}


void fixBlink(){
  static unsigned long prevTime = 0;
  if (GPS.getFixAge()<4000){
    if(millis()-prevTime>=15000){
      prevTime = millis();
      digitalWrite(Fix_LED,HIGH);
      delay(10);
      digitalWrite(Fix_LED,LOW);
    }
  }
  else{
      digitalWrite(Fix_LED,HIGH);
      delay(10);
      digitalWrite(Fix_LED,LOW);
  }
   
}


void logBlink(){
  digitalWrite(SD_LED,HIGH);
  delay(10);
  digitalWrite(SD_LED,LOW);
}
