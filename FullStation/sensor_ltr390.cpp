/*
 * ======================================================================================================================
 * sensor_ltr390.cpp - Adafruit LTR390 UV Light Sensor - I2C
 * ======================================================================================================================
 */
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/nvcf.h"
#include "include/obs.h"
#include "include/sensor_ltr390.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Adafruit LTR390 UV Light Sensor
 * ======================================================================================================================
 */
bool LTR390_exists;
Adafruit_LTR390 ltr390; 

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * ltr390_obs_do() - add ltr390 to obs msg
 *=======================================================================================================================
 */
void ltr390_obs_do(int &sidx) {
  if (LTR390_exists) {
    const uint32_t uv_counts=ltr390.readUVS();
    // No QC check. Rtn vaalue will be 0 through 1,048,575 counts

    // LTR390 Count
    strcpy (obs.sensor[sidx].id, "ltruvc");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = uv_counts;
    obs.sensor[sidx++].inuse = true;

    // LTR390 UVI (UV Index)
    strcpy (obs.sensor[sidx].id, "ltruvi");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = static_cast<float>(uv_counts) / 95.83f;
    obs.sensor[sidx++].inuse = true;
  }
}

/* 
 *=======================================================================================================================
 * ltr390_info() - add ltr390 to info msg
 *=======================================================================================================================
 */
void ltr390_info(char *rest, int size, const char *&comma) {

  if (LTR390_exists) {
    int used = strlen(rest);
    int n = sprintf (Buffer32Bytes, "%sLTR390", comma);

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
 * ltr390_detail() - display LTR390 details
 *=======================================================================================================================
 */
void ltr390_detail() {

  Output ("LTR Information");

  if (ltr390.getMode() == LTR390_MODE_ALS) {
    Output(" In ALS mode"); // default
  } else {
    Output(" In UVS mode");
  }

  switch (ltr390.getGain()) {
    case LTR390_GAIN_1:  Output (" GAIN: 1"); break;
    case LTR390_GAIN_3:  Output (" GAIN: 3"); break;
    case LTR390_GAIN_6:  Output (" GAIN: 6"); break;
    case LTR390_GAIN_9:  Output (" GAIN: 9"); break;
    case LTR390_GAIN_18: Output (" GAIN: 18"); break;
  }

  switch (ltr390.getResolution()) {
    case LTR390_RESOLUTION_13BIT: Output (" Resolution: 13"); break;
    case LTR390_RESOLUTION_16BIT: Output (" Resolution: 16"); break; 
    case LTR390_RESOLUTION_17BIT: Output (" Resolution: 17"); break;
    case LTR390_RESOLUTION_18BIT: Output (" Resolution: 18"); break;
    case LTR390_RESOLUTION_19BIT: Output (" Resolution: 19"); break;
    case LTR390_RESOLUTION_20BIT: Output (" Resolution: 20"); break;
  }
}

/* 
 *=======================================================================================================================
 * ltr390_initialize() - initialize Adafruit LTR390 UV Light Sensor
 *=======================================================================================================================
 */
void ltr390_initialize() {
  Output ("LTR:INIT");

  if (!ltr390.begin()) { 
    Output ("LTR:NF");
  }
  else {
    LTR390_exists=true;
    Output ("LTR:OK");

    // LTR390_MODE_UVS and LTR390_MODE_ALS are two operational modes for the LTR390 sensor, each targeting a different 
    // type of light measurement. LTR390_MODE_UVS configures the sensor to measure ultraviolet (UV) light, specifically 
    // in the UVA range (around 300–350 nm), which is useful for UV index calculations and monitoring UV exposure. 
    // LTR390_MODE_ALS, on the other hand, sets the sensor to measure ambient light (ALS), which is in the visible 
    // spectrum (typically 500–600 nm), similar to how the human eye perceives light.
    ltr390.setMode(LTR390_MODE_UVS); // default ALS mode

    ltr390.setGain(LTR390_GAIN_3); // default 3 - Higher gain values increase sensitivity but may introduce more noise.

    ltr390.setResolution(LTR390_RESOLUTION_18BIT); // default 16 Bits - balances measurement precision and speed.
                                                // Keeing Gain at 3
                                                // Set 16 for UVI (UV Index) = UVS counts / 23.96 conversion
                                                // Set 18 for UVI (UV Index) = UVS counts / 95.83 conversion
                                                // Set 20 for UVI (UV Index) = UVS counts / 383.22 conversion

    // ltr390.setThresholds(100, 1000); // disabled - defines the lower and upper thresholds for the interrupt. 
                                     // When the UV reading drops below 100 or exceeds 1000, the interrupt will trigger.

    ltr390.configInterrupt(false, LTR390_MODE_UVS); // disabled - true would enable the interrupt

    ltr390_detail();
  }
}