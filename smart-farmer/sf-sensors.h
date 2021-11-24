#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <DHT.h>
#include <DHT_U.h>
#include <NewPing.h>

class SmartFarmerSensors {
  public:
    void begin() {
      pinMode(DHT22_PIN,           INPUT);
      pinMode(TDS_INSTRUMENT_PIN,  INPUT);
      pinMode(PH_INSTRUMENT_PIN,   INPUT);
      pinMode(YF_S201_PIN,         INPUT);

      dht                         = new DHT_Unified(DHT22_PIN, DHT22);
      nutrientLevelUltrasonic     = new NewPing(NUTRIENT_SOL_ULT_TRIG_PIN, NUTRIENT_SOL_ULT_ECHO_PIN, MAX_DISTANCE);
      acidSolutionLevelUltrasonic = new NewPing(ACID_SOL_ULT_TRIG_PIN, ACID_SOL_ULT_ECHO_PIN, MAX_DISTANCE);
      baseSolutionLevelUltrasonic = new NewPing(BASE_SOL_ULT_TRIG_PIN, BASE_SOL_ULT_ECHO_PIN, MAX_DISTANCE);

      lightIntensitySensor        = new Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 0x39);

      if (!lightIntensitySensor->begin()) Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");

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
      delay(500);
  
      /* You can also manually set the gain or enable auto-gain support */
      // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
      // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
      lightIntensitySensor->enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
      
      /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
      lightIntensitySensor->setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
      // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
      // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
      
      dht->begin();

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

      Serial.println(F("[SNCK] Checking sensors..."));
      
      dht->temperature().getEvent(&sensorsEvent);
      if (isnan(sensorsEvent.temperature)) Serial.println(F("[SNCK] Temperature sensor (DHT22) ERROR!"));
      else Serial.println(F("[SNCK] Temperature sensor (DHT22) OK!"));

      dht->humidity().getEvent(&sensorsEvent);
      if (isnan(sensorsEvent.relative_humidity)) Serial.println(F("[SNCK] Humidity sensor (DHT22) ERROR!"));
      else Serial.println(F("[SNCK] Humidity sensor (DHT22) OK!"));

      lightIntensitySensor->getEvent(&sensorsEvent);
      if (sensorsEvent.light >= 65536) Serial.println(F("[SNCK] Light intensity sensor (TSL2561) ERROR!"));
      else Serial.println(F("[SNCK] Light intensity sensor (TSL2561) OK!"));

      float ntVol = nutrientLevelUltrasonic->ping_cm();
      
      if ( ntVol == 0 )
        Serial.println(F("[SNCK] Ultrasonic sensor (HC-SR04) ERROR!"));
      else
        Serial.println(F("[SNCK] Ultrasonic sensor (HC-SR04) OK!"));
      
      Serial.println(F("[SNCK] ------------------------------------"));
    }

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

    float getPH() {
      return (float) ph;
      // y = m*x + c;
      //  when:
      //  y = calculated pH
      //  x = analog read from ph instrument pin

      //      float m = 1;
      //      float c = 5;
      //      float calculatedPh = ( m * analogRead(PH_INSTRUMENT_PIN) ) + c;
      //      return calculatedPh;
      //      return (float) random(10,70) / 10;

//      int samples = 100;
//      float adc_resolution = 1024.0;
//      int measurings = 0;

//      for (int i = 0; i < samples; i++) {
//        measurings += analogRead(PH_INSTRUMENT_PIN);
//        delay(10);
//      }

//      float voltage = 5 / adc_resolution * measurings / samples;
//      return analogRead(PH_INSTRUMENT_PIN) / 1023.0 * 5.0 ;
//      return (double) 7 + ((2.5 - ( analogRead(PH_INSTRUMENT_PIN) / 1023.0 * 5.0 ) ) / 0.18);
//      return 7 + ((2.5 - voltage) / 0.18);
    }

    float getLightIntensity() {
      lightIntensitySensor->getEvent(&sensorsEvent);

      if (sensorsEvent.light <= 65536) return sensorsEvent.light;
      else return -1;
    }

    float getNutrientFlow() {
      return nutrientFlow;
    }

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

    float getTDS() {
      return (float) tds;
    }

    float getEC() {
      return (double) tds * 2 / 1000;
    }

    void setTDS(){
      for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
        analogBufferTemp[copyIndex] = tdsAnalogBuffer[copyIndex];
        
      // Read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
      // Temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
      // Temperature compensation
      float compensationVolatge = averageVoltage / compensationCoefficient;
      // Convert voltage value to tds value
      tds = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5;
//      Serial.println(tds);
//      return tds;       
    }

    void setPH(){
      unsigned int phSum = 0;
      for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
        phSum += analogBufferTemp[copyIndex];

//      averageVoltage = phSum / SCOUNT * (float) VREF / 1024.0;
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * VREF / 1024.0;
      ph = averageVoltage;
//      ph = 7.0 + ((2.5 - averageVoltage) / 0.18);
    }
    
    void setFlow(int *count, float factor) {
      nutrientFlow = (*count) / 7.5 * factor;
      *count = 0;
    }

  private:
    DHT_Unified *dht;
    NewPing *nutrientLevelUltrasonic;
    NewPing *acidSolutionLevelUltrasonic;
    NewPing *baseSolutionLevelUltrasonic;
    Adafruit_TSL2561_Unified *lightIntensitySensor;

    sensors_event_t sensorsEvent;
    
    const unsigned int nutrientContainerWidth       = 10;
    const unsigned int nutrientContainerLength      = 20;
    const unsigned int baseSolutionContainerWidth   = 30;
    const unsigned int baseSolutionContainerLength  = 40;
    const unsigned int acidSolutionContainerWidth   = 50;
    const unsigned int acidSolutionContainerLength  = 60;

    float nutrientFlow  = 0.00;
    float ph            = 0.00;
        
    unsigned int tds = 0;

};
