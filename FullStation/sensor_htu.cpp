/*
 * ======================================================================================================================
 * sensor_htu.cpp - HTU21DF I2C Sensor
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/nvcf.h"
#include "include/obs.h"
#include "include/sensor_htu.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  HTU21D-F - I2C - Humidity & Temp Sensor
 * ======================================================================================================================
 */
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
bool HTU_exists = false;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * htu_obs_do() - 
 *=======================================================================================================================
 */
void htu_obs_do(int &sidx) {
  if (HTU_exists) {
    float t = 0.0;
    float h = 0.0;

    // HTU Temperature
    strcpy (obs.sensor[sidx].id, "ht1");
    obs.sensor[sidx].type = F_OBS;
    t = htu.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // HTU Humidity
    strcpy (obs.sensor[sidx].id, "hh1");
    obs.sensor[sidx].type = F_OBS;
    h = htu.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }
}

/* 
 *=======================================================================================================================
 * htu_info() - add to msg htu sensor, Passing msg ptr, msg bubber sz, and pointer to comma
 *=======================================================================================================================
 */
void htu_info(char *rest, int size, const char *&comma) {

  if (HTU_exists) {
    int used = strlen(rest);
    int n = sprintf (Buffer32Bytes, "%sHTU", comma);

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
 * htu_initialize() - HTU21D sensor initialize
 *=======================================================================================================================
 */
void htu_initialize() {
  Output("HTU:INIT");
  
  if (!htu.begin()) {
    msgp = (char *) "HTU NF";
    HTU_exists = false;
  }
  else {
    HTU_exists = true;
    msgp = (char *) "HTU OK";
  }
  Output (msgp);
}