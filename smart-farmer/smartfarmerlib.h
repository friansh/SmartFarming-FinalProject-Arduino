/*
 *  -----------------------------------------------
 * |          Smart Farming Main Library           |
 *  -----------------------------------------------
 * | Bismillahhirrahmannirrahhiim.                 |
 * |                                               |
 * | All credits reserved to Fikri Rida P with     |
 * | student number 140910170023 :)                |
 *  -----------------------------------------------
 *  
 */

#ifdef INTERFACE_LCD_16x2
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif

#ifdef DATA_VIA_HTTP
#include <SPI.h>
#include <EthernetENC.h>
#endif

#include "configuration.h"
#include "helpers.h"

#include <ArduinoJson.h>

#ifdef SAVE_CONFIG_EEPROM
#include "sf-eeprom.h"
SmartFarmerEEPROM sf_eeprom;
#endif

#include "sf-sensors.h"
SmartFarmerSensors sf_sensors;

#include "sf-actuators.h"
SmartFarmerActuators sf_actuators;

#include "sf-tasks.h"
SmartFarmerTasks sf_tasks;

#ifdef DATA_VIA_HTTP
#include "sf-http.h"
SmartFarmerHttp sf_http;
#endif

class SmartFarmer {
  public:
    void begin() {
      Serial.println(F("[INFO] Program started..."));

#ifdef INTERFACE_LCD_16x2
      lcd.begin();
#endif
      lcdPrint("Device", "running...");

      sf_sensors.begin();
      sf_actuators.begin();

#ifdef DATA_VIA_HTTP
      sf_http.begin();
#endif

#ifdef DATA_VIA_SERIAL
      sf_tasks.waitForSettings();
      Serial.println(F("[INFO] The smartfarmer Arduino sub-system is ready to execute the specified commands..."));
      Serial.println(F("[INFO] - 1: Command for Arduino to update the agroclimate parameter settings."));
      Serial.println(F("[INFO] - 2: Command for Arduino to send the actual agroclimate parameters."));
      Serial.println(F("[INFO] - 3: Command for Arduino to turn on or off the growth light."));
      Serial.println(F("[INFO] - 4: Command for Arduino to correct the agroclimate parameters."));
#endif

      randomSeed(micros());

      delay(1000);
    }

    void run() {
      sf_tasks.run();
      sf_actuators.run();
#ifdef DATA_VIA_HTTP
      sf_http.run();
#endif
    }
};
