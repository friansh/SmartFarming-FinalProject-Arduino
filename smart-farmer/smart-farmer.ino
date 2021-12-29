// Uncomment to activate feature(s)
#define SAVE_CONFIG_EEPROM
//#define DATA_VIA_SERIAL
#define DATA_VIA_HTTP


// Main program
#include "smartfarmerlib.h"

SmartFarmer sf;

void setup() {
  Serial.begin(115200);
  sf.begin();
}

void loop() {
  sf.run();
}
