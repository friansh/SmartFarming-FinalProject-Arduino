/*
 *  -----------------------------------------------
 * |               Sensor Sub-library              |
 *  -----------------------------------------------
 * |  Agroclimate data (if all enabled):           |
 * |    1. temperature (C)                         |
 * |    2. humidity (RH)                           |
 * |    3. pH                                      |
 * |    4. light intensity (lux)                   |
 * |    5. nutrient flow (m^3/min)                 |
 * |    6. nutrient level (m^3)                    |
 * |    7. acid solution level (m^3)               |
 * |    8. base solution level (m^3)               |
 * |    9. total dissolved solid (ppm)             |
 * |    10. electrical conductivity (dS/m)         |
 *  -----------------------------------------------
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

#ifdef SENSORS_HUMIDTEMP
#include <DHT.h>
#include <DHT_U.h>
#endif

#ifdef SENSORS_ULTRASONIC
#include <NewPing.h>
#endif

class SmartFarmerSensors {
  public:
    void begin() {
#ifdef SENSORS_HUMIDTEMP
      pinMode(DHT22_PIN,           INPUT);
#endif
      pinMode(TDS_INSTRUMENT_PIN,  INPUT);
      pinMode(PH_INSTRUMENT_PIN,   INPUT);
      pinMode(YF_S201_PIN,         INPUT);

      pinMode(I2C_MULTIPLEXER_SELECTOR_S0,    OUTPUT);
      pinMode(I2C_MULTIPLEXER_SELECTOR_S1,    OUTPUT);
      pinMode(I2C_MULTIPLEXER_SELECTOR_S2,    OUTPUT);

      attachInterrupt(digitalPinToInterrupt(YF_S201_PIN), increaseFlowCount, FALLING);

#ifdef SENSORS_HUMIDTEMP
      dht                         = new DHT_Unified(DHT22_PIN, DHT22);
#endif

#ifdef SENSORS_ULTRASONIC
      nutrientLevelUltrasonic     = new NewPing(NUTRIENT_SOL_ULT_TRIG_PIN, NUTRIENT_SOL_ULT_ECHO_PIN, MAX_DISTANCE);
      acidSolutionLevelUltrasonic = new NewPing(ACID_SOL_ULT_TRIG_PIN, ACID_SOL_ULT_ECHO_PIN, MAX_DISTANCE);
      baseSolutionLevelUltrasonic = new NewPing(BASE_SOL_ULT_TRIG_PIN, BASE_SOL_ULT_ECHO_PIN, MAX_DISTANCE);
#endif

      // Get light intensity sensor 0 ( L L L - 0x70 )
      digitalWrite(I2C_MULTIPLEXER_SELECTOR_S0, LOW);
      digitalWrite(I2C_MULTIPLEXER_SELECTOR_S1, LOW);
      digitalWrite(I2C_MULTIPLEXER_SELECTOR_S2, LOW);
      
      lightIntensitySensor        = new Adafruit_TSL2561_Unified(GROWTH_LIGHT_IN_ADDR, 12345);

      if (!lightIntensitySensor->begin())
        Serial.println("[SNCK] no TSL2561 detected");
      else {
        /* You can also manually set the gain or enable auto-gain support */
        // lightIntensitySensor->setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
        // lightIntensitySensor->setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
        lightIntensitySensor->enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

        /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
        // lightIntensitySensor->setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
        // lightIntensitySensor->setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
        lightIntensitySensor->setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

        gatherSensorsInfo();
      }

      checkSensors();

#ifdef SENSORS_HUMIDTEMP
      dht->begin();
#endif
    }

#ifdef SENSORS_HUMIDTEMP
    float getTemperature() {
      dht->temperature().getEvent(&sensorsEvent);
      if (isnan(sensorsEvent.temperature)) return -1;
      else return sensorsEvent.temperature;
    }

    float getHumidity() {
      dht->humidity().getEvent(&sensorsEvent);
      if (isnan(sensorsEvent.relative_humidity)) return -1;
      else return sensorsEvent.relative_humidity;
    }
