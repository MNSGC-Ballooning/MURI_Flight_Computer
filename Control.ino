// Function to control the balloon descent

unsigned long floatTimeStart = 0;
boolean floating = false;
float ascentRate = 0;

void control(){
  static unsigned int prevTime = 0;
  static float prevAlt = 0;

  ascentRate = ((GPS.altitude.feet()-prevAlt)/(getGPStime()-prevTime))*60;
  prevTime = getGPStime();
  prevAlt = GPS.altitude.feet();
  
  if(millis()>=floatTimer && !recovery){
    smartOne.release();
    smartOneString = "Released";
    floating = true;
    floatTimeStart = millis();
  }
  if(floating && millis()-floatTimeStart >= terminationTimer && !recovery){
    smartTwo.release();
    smartOne.release();
    smartTwoString = "Released";
    recovery = true;
  }
  if(recovery){
    static bool once = false;
    opcRelay.closeRelay();
    once = true;
  }
  
}
