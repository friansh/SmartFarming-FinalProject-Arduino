class SmartFarmerActuators {
  public:
    void begin() {
      pinMode( GROWTH_LIGHT_PIN,      OUTPUT );
      pinMode( NUTRIENT_PUMP_PWM_PIN,     OUTPUT );
      pinMode( BASE_SOL_PUMP_EN_PIN,      OUTPUT );
      pinMode( ACID_SOL_PUMP_EN_PIN,      OUTPUT );
      pinMode( BASE_SOL_PUMP_PWM_PIN,     OUTPUT );
      pinMode( ACID_SOL_PUMP_PWM_PIN,     OUTPUT );
    }

    void run() {
        currentMillis = millis();
        if ( currentMillis - lastNFCorrect >= 5000) {
          float currentNutrientFlow = sf_sensors.getNutrientFlow();
          Serial.print("[INFO] Current nutrient flow: ");
          Serial.print(currentNutrientFlow);
          Serial.print(" nutrient flow threshold is > ");
          Serial.print(nutrientFlowSetting * 0.9);
          Serial.print(" and < ");
          Serial.println(nutrientFlowSetting * 1.1);
          
          if ((currentNutrientFlow > (nutrientFlowSetting * 1.1)) || (currentNutrientFlow < ( nutrientFlowSetting * 0.9 ))){
            if (currentNutrientFlow < (nutrientFlowSetting * 1.1)) {
              if (nutrientFlowPWMValue >= 255) {
                Serial.print("[CORR] Nutrient motor PWM reached maximum value! Decreasing to ");
                Serial.print(nutrientFlowPWMValue = 255);
                Serial.println("...");
              } else {
                Serial.print("[CORR] Increasing nutrient motor PWM value by 5. Current PWM: ");
                Serial.println(nutrientFlowPWMValue += 5);
              }
            } else if (currentNutrientFlow > ( nutrientFlowSetting * 0.9 )) {
              if (nutrientFlowPWMValue <= 0) {
                Serial.print("[CORR] Nutrient motor PWM reached minimum value! Increasing to ");
                Serial.print(nutrientFlowPWMValue = 0);
                Serial.print("...");
              } else {
                Serial.print("[CORR] Decreasing nutrient motor PWM value by 5. Current PWM: ");
                Serial.println(nutrientFlowPWMValue -= 5);
              }
            }
          }
              
          lastNFCorrect = currentMillis;
        }
        
        setNutrientFlowAnalog(nutrientFlowPWMValue);
        digitalWrite(GROWTH_LIGHT_PIN, growthLightSetting);
    }
    
    bool setPh(float ph){
      if ( ph > 14 || ph < 1 ) {
        Serial.println("[CONF] Invalid pH value!");
        return false;
      }
      phSetting = ph;
      return true;
    }

    bool setLightIntensity(float lightIntensity){
      if ( lightIntensity > 20000 || lightIntensity < 0 ) {
        Serial.println("[CONF] Invalid light intensity value!");
        return false;
      }
      lightIntensitySetting = lightIntensity;
      return true;
    }

    bool setNutrientFlow(float nutrientFlow){
      if ( nutrientFlow > 10 || nutrientFlow < 0 ) {
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

    void correctPh() {
      while (true) {
        
      }      
    }

    void setNutrientFlowAnalog(byte val){
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
