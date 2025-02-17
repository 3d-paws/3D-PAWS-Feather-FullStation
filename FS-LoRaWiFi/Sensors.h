/*
 * ======================================================================================================================
 *  Sensors.h
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  BMX280 humidity - I2C - Temperature, pressure sensor & altitude - Support 2 of any combination
 * 
 *  https://www.asknumbers.com/PressureConversion.aspx
 *  Pressure is returned in the SI units of Pascals. 100 Pascals = 1 hPa = 1 millibar. 
 *  Often times barometric pressure is reported in millibar or inches-mercury. 
 *  For future reference 1 pascal = 0.000295333727 inches of mercury, or 1 inch Hg = 3386.39 Pascal. 
 *
 *  Looks like you divide by 100 and you get millibars which matches NWS page
 * 
 *  Surface Observations and Station Elevation 
 *  https://forecast.weather.gov/product.php?issuedby=BOU&product=OSO&site=bou 
 * ======================================================================================================================
 */
#define BMX_STATION_ELEVATION 1017.272  // default 1013.25
#define BMX_ADDRESS_1         0x77        // BMP Default Address - Connecting SDO to GND will change BMP to 0x76
#define BMX_ADDRESS_2         0x76        // BME Default Address - Connecting SDO to GND will change BME to 0x77
#define BMP280_CHIP_ID        0x58
#define BME280_BMP390_CHIP_ID 0x60
#define BMP388_CHIP_ID        0x50
#define BMX_TYPE_UNKNOWN      0
#define BMX_TYPE_BMP280       1
#define BMX_TYPE_BME280       2
#define BMX_TYPE_BMP388       3
#define BMX_TYPE_BMP390       4
Adafruit_BMP280 bmp1;
Adafruit_BMP280 bmp2;
Adafruit_BME280 bme1;
Adafruit_BME280 bme2;
Adafruit_BMP3XX bm31;
Adafruit_BMP3XX bm32;
byte BMX_1_chip_id = 0x00;
byte BMX_2_chip_id = 0x00;
bool BMX_1_exists = false;
bool BMX_2_exists = false;
byte BMX_1_type=BMX_TYPE_UNKNOWN;
byte BMX_2_type=BMX_TYPE_UNKNOWN;

/*
 * ======================================================================================================================
 *  SHTX - I2C - Temperature & Humidity sensor (SHT31)  - Note the SHT40, SHT45 use same i2c address
 * ======================================================================================================================
 */
#define SHT_ADDRESS_1     0x44
#define SHT_ADDRESS_2     0x45        // ADR pin set high, VDD
Adafruit_SHT31 sht1;
Adafruit_SHT31 sht2;
bool SHT_1_exists = false;
bool SHT_2_exists = false;

/*
 * ======================================================================================================================
 *  HIH8 - I2C - Temperature & Humidity sensor (HIH8000)  - 
 * ======================================================================================================================
 */
#define HIH8000_ADDRESS   0x27
bool HIH8_exists = false;

/*
 * ======================================================================================================================
 *  HTU21D-F - I2C - Humidity & Temp Sensor
 * ======================================================================================================================
 */
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
bool HTU21DF_exists = false;

/*
 * ======================================================================================================================
 *  Tinovi Leaf Wetness
 *    Chip ID = 0x61,  Library init checks this.
 * ======================================================================================================================
 */
#define TLW_ADDRESS     0x61
LeafSens tlw;
bool TLW_exists = false;

/*
 * ======================================================================================================================
 *  Tinovi Soil Moisture
 *    Chip ID = 0x63,  Library init checks this.
 * ======================================================================================================================
 */
#define TSM_ADDRESS     0x63
SVCS3 tsm;
bool TSM_exists = false;

/*
 * ======================================================================================================================
 *  Tinovi MultiLevel Soil Moisture (4 Soil and 2 Temperature) 
 *    Chip ID = 0x63,  Library init checks this.
 * ======================================================================================================================
 */
#define TMSM_ADDRESS    0x65
SVMULTI tmsm;
bool TMSM_exists = false;

/*
 * ======================================================================================================================
 *  Wet Bulb Temperature - Derived from Temperature and Humidity Sensonrs
 * ======================================================================================================================
 */
bool WBT_exists = false;

/*
 * ======================================================================================================================
 *  Heat Index Temperature - Derived from Temperature and Humidity Sensonrs
 * ======================================================================================================================
 */
bool HI_exists = false;

/*
 * ======================================================================================================================
 *  Wet Bulb Globe Temperature - Derived from Temperature and Humidity Sensonrs
 * ======================================================================================================================
 */
bool WBGT_exists = false;

/*
 * ======================================================================================================================
 *  Si1145 - I2C - UV/IR/Visible Light Sensor
 *  The SI1145 has a fixed I2C address (0x60), you can only connect one sensor per microcontroller!
 * ======================================================================================================================
 */
Adafruit_SI1145 uv = Adafruit_SI1145();
bool SI1145_exists = false;
// When we do a read of all three and we get zeros. If these llast readings are not zero, we will reinitialize the
// chip. When does a reset on it and then read again.
float si_last_vis = 0.0;
float si_last_ir = 0.0;
float si_last_uv = 0.0;

/*
 * ======================================================================================================================
 *  VEML7700 - I2C - Lux Sensor
 * ======================================================================================================================
 */
#define VEML7700_ADDRESS   0x10
Adafruit_VEML7700 veml = Adafruit_VEML7700();
bool VEML7700_exists = false;

/*
 * ======================================================================================================================
 *  MCP9808 - I2C - Temperature sensor
 * 
 * I2C Address is:  0011,A2,A1,A0
 *                  0011000 = 0x18  where A2,1,0 = 0 MCP9808_I2CADDR_DEFAULT  
 *                  0011001 = 0x19  where A0 = 1
 * ======================================================================================================================
 */
