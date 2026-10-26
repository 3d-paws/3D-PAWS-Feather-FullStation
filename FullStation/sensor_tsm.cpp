/*
 * ======================================================================================================================
 * sensor_tsm.cpp - Tinovi Soil Moisture I2C Sensor
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/nvcf.h"
#include "include/obs.h"
#include "include/sensor_tsm.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Tinovi Soil Moisture (Soil and Temperature)
 * ======================================================================================================================
 */
SVCS3 tsm;
bool TSM_exists = false;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * tsm_obs_do() - Tinovi Soil Moisture
 *=======================================================================================================================
 */
void tsm_obs_do(int &sidx) {
  if (TSM_exists) {
    tsm.newReading();
    delay(100);
    float e25 = tsm.getE25();
    float ec = tsm.getEC();
    float vwc = tsm.getVWC();
    float t = tsm.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    strcpy (obs.sensor[sidx].id, "tsme25");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) e25;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tsmec");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) ec;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tsmvwc");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) vwc;
    obs.sensor[sidx++].inuse = true; 

    strcpy (obs.sensor[sidx].id, "tsmt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  }
}

/* 
 *=======================================================================================================================
 * tsm_info() - add to msg tlw sensor, Passing msg ptr, msg bubber sz, and pointer to comma
 *=======================================================================================================================
 */
void tsm_info(char *rest, int size, const char *&comma) {

  if (TSM_exists) {
    int used = strlen(rest);
    int n = sprintf (Buffer32Bytes, "%sTSM", comma);

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
  }
  Output (msgp);
}