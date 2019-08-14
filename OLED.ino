//OLED functions
void oledOn(MicroOLED &named){ named.command(DISPLAYON); }              //Turn on Display
                                                                        
void oledOff(MicroOLED &named){ named.command(DISPLAYOFF); }            //Turn off Display

void oledPrintNew(MicroOLED &named, String message){                    //Print new page
  named.clear(PAGE);
  named.setCursor(0, 0);
  named.print(message);
  named.display();
}

void oledPrintAdd(MicroOLED &named, String message){                    //Add to page
  named.print(message);
  named.display();
}

void oledUpdate(){                                                      //Update screen
  if (recovery&&!finalMessage[1]){                                      //Recovery screen
    oledOn(oled);
    oled.setFontType(0);
    oledPrintNew(oled, "If Found  Call:JamesFlaten    (651).399.2423");
    finalMessage[1] = true;
    //ADD REASON FOR CUT  
  } 
  else if ((GPS.getAlt_feet()>2000)&&!finalMessage[0]) {              //Turn off screen in flight
    oledPrintNew(oled, "");
    oledOff(oled);
    finalMessage[0] = true; 
  }
  else if(millis()-screenUpdateTimer>=SCREEN_UPDATE_RATE){            //Initialization screen
     String localDataPrint = "";
     
     screenUpdateTimer = millis();

    if (screen == 0) {                                                 //There are two screens that this system will cycle through    
      localDataPrint += "GPSALT:";                                     //Print GPS satellite count
      localDataPrint += String(GPS.getAlt_feet(),7);
      localDataPrint += "SAT: ";
      if (GPS.getSats()<10) {
        localDataPrint += '0' + String(GPS.getSats()) + " ";
      } else {
        localDataPrint += String(GPS.getSats()) + " ";
      }
       
     screen++; 
     oledPrintNew(oled, localDataPrint);
     
    } 
    else if (screen == 1) {
      if ((millis() - oledTime) < STATE_LOG_TIMER) {                                       //Indicate proper logging
        localDataPrint += "GoodLog";
      } else {
        localDataPrint += "BadLog!";
       }
            
      localDataPrint += String(flightMinutes(),7);      //Master timer

      localDataPrint += String(t2,7);                   //Internal temp
         
      screen++;
      oledPrintNew(oled, localDataPrint);  
    }
    else if (screen == 2) {
      if (SmartLogA != "") {                                            //Smart statuses
        localDataPrint += "SMRTA=1";
      } else {
        localDataPrint += "SMRTA=0";
      }
      if (SmartLogB != "") {
        localDataPrint += "SMRTB=1";
      } else {
        localDataPrint += "SMRTB=0";
      }

      if (PlanA.getLogQuality()){                                       //OPC Statuses
        localDataPrint += "P1";
      } else {
        localDataPrint += "P0";
      }

      if (SPSA.getLogQuality()){
        localDataPrint+= "S1";
      } else {
        localDataPrint += "S0";
      }

      if (R1A.getLogQuality()){
        localDataPrint+= "R1";
      } else {
        localDataPrint += "R0";
      }   
         
       screen = 0;
       oledPrintNew(oled, localDataPrint);  
    }
    else {
       oledPrintNew(oled, "Error");
       screen = 0;
    }
  }
}