#define MCP_ADDRESS_1     0x18
#define MCP_ADDRESS_2     0x19        // A0 set high, VDD
Adafruit_MCP9808 mcp1;
Adafruit_MCP9808 mcp2;
bool MCP_1_exists = false;
bool MCP_2_exists = false;

/*
 * ======================================================================================================================
 *  PM25AQI - I2C - Air Quality Sensor
 * 
 *  PM25_AQI_Data variables returned are unsigned integers
 *    pm10_standard     < Standard Particle PM1.0 concentration unit µg 𝑚3
 *    pm25_standard     < Standard Particle PM2.5 concentration unit µg 𝑚3
 *    pm100_standard    < Standard Particle PM10.0 concentration unit µg 𝑚3
 *    pm10_env          < Atmospheric Environmental PM1.0 concentration unit µg 𝑚3
 *    pm25_env          < Atmospheric Environmental PM2.5 concentration unit µg 𝑚3
 *    pm100_env         < Atmospheric Environmental PM10.0 concentration unit µg 𝑚3
 *    particles_03um    < Particles with diameter beyond 0.3 µ 𝑚 in 0.1L of air
 *    particles_05um    < Particles with diameter beyond 0.5 µ 𝑚 in 0.1L of air
 *    particles_10um    < Particles with diameter beyond 1.0 µ 𝑚 in 0.1L of air
 *    particles_25um    < Particles with diameter beyond 2.5 µ 𝑚 in 0.1L of air
 *    particles_50um    < Particles with diameter beyond 5.0 µ 𝑚 in 0.1L of air
 *    particles_100um   < Particles with diameter beyond 10.0 µ 𝑚 in 0.1L of air
 * 
 *  pms = Particulate Matter Standard
 *  pme = Particulate Matter Environmental
 * 
 *  Variable Tags for what we monitor and report on
 *    pm1s10
 *    pm1s25
 *    pm1s100
 *    pm1e10
 *    pm1e25
 *    pm1e100
 * 
 * How the sensor works internally
 * Mainly output is the quality and number of each particles with different size per unit volume, the unit volume of 
 * particle number is 0.1L and the unit of mass concentration is μg/m³
 * Internal sampling is divided into two sub modes: stable mode and fast mode. If the concentration change is small 
 * the sensor would run at stable mode with the real interval of 2.3s. And if the change is big the sensor would be 
 * changed to fast mode automatically with the interval of 200~800ms, the higher of the concentration, the shorter of
 * the interval. 
 * 
 * We sample the sensor every second keeping highest 1 second returned values for the observation interval. 
 * ======================================================================================================================
 */
typedef struct {
  uint16_t max_s10, max_s25, max_s100; 
  uint16_t max_e10, max_e25, max_e100;
} PM25AQI_OBS_STR;
PM25AQI_OBS_STR pm25aqi_obs;

#define PM25AQI_ADDRESS   0x12
Adafruit_PM25AQI pmaq = Adafruit_PM25AQI();
bool PM25AQI_exists = false;

/* 
 *=======================================================================================================================
 * get_Bosch_ChipID ()  -  Return what Bosch chip is at specified address
 *   Chip ID BMP280 = 0x58 temp, preasure           - I2C ADDRESS 0x77  (SD0 to GND = 0x76)  
 *   Chip ID BME280 = 0x60 temp, preasure, humidity - I2C ADDRESS 0x77  (SD0 to GND = 0x76)  Register 0xE0 = Reset
 *   Chip ID BMP388 = 0x50 temp, preasure           - I2C ADDRESS 0x77  (SD0 to GND = 0x76)
 *   Chip ID BMP390 = 0x60 temp, preasure           - I2C ADDRESS 0x77  (SD0 to GND = 0x76)
 *=======================================================================================================================
 */
