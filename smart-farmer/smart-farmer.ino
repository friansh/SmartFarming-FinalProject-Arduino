#include "smartfarmerlib.h"

SmartFarmer sf;

void setup() {
  Serial.begin(115200);
  sf.begin();

  attachInterrupt(digitalPinToInterrupt(YF_S201_PIN), increaseFlowCount, FALLING);
}

void loop() {
  sf.run();
}
