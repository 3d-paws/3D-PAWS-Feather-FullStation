/*
 * ======================================================================================================================
 * sensor_bh1750.cpp - DFRobot SEN0562 - I2C - Ambient Light Sensor 1-65535lx - BH1750
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/nvcf.h"
#include "include/obs.h"
#include "include/sensor_bh1750.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Ambient Light Sensor LUX
 * ======================================================================================================================
 */
BH1750 bh1750_lux;
bool BH1750_exists = false;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * bh1750_obs_do() - add bh1750 to obs msg
 *=======================================================================================================================
 */
void bh1750_obs_do(int &sidx) {
  if (BH1750_exists) {
    // Light level in lux (0.0 ~ 54612,5 [117758,203]) Bracket is global max
  	// -1 : no valid return value
  	// -2 : sensor not configured 
    float lux=bh1750_lux.readLightLevel();
    lux = (isnan(lux) || (lux < QC_MIN_BHLX)  || (lux > QC_MAX_BHLX))  ? QC_ERR_BHLX  : lux;

    strcpy (obs.sensor[sidx].id, "bhlx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) lux;
    obs.sensor[sidx++].inuse = true;
  }
}

/* 
 *=======================================================================================================================
 * bh1750_info() - add bh1750 to info msg
 *=======================================================================================================================
 */
void bh1750_info(char *rest, int size, const char *&comma) {

  if (BH1750_exists) {
    int used = strlen(rest);
    int n = sprintf (Buffer32Bytes, "%sBHLX", comma);

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
 * bh1750_initialize() - initialize DFRobot SEN0562 Ambient Light Sensor 1-65535lx - BH1750 (via BH1750.h)
 *=======================================================================================================================
 */
void bh1750_initialize() {
  Output("BH1750:INIT");

  if (bh1750_lux.begin()) {
    BH1750_exists=true;
    msgp = (char *) "BH1750:OK";
  }
  else {
    BH1750_exists=false;
    msgp = (char *) "BH1750:NF";
  }
  Output (msgp);
}