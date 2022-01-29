/*
 *  -----------------------------------------------
 * |          Libraries Helper Functions           |
 *  -----------------------------------------------
 */
 
int flowCount = 0;

// TDS Sensor Parameters
#define VREF    5.0              // analog reference voltage(Volt) of the ADC
//#define SCOUNT  30               // sum of sample point

unsigned int phAnalogBuffer[SENSOR_SAMPLES];
unsigned int tdsAnalogBuffer[SENSOR_SAMPLES];

//int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
//int copyIndex = 0;

void increaseFlowCount() {
  flowCount++;
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

#ifdef INTERFACE_LCD_16x2
void lcdPrint(String messageLine1, String messageLine2) {
  int delta = 16 - messageLine1.length();
  for ( int i = 0; i < delta; i++ ) {
    messageLine1 += " ";
  }

  delta = 16 - messageLine2.length();
  for ( int i = 0; i < delta; i++ ) {
    messageLine2 += " ";
  }

  lcd.home();
  lcd.print(messageLine1);
  lcd.setCursor(0, 1);
  lcd.print(messageLine2);
}
#else
void lcdPrint(String param1, String param2) {}
#endif

#ifdef DATA_VIA_HTTP
String IpAddress2String(IPAddress address) {
 return String(address[0]) + "." + 
        String(address[1]) + "." + 
        String(address[2]) + "." + 
        String(address[3]);
}
#endif
