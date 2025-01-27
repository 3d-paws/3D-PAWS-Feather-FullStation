/*
 * ======================================================================================================================
 *  SF.h - Support Functions
 * ======================================================================================================================
 */

// Prototyping functions to aviod compile function unknown issue.
void Output(const char *str);


/* 
 *=======================================================================================================================
 * I2C_Device_Exist - does i2c device exist at address
 * 
 *  The i2c_scanner uses the return value of the Write.endTransmisstion to see 
 *  if a device did acknowledge to the address.
 *=======================================================================================================================
 */
bool I2C_Device_Exist(byte address) {
  byte error;

  Wire.begin();                     // Connect to I2C as Master (no addess is passed to signal being a slave)

  Wire.beginTransmission(address);  // Begin a transmission to the I2C slave device with the given address. 
                                    // Subsequently, queue bytes for transmission with the write() function 
                                    // and transmit them by calling endTransmission(). 

  error = Wire.endTransmission();   // Ends a transmission to a slave device that was begun by beginTransmission() 
                                    // and transmits the bytes that were queued by write()
                                    // By default, endTransmission() sends a stop message after transmission, 
                                    // releasing the I2C bus.

  // endTransmission() returns a byte, which indicates the status of the transmission
  //  0:success
  //  1:data too long to fit in transmit buffer
  //  2:received NACK on transmit of address
  //  3:received NACK on transmit of data
  //  4:other error 

  // Partice Library Return values
  // SEE https://docs.particle.io/cards/firmware/wire-i2c/endtransmission/
  // 0: success
  // 1: busy timeout upon entering endTransmission()
  // 2: START bit generation timeout
  // 3: end of address transmission timeout
  // 4: data byte transfer timeout
  // 5: data byte transfer succeeded, busy timeout immediately after
  // 6: timeout waiting for peripheral to clear stop bit

  if (error == 0) {
    return (true);
  }
  else {
    // sprintf (msgbuf, "I2CERR: %d", error);
    // Output (msgbuf);
    return (false);
  }
}

/*
 * ======================================================================================================================
 * Blink() - Count, delay between, delay at end
 * ======================================================================================================================
 */
void Blink(int count, int between)
{
  int c;

  for (c=0; c<count; c++) {
    digitalWrite(LED_PIN, HIGH);
    delay(between);
    digitalWrite(LED_PIN, LOW);
    delay(between);
  }
}

/*
 * ======================================================================================================================
 * FadeOn() - https://www.dfrobot.com/blog-596.html
 * ======================================================================================================================
 */
void FadeOn(unsigned int time,int increament){
  for (byte value = 0 ; value < 255; value+=increament){
  analogWrite(LED_PIN, value);
  delay(time/(255/5));
  }
}

/*
 * ======================================================================================================================
 * FadeOff() - 
 * ======================================================================================================================
 */
void FadeOff(unsigned int time,int decreament){
  for (byte value = 255; value >0; value-=decreament){
  analogWrite(LED_PIN, value);
  delay(time/(255/5));
  }
}

/*
 * ======================================================================================================================
 * myswap()
 * ======================================================================================================================
 */
void myswap(unsigned int *p, unsigned int *q) {
  int t;
   
  t=*p; 
  *p=*q; 
  *q=t;
}

/*
 * ======================================================================================================================
 * mysort()
 * ======================================================================================================================
 */
void mysort(unsigned int a[], unsigned int n) { 
  unsigned int i, j;

  for(i = 0;i < n-1;i++) {
    for(j = 0;j < n-i-1;j++) {
      if(a[j] > a[j+1])
        myswap(&a[j],&a[j+1]);
    }
  }
}

/*
 * =======================================================================================================================
 * isnumeric() - check if string contains all digits
 * =======================================================================================================================
 */
bool isnumeric(char *s) {
  for (int i=0; i< strlen(s); i++) {
    if (!isdigit(*(s+i)) ) {
      return(false);
    }
  }
  return(true);
}

