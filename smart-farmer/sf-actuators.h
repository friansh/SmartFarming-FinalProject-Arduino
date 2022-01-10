/*
    -----------------------------------------------
   |             Actuator Sub-library              |
   -----------------------------------------------
   | Controlled agroclimate params:                |
   |   1. pH                                       |
   |   2. light intensity (lux)                    |
   |   3. nutrient flow (m^3/min)                  |
    -----------------------------------------------
*/

class SmartFarmerActuators {
  public:
    void begin() {
      pinMode( GROWTH_LIGHT_PIN,          OUTPUT );
      pinMode( NUTRIENT_PUMP_PWM_PIN,     OUTPUT );
      pinMode( BASE_SOL_PUMP_EN_PIN,      OUTPUT );
      pinMode( ACID_SOL_PUMP_EN_PIN,      OUTPUT );
      pinMode( BASE_SOL_PUMP_PWM_PIN,     OUTPUT );
      pinMode( ACID_SOL_PUMP_PWM_PIN,     OUTPUT );

#ifdef SAVE_CONFIG_EEPROM
      Serial.println("[EPRM] Loading saved config from EEPROM...");
      if ( setPh(sf_eeprom.loadPhConfig()) ) {
        Serial.print("[EPRM] - pH configuration: ");
        Serial.println(sf_eeprom.loadPhConfig());
      } else
        Serial.println("[EPRM] Failed to load pH configuration from EEPROM!");

      if ( setLightIntensity(sf_eeprom.loadLightIntensityConfig()) ) {
        Serial.print("[EPRM] - light intensity configuration: ");
        Serial.println(sf_eeprom.loadLightIntensityConfig());
      } else
        Serial.println("[EPRM] Failed to load light intensity configuration from EEPROM!");

      if ( setNutrientFlow(sf_eeprom.loadNutrientFlowConfig()) ) {
        Serial.print("[EPRM] - nutrient flow configuration: ");
        Serial.println(sf_eeprom.loadNutrientFlowConfig());
      } else
        Serial.println("[EPRM] Failed to load nutrient flow configuration from EEPROM!");
#endif
    }

    void run() {
      currentMillis = millis();
      if ( currentMillis - lastNFCorrect >= NUTRIENT_FLOW_CORR_INTERVAL ) {
        float currentNutrientFlow = sf_sensors.getNutrientFlow();
        Serial.print("[INFO] Current nutrient flow: ");
        Serial.print(currentNutrientFlow);
        Serial.print(" nutrient flow threshold is > ");
        Serial.print(nutrientFlowSetting * 0.9);
        Serial.print(" and < ");
        Serial.println(nutrientFlowSetting * 1.1);

        if (currentNutrientFlow < (nutrientFlowSetting * 0.9)) {    // Check if nutrient flow below threshold (90% of config)
          if (nutrientFlowPWMValue >= 255) {
            Serial.print("[CORR] Nutrient motor PWM reached maximum value! Decreasing to ");
            Serial.print(nutrientFlowPWMValue = 255);
            Serial.println("...");
            lcdPrint("PWM reached max", "value!");
          } else {
            Serial.print("[CORR] Increasing nutrient motor PWM value by 5. Current PWM: ");
            Serial.println(nutrientFlowPWMValue += 5);
            lcdPrint("Increasing PWM", "to " + String(nutrientFlowPWMValue));
          }
        } else if (currentNutrientFlow > ( nutrientFlowSetting * 1.1 )) {   // Check if nutrient flow above threshold (110% of config)
          if (nutrientFlowPWMValue <= 0) {
            Serial.print("[CORR] Nutrient motor PWM reached minimum value! Increasing to ");
            Serial.print(nutrientFlowPWMValue = 0);
            Serial.println("...");
            lcdPrint("PWM reached min", "value!");
          } else {
            Serial.print("[CORR] Decreasing nutrient motor PWM value by 5. Current PWM: ");
            Serial.println(nutrientFlowPWMValue -= 5);
            lcdPrint("Decreasing PWM", "to " + String(nutrientFlowPWMValue));
          }
        } else {    // Check if nutrient flow between threshold
          // Do nothing
        }

        lastNFCorrect = currentMillis;
      }

      if ( currentMillis - lastPhCorrect >= PH_CORR_INTERVAL ) {
        float currentPH = sf_sensors.getPH();
        Serial.print("[INFO] Current pH: ");
        Serial.print(currentPH);
        Serial.print(" pH threshold is > ");
        Serial.print(phSetting * 0.9);
        Serial.print(" and < ");
        Serial.println(phSetting * 1.1);

        if (currentPH > (phSetting * 1.1)) {    // Too base (pH above 90% of setpoint)
          // Turn on acid pump and turn off base pump
          Serial.println("[CORR] Nutrient is too base, correcting... ");
          digitalWrite(BASE_SOL_PUMP_EN_PIN, LOW);
          digitalWrite(ACID_SOL_PUMP_EN_PIN, HIGH);
          analogWrite(BASE_SOL_PUMP_PWM_PIN, 0);
          analogWrite(ACID_SOL_PUMP_PWM_PIN, ACID_PUMP_PWM_SPEED);
        } else if (currentPH < ( phSetting * 0.9 )) {   // Too acid (pH below 90% of setpoint)
          Serial.println("[CORR] Nutrient is too acid, correcting... ");
          // Turn on base pump and turn off acid pump
          digitalWrite(BASE_SOL_PUMP_EN_PIN, HIGH);
          digitalWrite(ACID_SOL_PUMP_EN_PIN, LOW);
          analogWrite(BASE_SOL_PUMP_PWM_PIN, BASE_PUMP_PWM_SPEED);
          analogWrite(ACID_SOL_PUMP_PWM_PIN, 0);
        } else {    // All correct
          // Turn off both acid and base pump
          digitalWrite(BASE_SOL_PUMP_EN_PIN, LOW);
          digitalWrite(ACID_SOL_PUMP_EN_PIN, LOW);
          analogWrite(BASE_SOL_PUMP_PWM_PIN, 0);
          analogWrite(ACID_SOL_PUMP_PWM_PIN, 0);
        }

        lastPhCorrect = currentMillis;
      }

      setNutrientFlowAnalog(nutrientFlowPWMValue);
      digitalWrite(GROWTH_LIGHT_PIN, !growthLightSetting);
    }

