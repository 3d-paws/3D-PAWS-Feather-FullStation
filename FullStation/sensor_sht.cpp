/*
 * ======================================================================================================================
 * sensor_sht.cpp - Sensirion I2C Sensors
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/nvcf.h"
#include "include/obs.h"
#include "include/sensor_sht.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  SHT - I2C - Temperature & Hmidity
 * ======================================================================================================================
 */
Adafruit_SHT31 sht31;
Adafruit_SHT31 sht32;
Adafruit_SHT4x sht4;

// We are going to support 2 SHT sensors combination.  
// Rule of Discovery - Check SHT31 1st.
// For st1 & sh1 - SHT31 or SHT45 @ 0x44  - SHT45(0x44 Only)
// For st2 & sh2 - SHT31 @ 0x45

bool SHT_1_exists = false;
bool SHT_2_exists = false;
byte SHT_1_type=SHT_TYPE_UNKNOWN;
byte SHT_2_type=SHT_TYPE_UNKNOWN;
uint32_t SHT_1_sn=0;
uint32_t SHT_2_sn=0;
const char *shttype[] = {"UNKN", "SHT31", "SHT45"};
float sht1_temp = -999.9;  // Used with Derived Observations (hi, wbt, wgbt, mslp)
float sht1_humid = -999.9; // Used with Derived Observation (mslp)

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * sht31_probe() - return true if sensor found at address.
 * =======================================================================================================================
 */
bool sht31_probe(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x37);
  Wire.write(0x80);
  if (Wire.endTransmission(false) != 0) return false;

  delay(1);

  uint8_t data[6];
  if (Wire.requestFrom(addr, (uint8_t)6) != 6) return false;

  for (int i = 0; i < 6; i++) data[i] = Wire.read();

  uint16_t sn1 = ((uint16_t)data[0] << 8) | data[1];
  uint16_t sn2 = ((uint16_t)data[3] << 8) | data[4];

  return (sn1 != 0x0000 || sn2 != 0x0000);
}

/*
 * ======================================================================================================================
 * sht45_probe()
 * =======================================================================================================================
 */
bool sht45_probe() {
  return sht4.begin();
}

/* 
 *=======================================================================================================================
 * sht3x_read_sn() - Read SHT31 Serial Number
 *=======================================================================================================================
 */
uint32_t sht3x_read_sn(uint8_t i2cAddr) {
  uint8_t buffer[6];
  const uint16_t READ_SERIAL_CMD = 0x3780;

  // 1. Send the command
  Wire.beginTransmission(i2cAddr);
  Wire.write(highByte(READ_SERIAL_CMD));
  Wire.write(lowByte(READ_SERIAL_CMD));
  
  // Return 0 if the transmission fails (sensor not present or bus busy)
  if (Wire.endTransmission() != 0) {
    return 0;
  }

  delay(1);

  // 2. Request the 6-byte data packet
  if (Wire.requestFrom(i2cAddr, (uint8_t)6) != 6) {
    return 0; // Return 0 if we didn't receive exactly 6 bytes
  }

  for (int i = 0; i < 6; i++) {
    buffer[i] = Wire.read();
  }

  // 3. Reconstruct and return the 32-bit serial number
  return ((uint32_t)buffer[0] << 24) | 
         ((uint32_t)buffer[1] << 16) | 
         ((uint32_t)buffer[3] << 8)  | 
         ((uint32_t)buffer[4]);
}

/* 
 *=======================================================================================================================
 * sht3x_detail() - display sht3x details
 *=======================================================================================================================
 */
void sht3x_detail(uint8_t i2cAddr) {
  int n = (i2cAddr == SHT_ADDR_1) ? 1 : 2;

  // sprintf (Buffer32Bytes, "  SHT3%d Information", n);
  // Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " SN:%lX", (n==1) ? SHT_1_sn : SHT_2_sn);
  Output(Buffer32Bytes);

  if ((n==1) ? sht31.isHeaterEnabled() : sht32.isHeaterEnabled()) {
    Output(" Heater On");
  }
  else {
    Output(" No Heater");
  }
}

/* 
 *=======================================================================================================================
 * sht4_detail() - display sht4x details
 *=======================================================================================================================
 */
void sht4_detail() {
  // Output (" SHT41 Information");
  sprintf (Buffer32Bytes, " SN: %lX", sht4.readSerial());
  Output(Buffer32Bytes);

  switch (sht4.getPrecision()) {
     case SHT4X_HIGH_PRECISION:  // default
       Output(" High precision");
       break;
     case SHT4X_MED_PRECISION: 
       Output(" Med precision");
       break;
     case SHT4X_LOW_PRECISION: 
       Output(" Low precision");
       break;
  }

  switch (sht4.getHeater()) {
     case SHT4X_NO_HEATER:   // default
       Output(" No heater");
       break;
     case SHT4X_HIGH_HEATER_1S: 
       Output(" High heat for 1 second");
       break;
     case SHT4X_HIGH_HEATER_100MS: 
       Output( "High heat for 0.1 second");
       break;
     case SHT4X_MED_HEATER_1S: 
       Output(" Medium heat for 1 second");
       break;
     case SHT4X_MED_HEATER_100MS: 
       Output(" Medium heat for 0.1 second");
       break;
     case SHT4X_LOW_HEATER_1S: 
       Output(" Low heat for 1 second");
       break;
     case SHT4X_LOW_HEATER_100MS: 
       Output(" Low heat for 0.1 second");
       break;
  }
}

/* 
 *=======================================================================================================================
 * sht1_read() - Read h t from Sensirion sensor
 *=======================================================================================================================
 */
