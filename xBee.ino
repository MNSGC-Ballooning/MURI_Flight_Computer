//Takes a string to send out via xBee, and logs the transmission to the SD card with a timestamp
void sendXBee(String out) {
  Serial.println(xBeeID + ";" + out + "!");
  openEventlog();
  if(SDcard){
    eventLog.println(flightTimeStr() + "  TX  " + out);
  // eventlogB.println(flightTimeStr() + "  TX  " + out);
  }
  closeEventlog();  
}

void acknowledge() {
  Serial.println(xBeeID + "\n");
}

//Takes the string of the xBee command as well as a description and logs to the SD card with a timestamp
void logCommand(String com, String command){
  openEventlog();
  if(SDcard){
    eventLog.println(flightTimeStr() + "  RX  " + com + "  " + command);
  // eventlogB.println(flightTimeStr() + "  RX  " + com + "  " + command);
  }
  closeEventlog();
}
String lastCommand = "";
unsigned long commandTime = 0;
//Primary xBee function that looks for incoming messages, parses them, and executes the corresponding commands
void xBeeCommand(){
  boolean complete = false;                  //Stuff we don't care about
  String command = "";
  char inChar;
  while (Serial.available() > 0) {
    inChar = (char)Serial.read();
    if (inChar != ' ') {
      command += inChar;
      if (inChar == '!') { 
        complete = true;
        break;
      }
    }
    delay(10);
  }

  if (!complete) return;                                                                   //Stuff we don't care about
  if (command.equals(lastCommand) && (millis() - commandTime < 30000)) return; 
  int split = command.indexOf('?');
  if (!(command.substring(0, split)).equals(xBeeID)) return;
  lastCommand = command;
  String Com = command.substring(split + 1, command.length() - 1);
  acknowledge();
commandTime = millis();

  //receive() returns empty string if no commands sent
  if (Com.equals("")) return;

  //Find and execute correct command
  if ((Com.substring(0,2)).equals("WA")) {
    //Add time in minutes to failsafe
    unsigned long addedTime = atol((Com.substring(2, Com.length())).c_str());
    logCommand(Com, "Added time to failsafe");
    sendXBee("added Time: "+ String(addedTime)+ " Minutes");
    masterTimer += (addedTime*60*1000);  //Converts minutes to milliseconds
    sendXBee("Master Timer: " + timeLeft());
  }

 else if ((Com.substring(0,2)).equals("WR")) {
    //Remove time in minutes to failsafe
    unsigned long remTime = atol((Com.substring(2, Com.length())).c_str());
    logCommand(Com, "Removed time from failsafe");
    sendXBee("Time removed: "+ String(remTime)+ " Minutes");
    masterTimer -= (remTime*60*1000);  //Converts minutes to milliseconds
    int timeLeft = int((masterTimer-millis())/1000);
    String timeLeftStr = (String(timeLeft/60) + ":");
    timeLeft %= 60;
    timeLeftStr += (String(timeLeft / 10) + String(timeLeft % 10));
    sendXBee("Master Timer: " + timeLeftStr);
  }



   else if(Com.equals("WB")){
    //blinks the LED so you know it's connected
    testBlink();
    logCommand(Com, "Loggy log-log");
    sendXBee("Hey you figured it out. Took you long enough");
   }
  
  

  else if (Com.equals("WX")) {
    //Burns the Tungsten, enters "recovery mode" after cutdown confirmed
    runBurn();
    if(GPS.Fix){ //GPS.fix
      logCommand(Com, "Cuttdown Attempted at " + flightTimeStr() + "," + String(GPS.location.lat(), 4) + "," + String(GPS.location.lng(), 4) + ", Altitude: " + String(GPS.altitude.feet()) + "ft. FIX");  
      sendXBee("Starting Cut at Altitude " + String(GPS.altitude.feet()) + "ft. Watch your heads!");
    }
    else{
      logCommand(Com, "Cuttdown Attempted at " + flightTimeStr() + "," + String(GPS.location.lat(), 4) + "," + String(GPS.location.lng(), 4) + ", Altitude: " + String(GPS.altitude.feet()) + "ft. FIX");
      sendXBee("Starting Cut at unknown altitude, Watch your heads!");
    }
  }
  
 
  else if (Com.equals("WT")) {
    //Poll for master timer remaining, returns minutes:seconds
    logCommand(Com, "Poll Remaining Time");
    if(judgementDay){
      sendXBee(timeLeft());
    }
    else{
      sendXBee("Master timer is currently not enabled");
    }
    
  }

  
   else if (Com.equals("GPS")) {
    //Poll most recent GPS data
    logCommand(Com, "Request GPS data");
    String message = "Time: " + String(GPS.time.hour()) + ":" + String(GPS.time.minute()) + ":" + String(GPS.time.second())+ ",";   
    message += "latitude: " + String(GPS.location.lat()) + "," + "logitude: " + String(GPS.location.lng()) + "," + "altitude: " + String(GPS.altitude.feet()) + ",";
    if (GPS.Fix) message += "Fix";
    else message += "No Fix";
    sendXBee(message);
  }

   else if((Com.substring(0,2)).equals("WD")){   //disable altitude cut
    altCut = false;
    logCommand(Com, "altitude cut disabled");
    sendXBee("Altitude cutdown is now disabled");
   }
   else if ((Com.substring(0,2)).equals("WU")) {   //sets new min cutdown Altitude
    //Set Cutdown Altitude
    long newAlt = atol((Com.substring(2, Com.length())).c_str());
    logCommand(Com, "New min Cutdown Altitude: "+ String(newAlt)+ " Feet");
    sendXBee("New min Cutdown Altitude: "+ String(newAlt)+ " Feet");
    minAlt = newAlt;                                              
  }
  else if ((Com.substring(0,2)).equals("WM")) {   //sets new max cutdown Altitude
    //Set Cutdown Altitude
    long newAlt = atol((Com.substring(2, Com.length())).c_str());
    logCommand(Com, "New max Cutdown Altitude: "+ String(newAlt)+ " Feet");
    sendXBee("New max Cutdown Altitude: "+ String(newAlt)+ " Feet");
    maxAlt = newAlt;                                              
  }
  else if((Com.substring(0,2)).equals("WE")){   //enable altitude cut
    altCut = true;
    logCommand(Com, "altitude cut enabled");
    sendXBee("Altitude cutdown is now enabled, will cut at: " + String(minAlt) + "feet");
   }
  else if((Com.substring(0,2)).equals("WC")){   //enable time burn     
    judgementDay = true;
    logCommand(Com, "Master timer enabled");
    masterTimer = millis() + 7200000;                //make the default timer 2 hours
    sendXBee("Master timer enabled, time until cutdown: " + timeLeft());
  }

  else if((Com.substring(0,2)).equals("WS")){   //disable time burn
    judgementDay = false;
    logCommand(Com, "Master timer disabled");
    sendXBee("Master timer disabled");
  }

  else if((Com.substring(0,2)).equals("FE")){   
    marryPoppins=true;
    logCommand(Com, "Timed float cut enabled");
    sendXBee("Timed float cut enabled");
  }

  else if((Com.substring(0,2)).equals("FD")){
    marryPoppins=false;
    logCommand(Com, "Timed float cut disabled");
    sendXBee("Timed float cut disabled");
  }

  else if((Com.substring(0,2)).equals("FA")){  //add time to float cut in minutes
    unsigned long addedTime = atol((Com.substring(2, Com.length())).c_str());
    logCommand(Com, "Added time to floattimer");
    sendXBee("added Time: "+ String(addedTime)+ " Minutes");
    floatTimer += (addedTime*60*1000);  //Converts minutes to milliseconds
    sendXBee("Float Timer in seconds: " + String(floatTimer/1000));
  }

  else if((Com.substring(0,2)).equals("FS")){  //subtract time from float cut in minutes
    unsigned long addedTime = atol((Com.substring(2, Com.length())).c_str());
    logCommand(Com, "subtracted time to floattimer");
    sendXBee("subtracted Time: "+ String(addedTime)+ " Minutes");
    floatTimer -= (addedTime*60*1000);  //Converts minutes to milliseconds
    sendXBee("Float Timer in seconds: " + String(floatTimer/1000));
  }

  else if((Com.substring(0,2)).equals("WF")){   //poll min cutdown altitude
    logCommand(Com, "poll cutdown altitude");
    String toSend = "Cutdown altitude: " + String(minAlt);
    if(altCut){
      toSend += ",  altitude cut is enabled";
    }
    else{
      toSend += ", altitude cut is disabled";
    }
    sendXBee(toSend);
  }
  else if(Com.substring(0,2).equals("WQ")){
    bacon = true;
    logCommand(Com, "beacon enabled");
    sendXBee("beacon enabled");
  }
  else if(Com.substring(0,2).equals("WP")){
    bacon = false;
    logCommand(Com, "beacon disabled");
    sendXBee("beacon disabled");
  }
  /*else if(Com.substring(0,2).equals("TE")){
    logCommand(Com, "poll temperature");
    sendXBee("Battery Temperature: " + Temperature);
  }*/
  else if(Com.substring(0,2).equals("PF")){
    logCommand(Com, "poll float time");
    sendXBee("Float time in seconds: " + String(floatTimer/1000));
    if(floating){
      sendXBee("floating time remaining in seconds: " + String((floatTimer - (millis()-floatStart))/1000));
    }
  }
      
  else {
    //If no recognizable command was received, inform ground station
    logCommand(Com, "Unknown Command");
    sendXBee(String(Com) + ":  Command Not Recognized");
  }
}
