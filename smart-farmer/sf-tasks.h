class SmartFarmerTasks {
  public:
    void run() {
      if ( Serial.available() > 0 ) {
        String receivedSerialData = Serial.readString();
        StaticJsonDocument<JSON_OBJECT_SIZE(11)> receivedData;
        DeserializationError err = deserializeJson(receivedData, receivedSerialData);
        if ( err ) {
          Serial.print(F("[WARN] JSON parser error: "));
          Serial.println(err.c_str());
        } else {
          if (!receivedData.containsKey("action")) return;
          const unsigned int receivedAction = receivedData["action"];
          doTaskSchedule[receivedAction] = true;
        }
      }

      currentMillis = millis();

      // Every 120 milliseconds,read the analog value from the ADC
      if (currentMillis - analogSampleTimepoint > 120U) {
        analogSampleTimepoint = millis();
        // Read the analog value and store into the buffer
        tdsAnalogBuffer[analogBufferIndex] = analogRead(TDS_INSTRUMENT_PIN);
        phAnalogBuffer[analogBufferIndex] = analogRead(PH_INSTRUMENT_PIN);

        analogBufferIndex++;
        if (analogBufferIndex == SCOUNT)
          analogBufferIndex = 0;
      }

      if (millis() - printTimepoint > 5000U) {
        printTimepoint = millis();
        sf_sensors.setTDS();
        sf_sensors.setPH();
      }

      task(0, [] {
        sf_sensors.setFlow(&flowCount, 0.5);
      });

      //      task(1, [](){
      //         Serial.println("[INFO] Subroutine 1 called: updating the agriclimate parameter settings.");
      //      });
      //      task(2, []() {
      //        Serial.println("[INFO] Subroutine 2 called: sending the actual agroclimate parameters via serial comm.");
      //        SmartFarmerTasks::sendAgroclimateData();
      //      });
      //      task(3, [](){ Serial.println("[INFO] Subroutine 3 called: turning on or off the growth light"); });
      //      task(4, []() {
      //        Serial.println("[INFO] Subroutine 4 called: correcting the agriclimate parameters.");
      //      });


    }

    void waitForSettingsFromSerial() {
      unsigned long lastConfigurationRequest = 0;
      Serial.println(F("[COMD] Please send the settings!"));
      while (true) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastConfigurationRequest > 10000UL) {
          Serial.println(F("[COMD] Please send the settings!"));
          lastConfigurationRequest = currentMillis;
        }
        if (Serial.available() > 0) {
          String receivedSerialData = Serial.readString();
          StaticJsonDocument<JSON_OBJECT_SIZE(10)> receivedSettings;
          DeserializationError err = deserializeJson(receivedSettings, receivedSerialData);
          if ( err ) {
            Serial.print(F("[WARN] JSON parser error: "));
            Serial.println(err.c_str());
            Serial.print("[INFO] Received JSON: ");
            Serial.println(receivedSerialData);
            delay(1000);
          } else {
            if ( receivedSettings.containsKey("ph") && receivedSettings.containsKey("light_intensity") && receivedSettings.containsKey("nutrient_flow") && receivedSettings.containsKey("growth_light") ) {
              float ph              = receivedSettings["ph"];
              float light_intensity = receivedSettings["light_intensity"];
              float nutrient_flow   = receivedSettings["nutrient_flow"];
              bool  growth_light    = receivedSettings["growth_light"];

              sf_actuators.setPh              ( ph               );
              sf_actuators.setLightIntensity  ( light_intensity  );
              sf_actuators.setNutrientFlow    ( nutrient_flow    );
              sf_actuators.setGrowthLight     ( growth_light     );

              Serial.println("[INFO] Received settings from the Raspberry Pi!");
              Serial.print  ("[INFO] - pH\t\t : ");
              Serial.println(ph);
              Serial.print  ("[INFO] - light intensity : ");
              Serial.println(light_intensity);
              Serial.print  ("[INFO] - nutrient flow\t : ");
              Serial.println(nutrient_flow);
              Serial.print  ("[INFO] - growth light\t : ");
              Serial.println(growth_light ? "true" : "false");
              break;
            }
          }
        }
      }
    }

  private:
    unsigned long   currentMillis           = 0;
    unsigned long   taskScheduleLastRun[6]  = {0, 0, 0, 0, 0, 0};
    unsigned long   taskScheduleInterval[6] = {2000, 1 * 1000, 1 * 1000, 5 * 60 * 1000, 1 * 1000, 40};
    bool            doTaskSchedule[6]       = {true, false, false, false, false, true};

    unsigned long   analogSampleTimepoint   = millis();
    unsigned long   printTimepoint          = millis();

    void task( uint8_t taskNum, void (*callback)() ) {
      if ( currentMillis - taskScheduleLastRun[taskNum] >= taskScheduleInterval[taskNum] ) {
        if ( doTaskSchedule[taskNum] ) {
          (*callback)();
          if (taskNum >= 1 && taskNum <= 4) doTaskSchedule[taskNum] = false;
        }
        taskScheduleLastRun[taskNum] = millis();
      }
    }

    static void sendAgroclimateDataToSerial() {
      StaticJsonDocument<JSON_OBJECT_SIZE(10)> actualAgroclimateData;

      actualAgroclimateData["temperature"]          = sf_sensors.getTemperature();
      actualAgroclimateData["humidity"]             = sf_sensors.getHumidity();
      actualAgroclimateData["ph"]                   = sf_sensors.getPH();
      actualAgroclimateData["light_intensity"]      = sf_sensors.getLightIntensity();
      actualAgroclimateData["nutrient_flow"]        = sf_sensors.getNutrientFlow();
      actualAgroclimateData["nutrient_level"]       = sf_sensors.getNutrientLevel();
      actualAgroclimateData["acid_solution_level"]  = sf_sensors.getAcidSolutionLevel();
      actualAgroclimateData["base_solution_level"]  = sf_sensors.getBaseSolutionLevel();
      actualAgroclimateData["tds"]                  = sf_sensors.getTDS();
      actualAgroclimateData["ec"]                   = sf_sensors.getEC();

      String JSONOutput = "[DATA] ";
      serializeJson(actualAgroclimateData, JSONOutput);
      Serial.println(JSONOutput);
    }
};