#endif

    float getPH() {
      return ph - 1.00;
    }

    float getLightIntensity() {
      // Get sensor 0 ( L L L - 0x70 )
      digitalWrite(I2C_MULTIPLEXER_SELECTOR_S0, LOW);
      digitalWrite(I2C_MULTIPLEXER_SELECTOR_S1, LOW);
      digitalWrite(I2C_MULTIPLEXER_SELECTOR_S2, LOW);
      
      lightIntensitySensor->getEvent(&sensorsEvent);

      if (sensorsEvent.light <= 65536) return sensorsEvent.light;
      else return -1;
    }

    float getOutLightIntensity() {
      // Get sensor 1 ( L L H - 0x71 )
      digitalWrite(I2C_MULTIPLEXER_SELECTOR_S0, HIGH);
      digitalWrite(I2C_MULTIPLEXER_SELECTOR_S1, LOW);
      digitalWrite(I2C_MULTIPLEXER_SELECTOR_S2, LOW);
      return 0;
    }

    float getNutrientFlow() {
      return nutrientFlow;
    }

#ifdef SENSORS_ULTRASONIC
    float getNutrientLevel() {
      float h = nutrientLevelUltrasonic->ping_cm();
      const float L1 = 41.5 * 28.5;
      const float L2 = 39.0 * 26.0;
      return ((0.333333) * ( 23.5 - h ) * ( L1 + L2 + sqrt(L1 * L2) )) / 1000;
    }

    float getAcidSolutionLevel() {
      float h = acidSolutionLevelUltrasonic->ping_cm();
      //      return (acidSolutionContainerWidth * acidSolutionContainerLength * h);
      return (float) random(0, 4000);
    }

    float getBaseSolutionLevel() {
      float h = baseSolutionLevelUltrasonic->ping_cm();
      //      return (baseSolutionContainerWidth * baseSolutionContainerLength * h);
      return (float) random(0, 4000);
    }
#endif

    float getTDS() {
      return tds - 493.00;
    }

    float getEC() {
      return (double) ( tds - 493.00 ) * 2 / 1000;
    }

    void setTDS() {
      for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
        analogBufferTemp[copyIndex] = tdsAnalogBuffer[copyIndex];

      float avgAnalogBit = getMedianNum(analogBufferTemp, SCOUNT);

      tds = 99.35716843 * exp(0.011733914 * avgAnalogBit);
    }

    void setPH() {
      unsigned int phSum = 0;
      for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
        analogBufferTemp[copyIndex] = phAnalogBuffer[copyIndex];

      float avgAnalogBit = getMedianNum(analogBufferTemp, SCOUNT);
      ph = 2.854567346 * exp(0.002079133 * avgAnalogBit);
    }

    void setFlow(int *count, float factor) {
      nutrientFlow = (*count) / 7.5 * factor;
      *count = 0;
    }

  private:
#ifdef SENSORS_HUMIDTEMP
    DHT_Unified *dht;
#endif

#ifdef SENSORS_ULTRASONIC
    NewPing *nutrientLevelUltrasonic;
    NewPing *acidSolutionLevelUltrasonic;
    NewPing *baseSolutionLevelUltrasonic;
#endif

    Adafruit_TSL2561_Unified *lightIntensitySensor;

    sensors_event_t sensorsEvent;

#ifdef SENSORS_ULTRASONIC
    const unsigned int nutrientContainerWidth       = 10;
    const unsigned int nutrientContainerLength      = 20;
    const unsigned int baseSolutionContainerWidth   = 30;
    const unsigned int baseSolutionContainerLength  = 40;
    const unsigned int acidSolutionContainerWidth   = 50;
    const unsigned int acidSolutionContainerLength  = 60;
