/*
 * ======================================================================================================================
 * sensor_bmp.cpp - Bosch I2C Sensors
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/nvcf.h"
#include "include/obs.h"
#include "include/sensor_bmp.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  BMP - I2C - Temperature, pressure sensor & altitude
 * ======================================================================================================================
 */
Adafruit_BMP3XX bmp31;
Adafruit_BMP3XX bmp32;
Adafruit_BMP5xx bmp51;
Adafruit_BMP5xx bmp52;

// We are going to support 2 BMP sensors combination.  
// Rule of Discovery - Check BMP390 1st.
// For bt1 & bp1 - BMP31 @ 0x77 or BMP51 @ 0x47
// For bt2 & bp2 - BMP32 @ 0x76 or BMP52 @ 0x46

byte BMP_1_chip_id = 0x00;
byte BMP_2_chip_id = 0x00;
bool BMP_1_exists = false;
bool BMP_2_exists = false;
byte BMP_1_type=BMP_TYPE_UNKNOWN;
byte BMP_2_type=BMP_TYPE_UNKNOWN;
const char *bmptype[] = {"UNKN", "BMP390", "BMP581"};
float bmp_1_pressure = -999.9; // Used with Derived Observation (mslp)

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * bmp390_probe()
 * =======================================================================================================================
 */
bool bmp390_probe(uint8_t addr) {
  Wire.begin();
  Wire.beginTransmission(addr);
  Wire.write(0x00);  // CHIP_ID register
  if (Wire.endTransmission(false) != 0) return false;

  if (Wire.requestFrom(addr, (uint8_t)1) != 1) return false;

  uint8_t chipid = Wire.read();
  return (chipid == BMP390_CHIPID);
}

/*
 * ======================================================================================================================
 * bmp581_probe()
 * =======================================================================================================================
 */
bool bmp581_probe(uint8_t addr) {
  Wire.begin();
  Wire.beginTransmission(addr);
  Wire.write(0x01); // CHIP_ID register
  if (Wire.endTransmission(false) != 0) return false;

  if (Wire.requestFrom(addr, (uint8_t)1) != 1) return false;

  uint8_t chipid = Wire.read();
  return (chipid == BMP581_CHIPID);
}

/* 
 *=======================================================================================================================
 * bmp1_read() - Read p t from Bosch sensor
 *=======================================================================================================================
 */
