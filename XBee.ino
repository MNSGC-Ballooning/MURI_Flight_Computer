
void releaseSMART(int i) {
  xBee.send("CUT" + String(i));
}
