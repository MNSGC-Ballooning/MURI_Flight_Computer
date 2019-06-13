void actHeat(){
  const char ON[] = "OPEN";
  const char OFF[] = "CLOSED";
  // Compare digital temp. to critical temp.:  
  if(150 < t3 < t_low) {
    coldBattery = true; // if temperature is below low critical temperature
  }
  if(t3 > t_high) {
    coldBattery = false; // if temperature is above high critical temperature
  }
  if(150 < t4 < t_low){
    coldOPC = true;    
  }
  if(t4 > t_high){
    coldOPC = false;
  }

//turn heater on/off:
  if(coldBattery && strcmp(batHeatRelay.getRelayStatus(),OFF)==0){
    batHeatRelay.openRelay();
  }
  else if(!coldBattery && strcmp(batHeatRelay.getRelayStatus(),ON)==0){
    batHeatRelay.closeRelay();
  }
  if(coldOPC && strcmp(opcHeatRelay.getRelayStatus(),OFF)==0){
    opcHeatRelay.openRelay();
  }
  else if(!coldOPC && strcmp(opcHeatRelay.getRelayStatus(),ON)==0){
    opcHeatRelay.closeRelay();
  }
}

//Relay class functions
Relay::Relay(int on,int off)
  : onPin(on)
  , offPin(off)
  , isOpen(false)
  {}
const char* Relay::getRelayStatus(){
  const char _open[] = "ON";
  const char _closed[] = "CLOSED";
  if (isOpen){
    return (_open);
  }
  else {
    return (_closed);
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
