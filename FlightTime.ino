//returns milliseconds since last flight start command (or bootup)
unsigned long flightTime() {
  return millis();
}

//returns the above flight time as a usable string for print statements
String flightTimeStr() {
  unsigned long t = flightTime() / 1000;
  String fTime = "";
  fTime += (String(t / 3600) + ":");
  t %= 3600;
  fTime += String(t / 600);
  t %= 600;
  fTime += (String(t / 60) + ":");
  t %= 60;
  fTime += (String(t / 10) + String(t % 10));
  return fTime;
}



