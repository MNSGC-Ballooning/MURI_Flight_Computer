// GPS   
int getGPStime() 
{
  return (GPS.getHour() * 3600 + GPS.getMinute() * 60 + GPS.getSecond());
}


void CheckFix() {
  //GPS should update once per second, if data is more than 4 seconds old, fix was likely lost
  if(GPS.getFixAge() > 4000){
    data += "No Fix,";
    GPSfix = false;
  }
  else{
    data += "Fix,";
    GPSfix = true;
  }
}


void GetGPSAltitude() {
  
  alt_GPS = GPS.getAlt_feet();                                // altitude calulated by the Ublox GPS
  
}


void UpdateGPS() {
  GPS.update();
}