#endif

    float nutrientFlow  = 0.00;
    float ph            = 0.00;

    float tds = 0;

    void gatherSensorsInfo() {
#ifdef SENSORS_HUMIDTEMP
      sensor_t dhtSensor;
      dht->temperature().getSensor(&dhtSensor);
      Serial.println(F("[SNIF] Temperature Sensor"));
      Serial.print  (F("[SNIF] Sensor Type: ")); Serial.println(dhtSensor.name);
      Serial.print  (F("[SNIF] Driver Ver:  ")); Serial.println(dhtSensor.version);
      Serial.print  (F("[SNIF] Unique ID:   ")); Serial.println(dhtSensor.sensor_id);
      Serial.print  (F("[SNIF] Max Value:   ")); Serial.print(dhtSensor.max_value); Serial.println(F("°C"));
      Serial.print  (F("[SNIF] Min Value:   ")); Serial.print(dhtSensor.min_value); Serial.println(F("°C"));
      Serial.print  (F("[SNIF] Resolution:  ")); Serial.print(dhtSensor.resolution); Serial.println(F("°C"));
      Serial.println(F("[SNIF] ------------------------------------"));
      // Print humidity sensor details.
      dht->humidity().getSensor(&dhtSensor);
      Serial.println(F("[SNIF] Humidity Sensor"));
      Serial.print  (F("[SNIF] Sensor Type: ")); Serial.println(dhtSensor.name);
      Serial.print  (F("[SNIF] Driver Ver:  ")); Serial.println(dhtSensor.version);
      Serial.print  (F("[SNIF] Unique ID:   ")); Serial.println(dhtSensor.sensor_id);
      Serial.print  (F("[SNIF] Max Value:   ")); Serial.print(dhtSensor.max_value); Serial.println(F("%"));
      Serial.print  (F("[SNIF] Min Value:   ")); Serial.print(dhtSensor.min_value); Serial.println(F("%"));
      Serial.print  (F("[SNIF] Resolution:  ")); Serial.print(dhtSensor.resolution); Serial.println(F("%"));
      Serial.println(F("[SNIF] ------------------------------------"));
#endif

      sensor_t tslSensor;
      lightIntensitySensor->getSensor(&tslSensor);
      Serial.println(F("[SNIF] ------------------------------------"));
      Serial.println(F("[SNIF] Light Intensity Sensor"));
      Serial.print  (F("[SNIF] Sensor:       ")); Serial.println(tslSensor.name);
      Serial.print  (F("[SNIF] Driver Ver:   ")); Serial.println(tslSensor.version);
      Serial.print  (F("[SNIF] Unique ID:    ")); Serial.println(tslSensor.sensor_id);
      Serial.print  (F("[SNIF] Max Value:    ")); Serial.print(tslSensor.max_value); Serial.println(" lux");
      Serial.print  (F("[SNIF] Min Value:    ")); Serial.print(tslSensor.min_value); Serial.println(" lux");
      Serial.print  (F("[SNIF] Resolution:   ")); Serial.print(tslSensor.resolution); Serial.println(" lux");
      Serial.println(F("[SNIF] ------------------------------------"));
    }

    void checkSensors() {
      Serial.println(F("[SNCK] Checking sensors..."));

#ifdef SENSORS_HUMIDTEMP
      dht->temperature().getEvent(&sensorsEvent);
      if (isnan(sensorsEvent.temperature)) Serial.println(F("[SNCK] Temperature sensor (DHT22) ERROR!"));
      else Serial.println(F("[SNCK] Temperature sensor (DHT22) OK!"));

      dht->humidity().getEvent(&sensorsEvent);
      if (isnan(sensorsEvent.relative_humidity)) Serial.println(F("[SNCK] Humidity sensor (DHT22) ERROR!"));
      else Serial.println(F("[SNCK] Humidity sensor (DHT22) OK!"));
#endif

#ifdef SENSORS_ULTRASONIC
      float ntVol = nutrientLevelUltrasonic->ping_cm();

      if ( ntVol == 0 )
        Serial.println(F("[SNCK] Ultrasonic sensor (HC-SR04) ERROR!"));
      else
        Serial.println(F("[SNCK] Ultrasonic sensor (HC-SR04) OK!"));
#endif

      lightIntensitySensor->getEvent(&sensorsEvent);
      if (sensorsEvent.light >= 65536) Serial.println(F("[SNCK] Light intensity sensor (TSL2561) ERROR!"));
      else Serial.println(F("[SNCK] Light intensity sensor (TSL2561) OK!"));

      Serial.println(F("[SNCK] ------------------------------------"));
    }
};
