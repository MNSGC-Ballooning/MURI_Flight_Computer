// GPS Fence code

static byte terminationN = 0; //  counter for being outside fence (north)
static byte terminationS = 0; //  counter for being outside fence (south)
static byte terminationE = 0; //  counter for being outside fence (east)
static byte terminationW = 0; //  counter for being outside fence (west) 

void GPS_Fence()
{
  ///// North Fence (47.0) /////
  if(Ublox.getFixAge < 4000 && Ublox.getLat() > termination_latitude_N && Ublox.getLat() != 0)
  {
    terminationN++; // add 1 to # of times outside mission area
    Serial.println("Termination Latitude check: " + String(terminationN));
    if (terminationN>5)
    {
      // SMART stuff here
      terminationN = 0;
    }
  }
  else
  {
    // if latitude is still in acceptable range, then reset check
    terminationN = 0;
  }
  
  ///// South Fence (43.0) /////
  if(Ublox.getFixAge < 4000 && Ublox.getLat() < termination_latitude_S && Ublox.getLat() != 0)
  {
    terminationS++; // add 1 to # of times outside mission area
    Serial.println("Termination Latitude check: " + String(terminationS));
    if (terminationS>5)
    {
      // SMART stuff here
      terminationS = 0;
    }
  }
  else
  {
    // if latitude is still in acceptable range, then reset check
    terminationS = 0;
  }

  ///// East Fence (-92.0) /////
  if(Ublox.getFixAge < 4000 && Ublox.getLon() > termination_longitude_E && Ublox.getLon() != 0)
  {
    terminationE++; // add 1 to # of times outside mission area
    Serial.println("Termination Longitude check: " + String(terminationE));
    if (terminationE>5)
    {
      // SMART stuff here
      terminationE = 0;
    }
  }
  else
  {
    // if longitude is still in acceptable range, then reset check
    terminationE = 0;
  }

  ///// West Fence (-98.0) /////
  if(Ublox.getFixAge < 4000 && Ublox.getLon() < termination_longitude_W && Ublox.getLon() != 0)
  {
    terminationE++; // add 1 to # of times outside mission area
    Serial.println("Termination Longitude check: " + String(terminationW));
    if (terminationW>5)
    {
      // SMART stuff here
      terminationW = 0;
    }
  }
  else
  {
    // if longitude is still in acceptable range, then reset check
    terminationW = 0;
  }


}