void sht1_read(float &h, float &t) {
  h = -999.9;
  t = -999.9;

  if (SHT_1_exists) {
    switch (SHT_1_type) {
      case SHT_TYPE_SHT31 :
        h = sht31.readHumidity();
        t = sht31.readTemperature();
        break; 
      case SHT_TYPE_SHT45 :
        sensors_event_t humidity, temp;
        sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
        t = temp.temperature;
        h = humidity.relative_humidity;
        break;  
      default: // WTF
        Output ("sht1_read (WTF)");
        return;
        break;
    }
    h = (isnan(h) || (h < QC_MIN_RH)  || (h > QC_MAX_RH))  ? QC_ERR_RH  : h;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
  }
}

/* 
 *=======================================================================================================================
 * sht2_read() - Read h t from Sensirion sensor
 *=======================================================================================================================
 */
void sht2_read(float &h, float &t) {
  h = -999.9;
  t = -999.9;

  if (SHT_2_exists) {
    switch (SHT_2_type) {
      case SHT_TYPE_SHT31 :
        h = sht32.readHumidity();
        t = sht32.readTemperature();
        break;
      default: // WTF
        Output ("sht2_read (WTF)");
        return;
        break;
    }
    h = (isnan(h) || (h < QC_MIN_RH)  || (h > QC_MAX_RH))  ? QC_ERR_RH  : h;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
  }
}

/* 
 *=======================================================================================================================
 * sht_obs_do() - 
 *=======================================================================================================================
 */
void sht_obs_do(int &sidx) {

  if (SHT_1_exists) {
    float h,t;
    sht1_read(h, t);

    // Temperature
    strcpy (obs.sensor[sidx].id, "st1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // Humidity
    strcpy (obs.sensor[sidx].id, "sh1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) h;
    obs.sensor[sidx++].inuse = true;

    sht1_temp = t;
    sht1_humid = h; 
  }

  if (SHT_2_exists) {
    float h,t;
    sht2_read(h, t);

    // Temperature
    strcpy (obs.sensor[sidx].id, "st2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // Pressure 
    strcpy (obs.sensor[sidx].id, "sh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) h;
    obs.sensor[sidx++].inuse = true;
  }
}

/* 
 *=======================================================================================================================
 * sht_info() - add to msg sht sensors, Passing msg ptr, msg bubber sz, and pointer to comma
 *=======================================================================================================================
 */
void sht_info(char *rest, int size, const char *&comma) {

  if (SHT_1_exists && (SHT_1_type!=SHT_TYPE_UNKNOWN)) {
    int used = strlen(rest);
    int n;

    if (SHT_1_type == SHT_TYPE_SHT31) {
      n = sprintf(Buffer32Bytes, "%s%s(%lX)", comma, shttype[SHT_1_type], SHT_1_sn);
    }
    else {
      n = sprintf(Buffer32Bytes, "%s%s", comma, shttype[SHT_1_type]);
    }
    
    // can we add with out over flowing?
    if ((used + n) < size) {
      sprintf(rest + used, "%s", Buffer32Bytes);
      comma = ",";
    }
    else {
      return;
    }
  }

  if (SHT_2_exists && (SHT_2_type!=SHT_TYPE_UNKNOWN)) {
    int used = strlen(rest);
    int n = sprintf(Buffer32Bytes, "%s%s(%lX)", comma, shttype[SHT_2_type], SHT_2_sn);
    // can we add with out over flowing?
    if ((used + n) < size) {
      sprintf(rest + used, "%s", Buffer32Bytes);
      comma = ",";
    }
    else {
      return;
    }
  }
}

/* 
 *=======================================================================================================================
 * sht_initialize() - Sensirion sensor initialize
 *=======================================================================================================================
 */
void sht_initialize() {
  Output("SHT:INIT");
  
  // 1st Sensirion Sensor
  if (sht31_probe(SHT_ADDR_1)) {
    if (!sht31.begin(SHT_ADDR_1)) {
      Output ("SHT1 SHT31 ERR");
      SHT_1_exists = false;
    }
    else {
      Output ("SHT1 SHT31 OK");
      SHT_1_exists = true;
      SHT_1_type = SHT_TYPE_SHT31;
      SHT_1_sn = sht3x_read_sn(SHT_ADDR_1);
      sht3x_detail(SHT_ADDR_1);
    }
  }
  else if (sht4.begin()) {
    Output ("SHT1 SHT45 OK");
    // You can have 3 different precisions, higher precision takes longer
    sht4.setPrecision(SHT4X_HIGH_PRECISION);

    // You can have 6 different heater settings
    // higher heat and longer times uses more power
    // and reads will take longer too!
    sht4.setHeater(SHT4X_NO_HEATER);

    SHT_1_exists = true;
    SHT_1_type = SHT_TYPE_SHT45;
    sht4_detail();
  }
  else {
    Output ("SHT1 NF");
  } 

  // 2nd Sensirion Sensor can only be a SHT31
  if (sht31_probe(SHT_ADDR_2)) {
    if (!sht32.begin(SHT_ADDR_2)) {
      Output ("SHT2 SHT31 ERR");
      SHT_2_exists = false;
    }
    else {
      Output ("SHT2 SHT31 OK");
      SHT_2_exists = true;
      SHT_2_type = SHT_TYPE_SHT31;
      SHT_2_sn = sht3x_read_sn(SHT_ADDR_2);
      sht3x_detail(SHT_ADDR_2);
    }
  }
  else {
    Output ("SHT2 NF");
  }
}