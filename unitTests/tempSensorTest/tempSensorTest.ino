
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 29
#define TWO_WIRE_BUS 30
#define THREE_WIRE_BUS 31
#define FOUR_WIRE_BUS 32
OneWire oneWire1(ONE_WIRE_BUS);
OneWire oneWire2(TWO_WIRE_BUS);
OneWire oneWire3(THREE_WIRE_BUS);
OneWire oneWire4(FOUR_WIRE_BUS);
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);
DallasTemperature sensor3(&oneWire3);
DallasTemperature sensor4(&oneWire4);
float t1;
float t2;
float t3;
float t4;



void setup() {
  sensor1.begin();
  sensor2.begin();
  sensor3.begin();
  sensor4.begin();

}

void loop() {
  sensor1.requestTemperatures();
  sensor2.requestTemperatures();
  sensor3.requestTemperatures();
  sensor4.requestTemperatures();
  t1 = sensor1.getTempC(0) + 273.15;
  t2 = sensor2.getTempC(0) + 273.15;
  t3 = sensor3.getTempC(0) + 273.15;
  t4 = sensor4.getTempC(0) + 273.15;
  Serial.println("t1=" + String(t1) +'\n' + "t2=" + String(t2) +'\n' + "t3=" + String(t3) + '\n' + "t4=" + String(t4) + '\n');

}
