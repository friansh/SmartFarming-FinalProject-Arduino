/*
 *  -----------------------------------------------
 * |              EEPROM Sub-library               |
 *  -----------------------------------------------
 */
 
#include <EEPROM.h>

class SmartFarmerEEPROM {
  public:
    void savePh(float pHConfig) {
      EEPROM.put(pHConfigAddress, pHConfig);
      Serial.print("[EPRM] New pH config: ");
      Serial.print(pHConfig);
      Serial.println(" has been saved!");
    }

    void saveNutrientFlow(float nutrientFlowConfig) {
      EEPROM.put(nutrientFlowConfigAddress, nutrientFlowConfig);
      Serial.print("[EPRM] New nutrient flow config: ");
      Serial.print(nutrientFlowConfig);
      Serial.println(" has been saved!");
    }

    void saveLightIntensity(float lightIntensityConfig) {
      EEPROM.put(lightIntensityConfigAddress, lightIntensityConfig);
      Serial.print("[EPRM] New light intensity config: ");
      Serial.print(lightIntensityConfig);
      Serial.println(" has been saved!");
    }

    float loadPhConfig() {
      float pH = 0;
      EEPROM.get( pHConfigAddress, pH );
      return pH;
    }

    float loadNutrientFlowConfig() {
      float nutrientFlow = 0;
      EEPROM.get( nutrientFlowConfigAddress, nutrientFlow );
      return nutrientFlow;
    }

    float loadLightIntensityConfig() {
      float lightIntensity = 0;
      EEPROM.get( lightIntensityConfigAddress, lightIntensity );
      return lightIntensity;
    }

  private:
    int pHConfigAddress             = 0;
    int nutrientFlowConfigAddress   = 1 * sizeof(float);
    int lightIntensityConfigAddress = 2 * sizeof(float);
};
