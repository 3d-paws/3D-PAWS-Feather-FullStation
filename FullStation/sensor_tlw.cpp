/*
 * ======================================================================================================================
 * sensor_tlw.cpp - Tinovi Leaf Wetness I2C Sensor
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/nvcf.h"
#include "include/obs.h"
#include "include/sensor_tlw.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Tinovi Leaf Wetness
 * ======================================================================================================================
 */
LeafSens tlw;
bool TLW_exists = false;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * tlw_obs_do() - Tinovi Leaf Wetness
 *=======================================================================================================================
 */
void tlw_obs_do(int &sidx) {
  if (TLW_exists) {
    tlw.newReading();
    delay(100);
    float w = tlw.getWet();
    float t = tlw.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    strcpy (obs.sensor[sidx].id, "tlww");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) w;
    obs.sensor[sidx++].inuse = true; 

    strcpy (obs.sensor[sidx].id, "tlwt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  }
}

/* 
 *=======================================================================================================================
 * tlw_info() - add to msg tlw sensor, Passing msg ptr, msg bubber sz, and pointer to comma
 *=======================================================================================================================
 */
void tlw_info(char *rest, int size, const char *&comma) {

  if (TLW_exists) {
    int used = strlen(rest);
    int n = sprintf (Buffer32Bytes, "%sTLW", comma);

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
  }
  Output (msgp);
}