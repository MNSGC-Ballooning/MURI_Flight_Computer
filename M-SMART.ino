void SMARTControl() { 
   if (ChangeData){
      SmartLog="";
      SOCO.RequestTemp(1);
      smartTimer=millis();
      while(millis()-smartTimer<150 && SmartLog == "")
      {
        SmartLog=SOCO.Response();
      }

      ChangeData=false;
   }
    
}
