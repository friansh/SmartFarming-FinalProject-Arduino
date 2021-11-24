/*
 * Agroclimate data:
 * 1. temperature (C)
 * 2. humidity (RH)
 * 3. pH
 * 4. light intensity (lux)
 * 5. nutrient flow (m^3/min)
 * 6. nutrient level (m^3)
 * 7. acid solution level (m^3)
 * 8. base solution level (m^3)
 * 9. total dissolved solid (ppm)
 * 10. electrical conductivity (dS/m)
 * 
 * Controller parameters:
 * 1. pH
 * 2. light intensity (lux)
 * 3. nutrient flow (m^3/min)
 * 
 */
 
#include "configuration.h"
#include <ArduinoJson.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include "sf-sensors.h"
SmartFarmerSensors sf_sensors;

#include "sf-actuators.h"
SmartFarmerActuators sf_actuators;

#include "sf-tasks.h"
SmartFarmerTasks sf_tasks; 

class SmartFarmer {    
  public:
    void begin() {
      Serial.println(F("[INFO] Program started..."));
    
      lcd.begin();
      sf_sensors.begin();
      sf_actuators.begin();
      
//      sf_tasks.waitForSettings();
      
      Serial.println(F("[INFO] The smartfarmer Arduino sub-system is ready to execute the specified commands..."));
      Serial.println(F("[INFO] - 1: Command for Arduino to update the agroclimate parameter settings."));
      Serial.println(F("[INFO] - 2: Command for Arduino to send the actual agroclimate parameters."));
      Serial.println(F("[INFO] - 3: Command for Arduino to turn on or off the growth light."));
      Serial.println(F("[INFO] - 4: Command for Arduino to correct the agroclimate parameters."));

      randomSeed(micros());
      
      delay(2000);
    }

    void run() {
      sf_tasks.run();
      sf_actuators.run();
    }
    
//    void lcdPrint(String messageLine1, String messageLine2) {
//      int delta = 16 - messageLine1.length();
//      for ( int i=0; i < delta; i++ ) {
//        messageLine1 += " ";
//      }
//    
//      delta = 16 - messageLine2.length();
//      for ( int i=0; i < delta; i++ ) {
//        messageLine2 += " ";
//      }
//      
//      lcd.home();
//      lcd.print(messageLine1);
//      lcd.setCursor(0,1);
//      lcd.print(messageLine2);
//    }
    

};
