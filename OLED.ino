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
      
      if (GPS.getAlt_feet()>=1000) {
        localDataPrint += String(GPS.getAlt_feet(),2);
      }
      else if (GPS.getAlt_feet() > 100) {
        localDataPrint += String(GPS.getAlt_feet(),2) + " ";
      }
      else if (GPS.getAlt_feet() > 10) {
        localDataPrint += String(GPS.getAlt_feet(),2) + "  ";
      }
      else{
        localDataPrint += String(GPS.getAlt_feet(),2) + "   ";
      }
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
      if ((millis() - oledTime) < STATE_LOG_TIMER) {                      //Indicate proper logging
        localDataPrint += "GoodLog";
      } else {
        localDataPrint += "BadLog!";
       }

      if (t2<=-10) {
        localDataPrint += String(t2,2) + " ";
      }
      else if (t2>=10 || (-10<t2 && t2<0)) {
        localDataPrint += String(t2,2) + "  ";                            //Internal temp
      }
      else if (t2>=0 && t2<10) {
        localDataPrint += String(t2,2) + "   ";
      }
            
      localDataPrint += String(flightMinutes(),2);                        //Master timer

         
      screen++;
      oledPrintNew(oled, localDataPrint);  
    }
    
    else if (screen == 2) {
      if (SmartLogA != "") {                                              //Smart statuses
        localDataPrint += "SMRTA=1";
      } else {
        localDataPrint += "SMRTA=0";
      }
      if (SmartLogB != "") {
        localDataPrint += "SMRTB=1";
      } else {
        localDataPrint += "SMRTB=0";
      }

      screen++;
      oledPrintNew(oled, localDataPrint);
    }
    
    else if (screen == 3) {
      if (PlanA.getLogQuality()){                                       //OPC Statuses
        localDataPrint += "P1 ";
      } else {
        localDataPrint += "P0 ";
      } 
      if (SPSA.getLogQuality()){
        localDataPrint += "S1  ";
      } else {
        localDataPrint += "S0  ";
      }
      if (HPMA.getLogQuality()){
        localDataPrint+= "H1 ";
      } else {
        localDataPrint += "H0 ";
      }
      if (R1A.getLogQuality()){
        localDataPrint+= "R1  ";
      } else {
        localDataPrint += "R0  ";
      }   

    screen++;
    oledPrintNew(oled, localDataPrint);
    }
       screen = 0;
       oledPrintNew(oled, localDataPrint);  
    }
    else {
       oledPrintNew(oled, "Error");
       screen = 0;
    }
  }