void bmp1_read(float &p, float &t) {

  p = -999.9;
  t = -999.9;

  if (BMP_1_exists) {
    switch (BMP_1_type) {
      case BMP_TYPE_BMP390 :
        p = bmp31.readPressure() / 100.0F;
        t = bmp31.readTemperature();
        break; 
      case BMP_TYPE_BMP581 :
        t = bmp51.readTemperature();
        p = bmp51.readPressure();
        break;  
      default: // WTF
        Output ("bmp1_read (WTF)");
        return;
        break;
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
  }
}

/* 
 *=======================================================================================================================
 * bmp2_read() - Read p t from Bosch sensor
 *=======================================================================================================================
 */
void bmp2_read(float &p, float &t) {

  p = -999.9;
  t = -999.9;

  if (BMP_2_exists) {
    switch (BMP_2_type) {
      case BMP_TYPE_BMP390 :
        p = bmp32.readPressure() / 100.0F;
        t = bmp32.readTemperature();
        break;
      case BMP_TYPE_BMP581 :
        t = bmp52.readTemperature();
        p = bmp52.readPressure();
        break;   
      default: // WTF
        Output ("bmp2_read (WTF)");
        return;
        break;
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
  }
}

/* 
 *=======================================================================================================================
 * bmp_obs_do() - 
 *=======================================================================================================================
 */
void bmp_obs_do(int &sidx) {

  if (BMP_1_exists) {
    float p,t;
    bmp1_read(p, t);

    // Temperature
    strcpy (obs.sensor[sidx].id, "bt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // Pressure 
    strcpy (obs.sensor[sidx].id, "bp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) p;
    obs.sensor[sidx++].inuse = true;

    bmp_1_pressure = p; // Save for use in mslp calc
  }

  if (BMP_2_exists) {
    float p,t;
    bmp2_read(p, t);

    // Temperature
    strcpy (obs.sensor[sidx].id, "bt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // Pressure 
    strcpy (obs.sensor[sidx].id, "bp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) p;
    obs.sensor[sidx++].inuse = true;
  }
}

/* 
 *=======================================================================================================================
 * bmp_info() - add to msg bmp sensors, Passing msg ptr, msg bubber sz, and pointer to comma
 *=======================================================================================================================
 */
void bmp_info(char *rest, int size, const char *&comma) {

  if (BMP_1_exists && (BMP_1_type!=BMP_TYPE_UNKNOWN)) {
    int used = strlen(rest);
    int n = sprintf(Buffer32Bytes, "%s%s", comma, bmptype[BMP_1_type]);
    // can we add with out over flowing?
    if ((used + n) < size) {
      sprintf(rest + used, "%s", Buffer32Bytes);
      comma = ",";
    }
    else {
      return;
    }
  }

  if (BMP_2_exists && (BMP_2_type!=BMP_TYPE_UNKNOWN)) {
    int used = strlen(rest);
    int n = sprintf(Buffer32Bytes, "%s%s", comma, bmptype[BMP_2_type]);
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
 * bmp_initialize() - Bosch sensor initialize
 *=======================================================================================================================
 */
void bmp_initialize() {
  Output("BMP:INIT");
  
  // 1st Bosch Sensor
  if (bmp390_probe(BMP390_ADDR_1)) {
    if (!bmp31.begin_I2C(BMP390_ADDR_1)) {
      Output ("BMP1 BMP390 ERR");
      BMP_1_exists = false;
    }
    else {
      Output ("BMP1 BMP390 OK");
      BMP_1_exists = true;
      BMP_1_type = BMP_TYPE_BMP390;
      [[maybe_unused]] float p = bmp31.readPressure();
    }
  }
  else if (bmp581_probe(BMP581_ADDR_1)) {
    if (!bmp51.begin(BMP581_ADDR_1, &Wire)) { 
      Output ("BMP1 BMP581 ERR");
      BMP_1_exists = false;
    }
    else {
      Output ("BMP1 BMP581 OK");
      BMP_1_exists = true;
      BMP_1_type = BMP_TYPE_BMP581;

      // Configure sensor for optimal performance
      bmp51.setTemperatureOversampling(BMP5XX_OVERSAMPLING_2X);
      bmp51.setPressureOversampling(BMP5XX_OVERSAMPLING_16X);
      bmp51.setIIRFilterCoeff(BMP5XX_IIR_FILTER_COEFF_3);
      bmp51.setOutputDataRate(BMP5XX_ODR_50_HZ);
      bmp51.setPowerMode(BMP5XX_POWERMODE_NORMAL);

      [[maybe_unused]] float p = bmp51.readPressure();  
    }
  }
  else {
    Output ("BMP1 NF"); 
  }      

  // 2nd Bosch Sensor
  if (bmp390_probe(BMP390_ADDR_2)) {
    if (!bmp32.begin_I2C(BMP390_ADDR_2)) {
      Output ("BMP2 BMP390 ERR");
      BMP_2_exists = false;
    }
    else {
      Output ("BMP2 BMP390 OK");
      BMP_2_exists = true;
      BMP_2_type = BMP_TYPE_BMP390;
      [[maybe_unused]] float p = bmp32.readPressure();
    }
  }
  else if (bmp581_probe(BMP581_ADDR_2)) {
    if (!bmp52.begin(BMP581_ADDR_1, &Wire)) { 
      Output ("BMP2 BMP581 ERR");
      BMP_2_exists = false;
    }
    else {
      Output ("BMP2 BMP581 OK");
      BMP_2_exists = true;
      BMP_2_type = BMP_TYPE_BMP581;

      // Configure sensor for optimal performance
      bmp52.setTemperatureOversampling(BMP5XX_OVERSAMPLING_2X);
      bmp52.setPressureOversampling(BMP5XX_OVERSAMPLING_16X);
      bmp52.setIIRFilterCoeff(BMP5XX_IIR_FILTER_COEFF_3);
      bmp52.setOutputDataRate(BMP5XX_ODR_50_HZ);
      bmp52.setPowerMode(BMP5XX_POWERMODE_NORMAL);

      [[maybe_unused]] float p = bmp52.readPressure();  
    }
  }
  else {
    Output ("BMP2 NF"); 
  }   
}