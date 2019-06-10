
//functions to manage the GPS

//time in seconds of last GPS update
unsigned long lastGPS = 0;
unsigned long GPSstartTime = 0;    //when the GPS starts
uint8_t days = 0;                  //if we're flying overnight (Ryan Bowers thinks of everying)

void updateGPS() 
{
  static bool firstFix = false;
  //GPS.update();
  while (Serial1.available() > 0) 
  {
    GPS.encode(Serial1.read());
  }
  if (GPS.altitude.isUpdated() || GPS.location.isUpdated()) 
  {
    if (!firstFix && GPS.Fix) //gps.fix 
    {     
      GPSstartTime = GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second();
      firstFix = true;
    }
  }
  if (getGPStime() > lastGPS) //if it's been more than a second
  {    
        lastGPS = GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second();
  }
}
   
int getGPStime() 
{
  return (GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second());
}

int getLastGPS() 
{ 
  //returns time in seconds between last successful fix and initial fix. Used to match with altitude data
  static bool newDay  = false;           //variable in case we're flying late at night (clock rollover)
  if (!newDay && lastGPS < GPSstartTime) 
  {
    days++;
    newDay = true;
  }
  else if (newDay && lastGPS > GPSstartTime)
    newDay = false;
  return days * 86400 + lastGPS;
}
