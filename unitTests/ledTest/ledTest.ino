#define ledPin 21
#define ledPin 22
#define ledPin 23

void setup() {
  pinMode(21,OUTPUT);
  pinMode(22,OUTPUT);
  pinMode(23,OUTPUT);

}

void loop() {
  digitalWrite(21,HIGH);
  delay(100);
  digitalWrite(21,LOW);
  delay(1000);
  digitalWrite(22,HIGH);
  delay(100);
  digitalWrite(22,LOW);
  delay(1000);
  digitalWrite(23,HIGH);
  delay(100);
  digitalWrite(23,LOW);
  delay(1000);
}
