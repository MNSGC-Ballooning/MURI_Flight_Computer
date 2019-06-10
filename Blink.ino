void actionBlink(){
  digitalWrite(ledPin,HIGH);
  delay(10);
  digitalWrite(ledPin,LOW);
}

void fixBlink(){
  static unsigned long prevTime = 0;
  if (GPS.getFixAge()<2000){
    if(millis()-prevTime>=15000){
      digitalWrite(fix_led,HIGH);
      delay(10);
      digitalWrite(fix_led,LOW);
    }
    else{
      digitalWrite(fix_led,HIGH);
      delay(10);
      digitalWrite(fix_led,LOW);
    }
  } 
}

void logBlink(){
  digitalWrite(ledSD,HIGH);
  delay(10);
  digitalWrite(ledSD,LOW);
}
