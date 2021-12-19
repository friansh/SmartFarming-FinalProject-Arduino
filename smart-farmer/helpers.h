int flowCount = 0;

// TDS Sensor Parameters
#define VREF    5.0              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30               // sum of sample point

int tdsAnalogBuffer[SCOUNT];        // store the analog value in the array, read from ADC
int phAnalogBuffer[SCOUNT];        // store the analog value in the array, read from ADC

int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;

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

String IpAddress2String(IPAddress address) {
 return String(address[0]) + "." + 
        String(address[1]) + "." + 
        String(address[2]) + "." + 
        String(address[3]);
}