    bool setPh(float ph) {
      if ( ph > 14 || ph < 1 || isnan(ph) ) {
        Serial.println("[CONF] Invalid pH value!");
        return false;
      }
      phSetting = ph;
      return true;
    }

    bool setLightIntensity(float lightIntensity) {
      if ( lightIntensity > 20000 || lightIntensity < 0 || isnan(lightIntensity)) {
        Serial.println("[CONF] Invalid light intensity value!");
        return false;
      }
      lightIntensitySetting = lightIntensity;
      return true;
    }

    bool setNutrientFlow(float nutrientFlow) {
      if ( nutrientFlow > 10 || nutrientFlow < 0 || isnan(nutrientFlow)) {
        Serial.println("[CONF] Invalid nutrient flow value!");
        return false;
      }
      nutrientFlowSetting = nutrientFlow;
      Serial.print("[INFO] Setting nutrient flow to: ");
      Serial.print(nutrientFlowSetting);
      nutrientFlowPWMValue = 15.67610421 * pow(nutrientFlowSetting, 2) - 17.63590707 * nutrientFlowSetting + 121;
      Serial.print(" l/m with theoretical PWM: ");
      Serial.println(nutrientFlowPWMValue);
      return true;
    }

    void setGrowthLight(bool growthLight) {
      growthLightSetting = growthLight;
    }

  private:
    float   phSetting               = 0;
    float   lightIntensitySetting   = 0;
    float   nutrientFlowSetting     = 0;
    bool    growthLightSetting      = false;
    float   nutrientFlow            = 0;

    unsigned int growthLightPWMValue  = 0;
    int nutrientFlowPWMValue = 0;

    unsigned long currentMillis = 0;

    unsigned long lastNFCorrect = 0;
    unsigned long lastPhCorrect = 0;

    void correctPh() {
      while (true) {

      }
    }

    void setNutrientFlowAnalog(byte val) {
      if ( nutrientFlowPWMValue > 255 )
        analogWrite(NUTRIENT_PUMP_PWM_PIN, 255);
      else
        analogWrite(NUTRIENT_PUMP_PWM_PIN, val);
    }

    //    void correctLightIntensity() {
    //      if ( growthLightSetting )
    //        while ( getLightIntensity() < ( lightIntensitySetting * 0.9 ) or
    //                  getLightIntensity() > ( lightIntensitySetting * 1.1 ) ) {
    //
    //          if ( getLightIntensity() < ( lightIntensitySetting * 0.9 ) ) growthLightPWMValue += 5;
    //          if ( getLightIntensity() > ( lightIntensitySetting * 1.1 ) ) growthLightPWMValue -= 5;
    //
    //          analogWrite( GROWTH_LIGHT_PIN, growthLightPWMValue );
    //          delay(10);
    //        }
    //      else turnOffGrowthLight();
    //    }
    //
    //
};
