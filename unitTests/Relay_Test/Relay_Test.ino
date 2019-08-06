// Relay Unit Test
// Garrett Ailts

#include <LatchRelay.h>

///////Pin Definitions/////////
#define OPCH_ON 3
#define OPCH_OFF 4
#define BATH_ON 5
#define BATH_OFF 6

////////Relay Objects//////////
LatchRelay batHeat(BATH_ON,BATH_OFF);
LatchRelay opcHeat(OPCH_ON,OPCH_OFF);

void setup() {
  batHeat.init(false);
  Serial.println("Battery Heater INITIALIZED");
  opcHeat.init(false);
  Serial.println("OPC Heater INITIALIZED");

}

void loop() {
  batHeat.setState(true);
  Serial.println("Battery Heater ON");
  opcHeat.setState(true);
  Serial.println("OPC Heater ON");
  delay(5000);
  batHeat.setState(false);
  Serial.println("Battery Heater OFF");
  opcHeat.setState(false);
  Serial.println("OPC Heater OFF");
  delay(5000); 

}
