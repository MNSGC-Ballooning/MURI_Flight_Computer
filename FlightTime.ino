String timeLeft(){
  int timeLeft = int((masterTimer-millis())/1000);
  String timeLeftStr = (String(timeLeft/60) + ":");
  timeLeft %= 60;
  timeLeftStr += (String(timeLeft / 10) + String(timeLeft % 10));
  return timeLeftStr;
}

