#define Fix 0x00 //0000 0000
#define NoFix 0x01  //0000 0001

uint8_t FixStatus;

void MeasurementCheck() {
  if (GPS.fixage < 2000 && CheckEstimate()) {
    FixStatus = Fix;
  }
  else {
    FixStatus = NoFix;
  }
}