byte get_Bosch_ChipID (byte address) {
  byte chip_id = 0;
  byte error;

  Output ("get_Bosch_ChipID()");
  // The i2c_scanner uses the return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address.

  // Important! Need to check the 0x00 register first. Doing a 0x0D (not chip id loaction) on a bmp388 
  // will return a value that could match one of the IDs 

  // Check Register 0x00
  sprintf (msgbuf, "  I2C:%02X Reg:%02X", address, 0x00);
  Output (msgbuf);
  Wire.begin();
  Wire.beginTransmission(address);
  Wire.write(0x00);  // BM3 CHIPID REGISTER
  error = Wire.endTransmission();
    //  0:success
    //  1:data too long to fit in transmit buffer
    //  2:received NACK on transmit of address
    //  3:received NACK on transmit of data
    //  4:other error 
  if (error) {
    sprintf (msgbuf, "  ERR_ET:%d", error);
    Output (msgbuf);
  }
  else if (Wire.requestFrom(address, 1)) {  // Returns the number of bytes returned from the slave device 
    chip_id = Wire.read();
    if (chip_id == BMP280_CHIP_ID) { // 0x58
      sprintf (msgbuf, "  CHIPID:%02X BMP280", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!
    }
    else if (chip_id == BMP388_CHIP_ID) {
      sprintf (msgbuf, "  CHIPID:%02X BMP388", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!   
    }
    else if (chip_id == BME280_BMP390_CHIP_ID) {  // 0x60
      sprintf (msgbuf, "  CHIPID:%02X BME/390", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!   
    }
    else {
      sprintf (msgbuf, "  CHIPID:%02X InValid", chip_id);
      Output (msgbuf);      
    }
  }
  else {
    sprintf (msgbuf, "  ERR_RF:0");
    Output (msgbuf);
  }

  // Check Register 0xD0
  chip_id = 0;
  sprintf (msgbuf, "  I2C:%02X Reg:%02X", address, 0xD0);
  Output (msgbuf);
  Wire.begin();
  Wire.beginTransmission(address);
  Wire.write(0xD0);  // BM2 CHIPID REGISTER
  error = Wire.endTransmission();
    //  0:success
    //  1:data too long to fit in transmit buffer
    //  2:received NACK on transmit of address
    //  3:received NACK on transmit of data
    //  4:other error 
  if (error) {
    sprintf (msgbuf, "  ERR_ET:%d", error);
    Output (msgbuf);
  }
  else if (Wire.requestFrom(address, 1)) {  // Returns the number of bytes returned from the slave device 
    chip_id = Wire.read(); 
    if (chip_id == BMP280_CHIP_ID) {  // 0x58
      sprintf (msgbuf, "  CHIPID:%02X BMP280", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!
    }
    else if (chip_id == BMP388_CHIP_ID) {  // 0x50
      sprintf (msgbuf, "  CHIPID:%02X BMP388", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!   
    }
    else if (chip_id == BME280_BMP390_CHIP_ID) {  // 0x60
      sprintf (msgbuf, "  CHIPID:%02X BME/390", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!   
    }
    else {
      sprintf (msgbuf, "  CHIPID:%02X InValid", chip_id);
      Output (msgbuf);   
    }
  }
  else {
    sprintf (msgbuf, "  ERR_RF:0");
    Output (msgbuf);
  }
  return(0);
}

/* 
 *=======================================================================================================================
 * bmx_initialize() - Bosch sensor initialize
 *=======================================================================================================================
 */
void bmx_initialize() {
  Output("BMX:INIT");
  
  // 1st Bosch Sensor - Need to see which (BMP, BME, BM3) is plugged in
  BMX_1_chip_id = get_Bosch_ChipID(BMX_ADDRESS_1);

  switch (BMX_1_chip_id) {
    case BMP280_CHIP_ID :
      if (!bmp1.begin(BMX_ADDRESS_1)) { 
        msgp = (char *) "BMP1 ERR";
        BMX_1_exists = false;
        SystemStatusBits |= SSB_BMX_1;  // Turn On Bit          
      }
      else {
        BMX_1_exists = true;
        BMX_1_type = BMX_TYPE_BMP280;
        msgp = (char *) "BMP1 OK";
        float p = bmp1.readPressure();
      }
    break;

    case BME280_BMP390_CHIP_ID :
      if (!bme1.begin(BMX_ADDRESS_1)) { 
        if (!bm31.begin_I2C(BMX_ADDRESS_1)) {  // Perhaps it is a BMP390
          msgp = (char *) "BMX1 ERR";
          BMX_1_exists = false;
          SystemStatusBits |= SSB_BMX_1;  // Turn On Bit          
        }
        else {
          BMX_1_exists = true;
          BMX_1_type = BMX_TYPE_BMP390;
          msgp = (char *) "BMP390_1 OK"; 
          float p = bm31.readPressure();       
        }      
      }
      else {
        BMX_1_exists = true;
        BMX_1_type = BMX_TYPE_BME280;
        msgp = (char *) "BME280_1 OK";
        float p = bme1.readPressure();
      }
    break;

    case BMP388_CHIP_ID :
      if (!bm31.begin_I2C(BMX_ADDRESS_1)) { 
        msgp = (char *) "BM31 ERR";
        BMX_1_exists = false;
        SystemStatusBits |= SSB_BMX_1;  // Turn On Bit          
      }
      else {
        BMX_1_exists = true;
        BMX_1_type = BMX_TYPE_BMP388;
        msgp = (char *) "BM31 OK";
        float p = bm31.readPressure();
      }
    break;

    default:
      msgp = (char *) "BMX_1 NF";
    break;
  }
  Output (msgp);

  // 2nd Bosch Sensor - Need to see which (BMP, BME, BM3) is plugged in
  BMX_2_chip_id = get_Bosch_ChipID(BMX_ADDRESS_2);
  switch (BMX_2_chip_id) {
    case BMP280_CHIP_ID :
      if (!bmp2.begin(BMX_ADDRESS_2)) { 
        msgp = (char *) "BMP2 ERR";
        BMX_2_exists = false;
        SystemStatusBits |= SSB_BMX_2;  // Turn On Bit          
      }
      else {
        BMX_2_exists = true;
        BMX_2_type = BMX_TYPE_BMP280;
        msgp = (char *) "BMP2 OK";
        float p = bmp2.readPressure();
      }
    break;

    case BME280_BMP390_CHIP_ID :
      if (!bme2.begin(BMX_ADDRESS_2)) { 
        if (!bm32.begin_I2C(BMX_ADDRESS_2)) {  // Perhaps it is a BMP390
          msgp = (char *) "BMX2 ERR";
          BMX_2_exists = false;
          SystemStatusBits |= SSB_BMX_2;  // Turn On Bit          
        }
        else {
          BMX_2_exists = true;
          BMX_2_type = BMX_TYPE_BMP390;
          msgp = (char *) "BMP390_2 OK"; 
          float p = bm32.readPressure();         
        }
      }
      else {
        BMX_2_exists = true;
        BMX_2_type = BMX_TYPE_BME280;
        msgp = (char *) "BME280_2 OK";
        float p = bme2.readPressure();
      }
    break;

    case BMP388_CHIP_ID :
      if (!bm32.begin_I2C(BMX_ADDRESS_2)) { 
        msgp = (char *) "BM32 ERR";
        BMX_2_exists = false;
        SystemStatusBits |= SSB_BMX_2;  // Turn On Bit          
      }
      else {
        BMX_2_exists = true;
        BMX_2_type = BMX_TYPE_BMP388;
        msgp = (char *) "BM32 OK";
        float p = bm32.readPressure();
      }
    break;

    default:
      msgp = (char *) "BMX_2 NF";
    break;
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * htu21d_initialize() - HTU21D sensor initialize
 *=======================================================================================================================
 */
void htu21d_initialize() {
  Output("HTU21D:INIT");
  
  // HTU21DF Humidity & Temp Sensor (I2C ADDRESS = 0x40)
  if (!htu.begin()) {
    msgp = (char *) "HTU NF";
    HTU21DF_exists = false;
    SystemStatusBits |= SSB_HTU21DF;  // Turn On Bit
  }
  else {
    HTU21DF_exists = true;
    msgp = (char *) "HTU OK";
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * mcp9808_initialize() - MCP9808 sensor initialize
 *=======================================================================================================================
 */
void mcp9808_initialize() {
  Output("MCP9808:INIT");
  
  // 1st MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x18)
  mcp1 = Adafruit_MCP9808();
  if (!mcp1.begin(MCP_ADDRESS_1)) {
    msgp = (char *) "MCP1 NF";
    MCP_1_exists = false;
    SystemStatusBits |= SSB_MCP_1;  // Turn On Bit
  }
  else {
    MCP_1_exists = true;
    msgp = (char *) "MCP1 OK";
  }
  Output (msgp);

  // 2nd MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x19)
  mcp2 = Adafruit_MCP9808();
  if (!mcp2.begin(MCP_ADDRESS_2)) {
    msgp = (char *) "MCP2 NF";
    MCP_2_exists = false;
    SystemStatusBits |= SSB_MCP_2;  // Turn On Bit
  }
  else {
    MCP_2_exists = true;
    msgp = (char *) "MCP2 OK";
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * sht_initialize() - SHT31 sensor initialize
 *=======================================================================================================================
 */
void sht_initialize() {
  Output("SHT:INIT");
  
  // 1st SHT31 I2C Temperature/Humidity Sensor (I2C ADDRESS = 0x44)
  sht1 = Adafruit_SHT31();
  if (!sht1.begin(SHT_ADDRESS_1)) {
    msgp = (char *) "SHT1 NF";
    SHT_1_exists = false;
    SystemStatusBits |= SSB_SHT_1;  // Turn On Bit
  }
  else {
    SHT_1_exists = true;
    msgp = (char *) "SHT1 OK";
  }
  Output (msgp);

  // 2nd SHT31 I2C Temperature/Humidity Sensor (I2C ADDRESS = 0x45)
  sht2 = Adafruit_SHT31();
  if (!sht2.begin(SHT_ADDRESS_2)) {
    msgp = (char *) "SHT2 NF";
    SHT_2_exists = false;
    SystemStatusBits |= SSB_SHT_2;  // Turn On Bit
  }
  else {
    SHT_2_exists = true;
    msgp = (char *) "SHT2 OK";
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * hih8_initialize() - HIH8000 sensor initialize
 *=======================================================================================================================
 */
void hih8_initialize() {
  Output("HIH8:INIT");

  if (I2C_Device_Exist(HIH8000_ADDRESS)) {
    HIH8_exists = true;
    msgp = (char *) "HIH8 OK";
  }
  else {
    msgp = (char *) "HIH8 NF";
    HIH8_exists = false;
    SystemStatusBits |= SSB_HIH8;  // Turn On Bit
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * hih8_getTempHumid() - Get Temp and Humidity
 *   Call example:  status = hih8_getTempHumid(&t, &h);
 *=======================================================================================================================
 */
bool hih8_getTempHumid(float *t, float *h) {
  if (HIH8_exists) {
    uint16_t humidityBuffer    = 0;
    uint16_t temperatureBuffer = 0;

    Wire.begin();
    Wire.beginTransmission(HIH8000_ADDRESS);
    Wire.write(0x00); // set the register location for read request

    delayMicroseconds(200); // give some time for sensor to process request


    // We skip ending the transmission after Wire.write() because the sensor seems to treat the read and write operations
    // as a continuous transaction. 
    
    // byte error = Wire.endTransmission();
    //  0:success
    //  1:data too long to fit in transmit buffer
    //  2:received NACK on transmit of address
    //  3:received NACK on transmit of data
    //  4:other error 
    // if (error != 0) {
    //     Output("HIH8 Err:I2C Failed");
    //     return false;
    //}

    if (Wire.requestFrom(HIH8000_ADDRESS, 4) == 4) {

      // Get raw humidity data
      humidityBuffer = Wire.read();
      humidityBuffer <<= 8;
      humidityBuffer |= Wire.read();
      humidityBuffer &= 0x3FFF;   // 14bit value, get rid of the upper 2 status bits

      // Get raw temperature data
      temperatureBuffer = Wire.read();
      temperatureBuffer <<= 8;
      temperatureBuffer |= Wire.read();
      temperatureBuffer >>= 2;  // Remove the last two "Do Not Care" bits (shift left is same as divide by 4)

      // Wire.endTransmission();
      
      *h = humidityBuffer * 6.10e-3;
      *t = temperatureBuffer * 1.007e-2 - 40.0;
      return (true);
    }
    else {
      Output("HIH8 Err:No Data Recv");
      return(false);
    }
  }
  else {
    return (false);
  }
}

/* 
 *=======================================================================================================================
 * tlw_initialize() -  Tinovi Leaf Wetness initialize
 *=======================================================================================================================
 */
void tlw_initialize() {
  Output("TLW:INIT");
  
  // Tinovi Leaf Wetness initialize (I2C ADDRESS = 0x61)
  if (!I2C_Device_Exist(TLW_ADDRESS)) { 
    msgp = (char *) "TLW NF";
    TLW_exists = false;
  }
  else {
    tlw.init(TLW_ADDRESS);
    msgp = (char *) "TLW OK";
    TLW_exists = true;
    SystemStatusBits |= SSB_TLW;  // Turn On Bit
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * tsm_initialize() -  Tinovi Soil Moisture initialize
 *=======================================================================================================================
 */
void tsm_initialize() {
  Output("TSM:INIT");
  
  // Tinovi Soil Moisture initialize (I2C ADDRESS = 0x63)
  if (!I2C_Device_Exist(TSM_ADDRESS)) { 
    msgp = (char *) "TSM NF";
    TSM_exists = false;
  }
  else {
    tsm.init(TSM_ADDRESS);
    msgp = (char *) "TSM OK";
    TSM_exists = true;
    SystemStatusBits |= SSB_TSM;  // Turn On Bit
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * tmsm_initialize() -  Tinovi MultiLevel Soil Moisture initialize
 *=======================================================================================================================
 */
void tmsm_initialize() {
  Output("TMSM:INIT");
  
  // Tinovi MultiLevel Soil Moisture initialize (I2C ADDRESS = 0x65)
  if (!I2C_Device_Exist(TMSM_ADDRESS)) { 
    msgp = (char *) "TMSM NF";
    TMSM_exists = false;
  }
  else {
    tmsm.init(TMSM_ADDRESS);
    msgp = (char *) "TMSM OK";
    TMSM_exists = true;
    SystemStatusBits |= SSB_TMSM;  // Turn On Bit
  }
  Output (msgp);
}


/* 
 *=======================================================================================================================
 * wbt_initialize() - Wet Bulb Temperature
 *=======================================================================================================================
 */
void wbt_initialize() {
  Output("WBT:INIT");
  if (MCP_1_exists && SHT_1_exists) {
    WBT_exists = true;
    Output ("WBT:OK");
  }
  else {
    Output ("WBT:NF");
  }
}

/* 
 *=======================================================================================================================
 * wbt_calculate() - Compute Web Bulb Temperature
 * 
 * By definition, wet-bulb temperature is the lowest temperature a portion of air can acquire by evaporative 
 * cooling only. When air is at its maximum (100 %) humidity, the wet-bulb temperature is equal to the normal 
 * air temperature (dry-bulb temperature). As the humidity decreases, the wet-bulb temperature becomes lower 
 * than the normal air temperature. Forecasters use wet-bulb temperature to predict rain, snow, or freezing rain.
 * 
 * SEE https://journals.ametsoc.org/view/journals/apme/50/11/jamc-d-11-0143.1.xml
 * SEE https://www.omnicalculator.com/physics/wet-bulb
 * 
 * Tw = T * atan[0.151977(RH + 8.3,3659)^1/2] + atan(T + RH%) - atan(RH - 1.676311)  + 0.00391838(RH)^3/2 * atan(0.023101 * RH%) - 4.686035
 * 
 * [ ] square bracket denote grouping for order of operations. 
 *     In Arduino code, square brackets are not used for mathematical operations. Instead, parentheses ( ).
 * sqrt(x) computes the square root of x, which is x to the 1/2.
 * pow(RH, 1.5) calculates RH to the 3/2, which is the relative humidity raised to the power of 1.5.
 *=======================================================================================================================
 */
double wbt_calculate(double T, double RH) {
  if ((T == -999.9) || (RH == -999.9)) {
    return (-999.9);
  }

  // Output("WBT:CALC");

  // Equation components
  double term1 = T * atan(0.151977 * sqrt(RH + 8.313659));
  double term2 = atan(T + RH);
  double term3 = atan(RH - 1.676311);
  double term4 = 0.00391838 * pow(RH, 1.5) * atan(0.023101 * RH);
  double constant = 4.686035;

  // Wet bulb temperature calculation
  double Tw = term1 + term2 - term3 + term4 - constant;

  Tw = (isnan(Tw) || (Tw < QC_MIN_T)  || (Tw >QC_MAX_T))  ? QC_ERR_T  : Tw;
  return (Tw);
}

/* 
 *=======================================================================================================================
 * hi_initialize() - Heat Index Temperature
 *=======================================================================================================================
 */
void hi_initialize() {
  Output("HI:INIT");
  if (MCP_1_exists && SHT_1_exists) {
    HI_exists = true;
    Output ("HI:OK");
  }
  else {
    Output ("HI:NF");
  }
}

/* 
 *=======================================================================================================================
 * hi_calculate() - Compute Heat Index Temperature Returns Celsius
 * 
 * SEE https://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
 * 
 * The regression equation of Rothfusz is:
 * HI = -42.379 + 2.04901523*T + 10.14333127*RH - .22475541*T*RH - .00683783*T*T - .05481717*RH*RH + .00122874*T*T*RH + 
 *      .00085282*T*RH*RH - .00000199*T*T*RH*RH
 * 
 * The Rothfusz regression is not appropriate when conditions of temperature and humidity 
 * warrant a heat index value below about 80 degrees F. In those cases, a simpler formula 
 * is applied to calculate values consistent with Steadman's results:
 * HI = 0.5 * {T + 61.0 + [(T-68.0)*1.2] + (RH*0.094)} 
 *=======================================================================================================================
 */
float hi_calculate(float T, float RH) {
  float HI;
  float HI_f;

  if ((T == -999.9) || (RH == -999.9)) {
    return (-999.9);
  }

  // Convert temperature from Celsius to Fahrenheit
  float T_f = T * 9.0 / 5.0 + 32.0;

  // Steadman's equation
  HI_f = 0.5 * (T_f + 61.0 + ((T_f - 68.0)*1.2) + (RH * 0.094));

  // Compute the average of the simple HI with the actual temperature [deg F]
  HI_f = (HI_f + T_f) / 2;

  if (HI_f >= 80.0) { 
    // Use Rothfusz's equation
    
    // Constants for the Heat Index formula
    float c1 = -42.379;
    float c2 = 2.04901523;
    float c3 = 10.14333127;
    float c4 = -0.22475541;
    float c5 = -0.00683783;
    float c6 = -0.05481717;
    float c7 = 0.00122874;
    float c8 = 0.00085282;
    float c9 = -0.00000199;
    
    // Heat Index calculation
    HI_f = c1 + (c2 * T_f) + (c3 * RH) + (c4 * T_f * RH) +
                (c5 * T_f * T_f) + (c6 * RH * RH) + 
                (c7 * T_f * T_f * RH) + (c8 * T_f * RH * RH) +
                (c9 * T_f * T_f * RH * RH);

    if ((RH < 13.0) && ((T_f > 80.0) && (T_f < 112.0)) ) {
      // If the RH is less than 13% and the temperature is between 80 and 112 degrees F, 
      // then the following adjustment is subtracted from HI: 
      // ADJUSTMENT = [(13-RH)/4]*SQRT{[17-ABS(T-95.)]/17}

      float Adjustment = ( (13 - RH) / 4 ) * sqrt( (17 - abs(T_f - 95.0) ) / 17 );

      HI_f = HI_f - Adjustment;

    }
    else if ((RH > 85.0) && ((T_f > 80.0) && (T_f < 87.0)) ) {
      // If the RH is greater than 85% and the temperature is between 80 and 87 degrees F, 
      // then the following adjustment is added to HI: 
      // ADJUSTMENT = [(RH-85)/10] * [(87-T)/5]

      float Adjustment = ( (RH - 85) / 10 ) * ( (87.0 - T_f) / 5 );

      HI_f = HI_f + Adjustment;
    }
  }

  // Convert Heat Index from Fahrenheit to Celsius
  HI = (HI_f - 32.0) * 5.0 / 9.0;

  // Quality Control Check
  HI = (isnan(HI) || (HI < QC_MIN_HI)  || (HI >QC_MAX_HI))  ? QC_ERR_HI  : HI;

  return (HI);
}

/* 
 *=======================================================================================================================
 * wbgt_initialize() - Wet Bulb Temperature
 *=======================================================================================================================
 */
void wbgt_initialize() {
  Output("WBGT:INIT");
  if (MCP_1_exists && SHT_1_exists) {
    WBGT_exists = true;
    Output ("WBGT:OK");
  }
  else {
    Output ("WBGT:NF");
  }
}

/* 
 *=======================================================================================================================
 * wbgt_calculate() - Compute Web Bulb Globe Temperature
 *=======================================================================================================================
 */
double wbgt_calculate(double HIc) {
  if (HIc == -999.9) {
    return (-999.9);
  }

  double HIf = HIc * 9.0 / 5.0 + 32.0;

  // Below produces Wet Bulb Globe Temperature in Celsius
  double TWc = -0.0034 * pow(HIf, 2) + 0.96 * HIf - 34;

  TWc = (isnan(TWc) || (TWc < QC_MIN_T)  || (TWc >QC_MAX_T))  ? QC_ERR_T  : TWc;
  return (TWc);
}

/* 
 *=======================================================================================================================
 * wbgt_calculate_opt2() - Compute Web Bulb Globe Temperature
 *=======================================================================================================================
 */
double wbgt_calculate_opt2(double Tc, double HIc) {
  if ((Tc == -999.9) || (HIc == -999.9)) {
    return (-999.9);
  }

  // Constants for the approximation
  const float a = 0.7;
  const float b = 0.2;
  const float c = 0.1;
 
  // Using temperature as a proxy for dry bulb temperature (Td)
  float Td = Tc * 9.0 / 5.0 + 32.0;
 
  // Using heat index as a proxy for wet bulb temperature (Tw)
  float Tw = HIc * 9.0 / 5.0 + 32.0;

  // Assuming globe temperature (Tg) as average of temperature and heat index
  float Tg = (Td + Tw) / 2;

  // Calculate WBGT using simplified formula
  float WBGT = a * Tw + b * Tg + c * Td;

  /*
  sprintf (msgbuf, "Td[%d.%d] Tw[%d.%d] Tg[%d.%d] WBGT[%d.%d]", 
    (int)Td, (int)(Td*100)%100, 
    (int)Tw, (int)(Tw*100)%100,
    (int)Tg, (int)(Tg*100)%100, 
    (int)WBGT, (int)(WBGT*100)%100
    );
  Output (msgbuf);
  */

  // Convert Heat Index from Fahrenheit to Celsius
  WBGT = (WBGT - 32.0) * 5.0 / 9.0;
  return (WBGT);
}

/* 
 *=======================================================================================================================
 * si1145_initialize() - SI1145 sensor initialize
 *=======================================================================================================================
 */
void si1145_initialize() {
  Output("SI1145:INIT");
  
  // SSB_SI1145 UV index & IR & Visible Sensor (I2C ADDRESS = 0x60)
  if (! uv.begin(&Wire)) {
    Output ("SI:NF");
    SI1145_exists = false;
    SystemStatusBits |= SSB_SI1145;  // Turn On Bit
  }
  else {
    SI1145_exists = true;
    Output ("SI:OK");
    si_last_vis = uv.readVisible();
    si_last_ir = uv.readIR();
    si_last_uv = uv.readUV()/100.0;

    sprintf (msgbuf, "SI:VI[%d.%02d]", (int)si_last_vis, (int)(si_last_vis*100.0)%100); 
    Output (msgbuf);
    sprintf (msgbuf, "SI:IR[%d.%02d]", (int)si_last_ir, (int)(si_last_ir*100.0)%100); 
    Output (msgbuf);
    sprintf (msgbuf, "SI:UV[%d.%02d]", (int)si_last_uv, (int)(si_last_uv*100.0)%100); 
    Output (msgbuf);
  }
}

/* 
 *=======================================================================================================================
 * lux_initialize() - VEML7700 sensor initialize
 * 
 * SEE https://learn.microsoft.com/en-us/windows/win32/sensorsapi/understanding-and-interpreting-lux-values
 * 
 * This data set is for illustration and may not be completely accurate for all users or situations.
 * 
 * Lighting condition     From (lux)     To (lux)     Mean value (lux)     Lighting step
 * Pitch Black            0              10           5                    1
 * Very Dark              11             50           30                   2
 * Dark Indoors           51             200          125                  3
 * Dim Indoors            201            400          300                  4
 * Normal Indoors         401            1000         700                  5
 * Bright Indoors         1001           5000         3000                 6
 * Dim Outdoors           5001           10,000       7500                 7
 * Cloudy Outdoors        10,001         30,000       20,000               8
 * Direct Sunlight        30,001         100,000      65,000               9
 * 
 * From www.vishay.com - Designing the VEML7700 Into an Application
 * 1    lx Full moon overhead at tropical latitudes
 * 3.4  lx Dark limit of civil twilight under a clear sky
 * 50   lx Family living room
 * 80   lx Hallway / bathroom
 * 100  lx Very dark overcast day
 * 320  lx to 500 lx Office lighting
 * 400  lx Sunrise or sunset on a clear day
 * 1000 lx Overcast day; typical TV studio lighting
 * 
 *=======================================================================================================================
 */
#ifdef NOWAY
void lux_initialize() {
  Output("LUX:INIT");

  if (veml.begin()) {
    VEML7700_exists = true;
    msgp = (char *) "LUX OK";
  }
  else {
    msgp = (char *) "LUX NF";
    VEML7700_exists = false;
    SystemStatusBits |= SSB_LUX;  // Turn On Bit
  }
  Output (msgp);
}
#endif

/* 
 *=======================================================================================================================
 * pm25aqi_clear() - clear observation
 *=======================================================================================================================
 */
void pm25aqi_clear() {
  pm25aqi_obs.max_s10 = 0;
  pm25aqi_obs.max_s25 = 0;
  pm25aqi_obs.max_s100 = 0;
  pm25aqi_obs.max_e10 = 0;
  pm25aqi_obs.max_e25 = 0;
  pm25aqi_obs.max_e100 = 0;
}

#ifdef NOWAY
/* 
 *=======================================================================================================================
 * pm25aqi_initialize() - air quality sensor
 *=======================================================================================================================
 */
void pm25aqi_initialize() {
  Output("PM25AQI:INIT");
  Wire.beginTransmission(PM25AQI_ADDRESS);
  if (Wire.endTransmission()) {
    msgp = (char *) "PM:NF";
    PM25AQI_exists = false;
    SystemStatusBits |= SSB_PM25AQI;  // Turn On Bit
  }
  else {
    if (! pmaq.begin_I2C()) {      // connect to the sensor over I2C
      msgp = (char *) "PM:Begin NF";
      PM25AQI_exists = false;
    }
    else {
      msgp = (char *) "PM:OK";
      PM25AQI_exists = true;
      pm25aqi_clear();
    }
  }
  Output (msgp);
}
#endif

/* 
 *=======================================================================================================================
 * pm25aqi_initialize() - air quality sensor
 *=======================================================================================================================
 */
void pm25aqi_initialize() {
  Output("PM25AQI:INIT");
  if (I2C_Device_Exist(PM25AQI_ADDRESS)) {
    if (! pmaq.begin_I2C()) {      // connect to the sensor over I2C
      msgp = (char *) "PM:Begin NF";
      PM25AQI_exists = false;
    }
    else {
      msgp = (char *) "PM:OK";
      PM25AQI_exists = true;
      pm25aqi_clear();
    }
  }
  else {
    msgp = (char *) "PM:NF";
    PM25AQI_exists = false;
    SystemStatusBits |= SSB_PM25AQI;  // Turn On Bit
  }
  Output (msgp);
}


/* 
 *=======================================================================================================================
 * pm25aqi_TakeReading() - take air quality reading - keep the larger value
 *=======================================================================================================================
 */
void pm25aqi_TakeReading() {
  if (PM25AQI_exists) {
    PM25_AQI_Data aqid;

    if (pmaq.read(&aqid)) {
      if (aqid.pm10_standard  > pm25aqi_obs.max_s10)  { pm25aqi_obs.max_s10  = aqid.pm10_standard;  }
      if (aqid.pm25_standard  > pm25aqi_obs.max_s25)  { pm25aqi_obs.max_s25  = aqid.pm25_standard;  }
      if (aqid.pm100_standard > pm25aqi_obs.max_s100) { pm25aqi_obs.max_s100 = aqid.pm100_standard; }

      if (aqid.pm10_env  > pm25aqi_obs.max_e10)  { pm25aqi_obs.max_e10  = aqid.pm10_env;  }
      if (aqid.pm25_env  > pm25aqi_obs.max_e25)  { pm25aqi_obs.max_e25  = aqid.pm25_env;  }
      if (aqid.pm100_env > pm25aqi_obs.max_e100) { pm25aqi_obs.max_e100 = aqid.pm100_env; }
    }
    else {
      SystemStatusBits &= ~SSB_PM25AQI; // Turn Off Bit
      PM25AQI_exists = false;
      Output ("PM OFFLINE");
    }
  }
}

/*
 * ======================================================================================================================
 * I2C_Check_Sensors() - See if each I2C sensor responds on the bus and take action accordingly             
 * ======================================================================================================================
 */
void I2C_Check_Sensors() {

  // BMX_1 Barometric Pressure 
  if (I2C_Device_Exist (BMX_ADDRESS_1)) {
    // Sensor online but our state had it offline
    if (BMX_1_exists == false) {
      if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
        if (bmp1.begin(BMX_ADDRESS_1)) { 
          BMX_1_exists = true;
          Output ("BMP1 ONLINE");
          SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
        } 
      }
      else if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
        if (bme1.begin(BMX_ADDRESS_1)) { 
          BMX_1_exists = true;
          Output ("BME1 ONLINE");
          SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
        }          
      }
      else {
        if (bm31.begin_I2C(BMX_ADDRESS_1)) { 
          BMX_1_exists = true;
          Output ("BM31 ONLINE");
          SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
        }                  
      }      
    }
  }
  else {
    // Sensor offline but our state has it online
    if (BMX_1_exists == true) {
      BMX_1_exists = false;
      Output ("BMX1 OFFLINE");
      SystemStatusBits |= SSB_BMX_1;  // Turn On Bit 
    }    
  }

  // BMX_2 Barometric Pressure 
  if (I2C_Device_Exist (BMX_ADDRESS_2)) {
    // Sensor online but our state had it offline
    if (BMX_2_exists == false) {
      if (BMX_2_chip_id == BME280_BMP390_CHIP_ID) {
        if (bmp2.begin(BMX_ADDRESS_2)) { 
          BMX_2_exists = true;
          Output ("BMP2 ONLINE");
          SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
        } 
      }
      else if (BMX_2_chip_id == BME280_BMP390_CHIP_ID) {
        if (bme2.begin(BMX_ADDRESS_2)) { 
          BMX_2_exists = true;
          Output ("BME2 ONLINE");
          SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
        }          
      }
      else {
         if (bm32.begin_I2C(BMX_ADDRESS_2)) { 
          BMX_2_exists = true;
          Output ("BM32 ONLINE");
          SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
        }                         
      }     
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (BMX_2_exists == true) {
      BMX_2_exists = false;
      Output ("BMX2 OFFLINE");
      SystemStatusBits |= SSB_BMX_2;  // Turn On Bit 
    }    
  }

  // HTU21DF Humidity & Temp Sensor
  if (I2C_Device_Exist (HTU21DF_I2CADDR)) {
    // Sensor online but our state had it offline
    if (HTU21DF_exists == false) {
      // See if we can bring sensor online
      if (htu.begin()) {
        HTU21DF_exists = true;
        Output ("HTU ONLINE");
        SystemStatusBits &= ~SSB_HTU21DF; // Turn Off Bit
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (HTU21DF_exists == true) {
      HTU21DF_exists = false;
      Output ("HTU OFFLINE");
      SystemStatusBits |= SSB_HTU21DF;  // Turn On Bit
    }   
  }

#ifdef NOWAY    // Sensor fails to update if this code is enabled
  // MCP9808 Precision I2C Temperature Sensor
  if (I2C_Device_Exist (MCP_ADDRESS_1)) {
    // Sensor online but our state had it offline
    if (MCP_1_exists == false) {
      // See if we can bring sensor online
      if (mcp1.begin(MCP_ADDRESS_1)) {
        MCP_1_exists = true;
        Output ("MCP ONLINE");
        SystemStatusBits &= ~SSB_MCP_1; // Turn Off Bit
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (MCP_1_exists == true) {
      MCP_1_exists = false;
      Output ("MCP OFFLINE");
      SystemStatusBits |= SSB_MCP_1;  // Turn On Bit
    }   
  }
#endif

  // SI1145 UV index & IR & Visible Sensor
  if (I2C_Device_Exist (SI1145_ADDR)) {
    // Sensor online but our state had it offline
    if (SI1145_exists == false) {
      // See if we can bring sensore online
      if (uv.begin()) {
        SI1145_exists = true;
        Output ("SI ONLINE");
        SystemStatusBits &= ~SSB_SI1145; // Turn Off Bit
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (SI1145_exists == true) {
      SI1145_exists = false;
      Output ("SI OFFLINE");
      SystemStatusBits |= SSB_SI1145;  // Turn On Bit
    }   
  }
  
  // AS5600 Wind Direction
  if (I2C_Device_Exist (AS5600_ADR)) {
    // Sensor online but our state had it offline
    if (AS5600_exists == false) {
      AS5600_exists = true;
      Output ("WD ONLINE");
      SystemStatusBits &= ~SSB_AS5600; // Turn Off Bit
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (AS5600_exists == true) {
      AS5600_exists = false;
      Output ("WD OFFLINE");
      SystemStatusBits |= SSB_AS5600;  // Turn On Bit
    }   
  }

#ifdef NOWAY
  // VEML7700 Lux 
  if (I2C_Device_Exist (VEML7700_ADDRESS)) {
    // Sensor online but our state had it offline
    if (VEML7700_exists == false) {
      // See if we can bring sensore online
      if (veml.begin()) {
        VEML7700_exists = true;
        Output ("LUX ONLINE");
        SystemStatusBits &= ~SSB_LUX; // Turn Off Bit
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (VEML7700_exists == true) {
      VEML7700_exists = false;
      Output ("LUX OFFLINE");
      SystemStatusBits |= SSB_LUX;  // Turn On Bit
    }   
  }
#endif

  // PM25AQI
  if (I2C_Device_Exist (PM25AQI_ADDRESS)) {
    // Sensor online but our state had it offline
    if (PM25AQI_exists == false) {
      // See if we can bring sensor online
      if (pmaq.begin_I2C()) {
        PM25AQI_exists = true;
        Output ("PM ONLINE");
        SystemStatusBits &= ~SSB_PM25AQI; // Turn Off Bit
        pm25aqi_clear();
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (PM25AQI_exists == true) {
      PM25AQI_exists = false;
      Output ("PM OFFLINE");
      SystemStatusBits |= SSB_PM25AQI;  // Turn On Bit
    }   
  }
}