/*
 * =======================================================================================================================
 * isValidHexString() - validates whether a given string is a valid hexadecimal string of a specified length
 * =======================================================================================================================
 */
bool isValidHexString(const char *hexString, size_t expectedLength) {
    size_t length = strlen(hexString);

    // Check if the length is as expected
    if (length != expectedLength) {
        sprintf(msgbuf, "Hexlen not %d", expectedLength);
        Output (msgbuf);
        return (false);
    }

    // Check if each character is a valid hexadecimal digit
    for (size_t i = 0; i < length; i++) {
        if (!isxdigit(hexString[i])) {
            sprintf(msgbuf, "!Hex char '%c' at pos %d", hexString[i], i);
            Output (msgbuf);
            return (false);
        }
    }

    return (true); // Valid hex string
}

/*
 * =======================================================================================================================
 * hexStringToUint32() - 
 * =======================================================================================================================
 */
bool hexStringToUint32(const char *hexString, uint32_t *result) {
    // Check if the hex string is of length 8
    if (strlen(hexString) != 8) {
        Output ("Error: Hex string must be of length 8");
        return false;
    }

    *result = 0;
    for (int i = 0; i < 8; i++) {
        char c = hexString[i];
        uint32_t digit;
    
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            digit = 10 + (c - 'A');
        } else {
            return false;
        }

        *result = (*result << 4) | digit;
    }
    return true; // Successful conversion
}

/*
 * =======================================================================================================================
 * hexStringToByteArray() -
 * =======================================================================================================================
 */
void hexStringToByteArray(const char *hexString, uint8_t *byteArray, int len) {
    for (int i = 0; i < len; i += 2) {
        char hexPair[3];
        hexPair[0] = hexString[i];
        hexPair[1] = hexString[i + 1];
        hexPair[2] = '\0';

        // Convert hex pair to uint8_t
        uint8_t byteValue = 0;
        for (int j = 0; j < 2; ++j) {
            byteValue <<= 4;

            if (hexPair[j] >= '0' && hexPair[j] <= '9') {
                byteValue |= hexPair[j] - '0';
            } else if (hexPair[j] >= 'A' && hexPair[j] <= 'F') {
                byteValue |= hexPair[j] - 'A' + 10;
            } else if (hexPair[j] >= 'a' && hexPair[j] <= 'f') {
                byteValue |= hexPair[j] - 'a' + 10;
            }
        }

        byteArray[i / 2] = byteValue;
    }
}

/*
 * ======================================================================================================================
 * JPO_ClearBits() - Clear System Status Bits related to initialization
 * ======================================================================================================================
 */
void JPO_ClearBits() {
  if (JustPoweredOn) {
    JustPoweredOn = false;
    SystemStatusBits &= ~SSB_PWRON;   // Turn Off Power On Bit
    SystemStatusBits &= ~SSB_OLED;    // Turn Off OLED Not Found Bit
    SystemStatusBits &= ~SSB_LORA;    // Turn Off LoRa Not Found Bit
    SystemStatusBits &= ~SSB_BMX_1;   // Turn Off BMX_1 Not Found Bit
    SystemStatusBits &= ~SSB_BMX_2;   // Turn Off BMX_2 Not Found Bit
    SystemStatusBits &= ~SSB_HTU21DF; // Turn Off HTU Not Found Bit
    SystemStatusBits &= ~SSB_MCP_1;   // Turn Off MCP_1 Not Found Bit
    SystemStatusBits &= ~SSB_MCP_2;   // Turn Off MCP_2 Not Found Bit
    SystemStatusBits &= ~SSB_SHT_1;   // Turn Off SHT_1 Not Found Bit
    SystemStatusBits &= ~SSB_SHT_2;   // Turn Off SHT_2 Not Found Bit
    SystemStatusBits &= ~SSB_HIH8;    // Turn Off HIH Not Found Bit
    SystemStatusBits &= ~SSB_SI1145;  // Turn Off UV,IR, VIS Not Found Bit
    SystemStatusBits &= ~SSB_PM25AQI; // Turn Off PM25AQI Not Found Bit
    SystemStatusBits &= ~SSB_AS5600;  // Turn Off AS5600 Not Found Bit (Wind Direction)
    SystemStatusBits &= ~SSB_TLW;     // Turn Off Tinovi Leaf Wetness Not Found Bit
    SystemStatusBits &= ~SSB_TSM;     // Turn Off Tinovi Soil Moisture Not Found Bit
    SystemStatusBits &= ~SSB_TMSM;    // Turn Off Tinovi MultiLevel Soil Moisture Not Found Bit
    
    // SystemStatusBits &= ~SSB_GPS;     // Turn Off GPS Not Found Bit - Required
    // SystemStatusBits &= ~SSB_EEPROM;  // Turn Off 24LC32 EEPROM Not Found Bit - Reqired
  }
}

