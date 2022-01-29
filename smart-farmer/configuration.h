/*
 *  -----------------------------------------------
 * |            Arduino Pin Allocation             |
 *  -----------------------------------------------
 */

#define YF_S201_PIN                   2
#define GROWTH_LIGHT_PIN              5
#define TDS_INSTRUMENT_PIN            A2
#define PH_INSTRUMENT_PIN             A3

// Pumps pins
#define NUTRIENT_PUMP_PWM_PIN         13
#define BASE_SOL_PUMP_EN_PIN          37
#define ACID_SOL_PUMP_EN_PIN          35
#define BASE_SOL_PUMP_PWM_PIN         4
#define ACID_SOL_PUMP_PWM_PIN         3

// Temperature and Humidity Sensor Pin
#ifdef SENSORS_HUMIDTEMP
#define DHT22_PIN                     
#endif

// Ultrasonics pins
#ifdef SENSORS_ULTRASONIC
#define ACID_SOL_ULT_ECHO_PIN         
#define ACID_SOL_ULT_TRIG_PIN         
#define BASE_SOL_ULT_ECHO_PIN         
#define BASE_SOL_ULT_TRIG_PIN         
#define NUTRIENT_SOL_ULT_TRIG_PIN     
#define NUTRIENT_SOL_ULT_ECHO_PIN     
#endif

/*
 *  -----------------------------------------------
 * |             Global Configurations             |
 *  -----------------------------------------------
 */
 
// Ultrasonic config(s)
#ifdef SENSORS_ULTRASONIC
#define MAX_DISTANCE 200
#endif

// Ethernet config(s)
#ifdef DATA_VIA_HTTP
#define STATIC_IP   192, 168, 1, 5
#define HTTP_PORT   80

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
#endif

//I2C address
#define GROWTH_LIGHT_IN_ADDR      0x49
#define GROWTH_LIGHT_OUT_ADDR     0x39 

// Pump config(s)
#define NUTRIENT_FLOW_CORR_INTERVAL   5000
#define PH_CORR_INTERVAL              5000
#define ACID_PUMP_PWM_SPEED           115    // Approx. 45% of 255
#define BASE_PUMP_PWM_SPEED           127    // Approx. 50% of 255


// Light intensity sensor and growth light config(s)
#define LIGHT_INTENSITY_CHECK_INTERVAL  5000
#define LIGHT_INTENSITY_SENSOR_INSIDE_INTEGRATION_TIME      TSL2561_INTEGRATIONTIME_101MS
#define LIGHT_INTENSITY_SENSOR_OUTSIDE_INTEGRATION_TIME     TSL2561_INTEGRATIONTIME_101MS

#define SENSOR_GATHERING_INTERVAL 10U
#define SENSOR_SAMPLES 100
