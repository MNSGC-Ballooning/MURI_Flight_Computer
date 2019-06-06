//Relay class functions
Relay::Relay(int on,int off)
  : onPin(on)
  , offPin(off)
  , isOpen(false)
  {}
String Relay::getRelayStatus(){
  String Open = "ON";
  String Close = "CLOSED";
  if (isOpen){
    return (Open);
  }
  else {
    return (Close);
  }
}
void Relay::init(){
  pinMode(onPin,OUTPUT);
  pinMode(offPin,OUTPUT);
}
void Relay::openRelay(){
  isOpen = true;
  digitalWrite(onPin,HIGH);
  delay(10);
  digitalWrite(onPin,LOW);
}
void Relay::closeRelay(){
  isOpen = false;
  digitalWrite(offPin,HIGH);
  delay(10);
  digitalWrite(offPin,LOW);
}