/*
 * =======================================================================================================================
 *  Measuring Battery - SEE https://learn.adafruit.com/adafruit-feather-m0-adalogger/power-management
 *  
 *  This voltage will 'float' at 4.2V when no battery is plugged in, due to the lipoly charger output,  so its not a good 
 *  way to detect if a battery is plugged in or not (there is no simple way to detect if a battery is plugged in)
 * =======================================================================================================================
 */
#define VBATPIN      A7

/*
 *=======================================================================================================================
 * vbat_get() -- return battery voltage
 *=======================================================================================================================
 */
float vbat_get() {
  float v = analogRead(VBATPIN);
  v *= 2;    // we divided by 2, so multiply back
  v *= 3.3;  // Multiply by 3.3V, our reference voltage
  v /= 4096; // convert to voltage
  return (v);
}

/*
 * ======================================================================================================================
 * GetDeviceID() - 
 * 
 * This function reads the 128-bit (16-byte) unique serial number from the 
 * SAMD21 microcontroller's flash memory, compresses it to 12 bytes, and 
 * converts it to a 24-character hexadecimal string.
 * 
 * The compression is done by XORing the first 12 bytes with the last 12 bytes 
 * of the original 16-byte identifier, ensuring all 128 bits contribute to 
 * the final result.
 * 
 * SAM D21/DA1 Family Datas Sheet
 * 10.3.3 Serial Number
 * Each device has a unique 128-bit serial number which is a concatenation of four 32-bit words contained at the
 * following addresses:
 * Word 0: 0x0080A00C
 * Word 1: 0x0080A040
 * Word 2: 0x0080A044
 * Word 3: 0x0080A048
 * The uniqueness of the serial number is guaranteed only when using all 128 bits.
 * ======================================================================================================================
 */
void GetDeviceID() {
  // Pointer to the unique 128-bit serial number in flash
  uint32_t *uniqueID = (uint32_t *)0x0080A00C;
  
  // Temporary buffer to store 16 bytes (128 bits)
  uint8_t fullId[16];
  
  // Extract all 16 bytes from the 128-bit serial number
  for (int i = 0; i < 4; i++) {
    uint32_t val = uniqueID[i];
    fullId[i*4] = (val >> 24) & 0xFF;
    fullId[i*4 + 1] = (val >> 16) & 0xFF;
    fullId[i*4 + 2] = (val >> 8) & 0xFF;
    fullId[i*4 + 3] = val & 0xFF;
  }

  // Compress 16 bytes to 12 bytes using XOR
  uint8_t compressedId[12];
  for (int i = 0; i < 12; i++) {
    compressedId[i] = fullId[i] ^ fullId[i + 4];
  }

  memset (DeviceID, 0, 25);
  for (int i = 0; i < 12; i++) {
    sprintf (DeviceID+strlen(DeviceID), "%02x", compressedId[i]);
  }
  DeviceID[24] = '\0'; // Ensure null-termination
}
