/*
 *  -----------------------------------------------
 * |              Features Activation              |
 *  -----------------------------------------------
 *  Please uncomment to activate feature(s)
 */
 
//#define SAVE_CONFIG_EEPROM
//#define DATA_VIA_SERIAL
#define DATA_VIA_HTTP
//#define SENSORS_ULTRASONIC
//#define SENSORS_HUMIDTEMP
//#define INTERFACE_LCD_16x2

#define AGROCLIMATE_CORRECTION_DEBUG

//#define WATER_FLOW_BENCHMARK
//#define PH_BENCHMARK
//#define GROWTH_LIGHT_BENCHMARK


/*
 *  -----------------------------------------------
 * |                  Main Program                 |
 *  -----------------------------------------------
 */

#include "smartfarmerlib.h"

SmartFarmer sf;

void setup() {
  Serial.begin(115200);
  sf.begin();
}

void loop() {
  sf.run();
}
