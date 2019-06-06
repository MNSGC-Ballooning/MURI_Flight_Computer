// Relay Unit Test
// Garrett Ailts

#include <LatchRelay.h>

///////Pin Definitions/////////
#define OPC_ON 5
#define OPC_OFF 6
#define BATH_ON 7
#define BATH_OFF 8
#define OPCH_ON 24
#define OPCH_OFF 25

////////Relay Objects//////////
LatchRelay OPC(OPC_ON,OPC_OFF);
LatchRelay batHeat(BATH_ON,BATH_OFF);
LatchRelay opcHeat(OPCH_ON,OPCH_OFF);

void setup() {
  OPC.init(false);
  batHeat.init(false);
  opcHeat.init(false);

}

void loop() {
  OPC.setState(true);
  delay(1000);
  batHeat.setState(true);
  delay(1000);
  opcHeat.setState(true);
  delay(1000);
  OPC.setState(false);
  delay(1000);
  batHeat.setState(false);
  delay(1000);
  opcHeat.setState(false);
  delay(1000); 

}
