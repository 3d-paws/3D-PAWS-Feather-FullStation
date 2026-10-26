/*
 * ======================================================================================================================
 *  sensor_pm25aqi.h - PM25AQI I2C Sensor
 * ======================================================================================================================
 */
#include "include/output.h"
#include "include/support.h"
#include "include/time.h"
#include "include/main.h"
#include "include/sensor_rgop.h"
#include "include/nvcf.h"
#include "include/obs.h"
#include "include/sensor_pm25aqi.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  PM25AQI - I2C - Air Quality Sensor
 * ======================================================================================================================
 */
PM25AQI_OBS pm25aqi_1m_obs[PM25AQI_1M_BUCKETS];
int pm25aqi_1m_bucket = 0;

PM25AQI_OBS_STR pm25aqi_obs;
Adafruit_PM25AQI pmaq = Adafruit_PM25AQI();
bool PM25AQI_exists = false;

int AQSWarmUpTime = 35;                 // Seconds to wait wile sensor warms up from sleep
int AQS_Correction = 0;                 // This will set to 30500ms for time we wait for sensor to initialize


/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

 /* 
 *=======================================================================================================================
 * pm25aqi_1m_clear() - clear 1m circular observation buffer
 *=======================================================================================================================
 */
void pm25aqi_1m_clear() {
  for (int i=0; i<PM25AQI_1M_BUCKETS; i++) {
    pm25aqi_1m_obs[i].e10 = 0;
    pm25aqi_1m_obs[i].e25 = 0;
    pm25aqi_1m_obs[i].e100 = 0;
  }
}

/* 
 *=======================================================================================================================
 * pm25aqi_clear() - clear observation
 *=======================================================================================================================
 */
void pm25aqi_clear() {
  pm25aqi_obs.e10 = 0;
  pm25aqi_obs.e25 = 0;
  pm25aqi_obs.e100 = 0;
  pm25aqi_obs.count = 0;
  pm25aqi_obs.fail_count = 0;
}

/* 
 *=======================================================================================================================
 * pm25aqi_Produce_1m_Average() - Sum samples and provide and average for reporting
 *=======================================================================================================================
 */
void pm25aqi_Produce_1m_Average() {
  if (PM25AQI_exists) {
    pm25aqi_clear();
    for (int i=0; i<PM25AQI_1M_BUCKETS; i++) {
      pm25aqi_obs.e10  += pm25aqi_1m_obs[i].e10;
      pm25aqi_obs.e25  += pm25aqi_1m_obs[i].e25;
      pm25aqi_obs.e100 += pm25aqi_1m_obs[i].e100;
    }
    // Do average
    pm25aqi_obs.e10  = (pm25aqi_obs.e10  / PM25AQI_1M_BUCKETS);
    pm25aqi_obs.e25  = (pm25aqi_obs.e25  / PM25AQI_1M_BUCKETS);
    pm25aqi_obs.e100 = (pm25aqi_obs.e100 / PM25AQI_1M_BUCKETS); 
  }
}

/* 
 *=======================================================================================================================
 * pm25aqi_TakeReading() - Take a reading and sum it for later average when reporting
 *=======================================================================================================================
 */
void pm25aqi_TakeReading() {
  if (PM25AQI_exists) {
    PM25_AQI_Data aqid;
  
    if (pmaq.read(&aqid)) {
      pm25aqi_1m_obs[pm25aqi_1m_bucket].e10  = aqid.pm10_env;
      pm25aqi_1m_obs[pm25aqi_1m_bucket].e25  = aqid.pm25_env;
      pm25aqi_1m_obs[pm25aqi_1m_bucket].e100 = aqid.pm100_env;
      pm25aqi_1m_bucket = (pm25aqi_1m_bucket+1) % PM25AQI_1M_BUCKETS; // Advance bucket index for next reading
    }
  }
}

/* 
 *=======================================================================================================================
 * pm25aqi_TakeReading_AQS() - Air Quality station, wake up sensor, wait 30s, skip 1st reading, take sample, put sensor to sleep
 *=======================================================================================================================
 */
void pm25aqi_TakeReading_AQS() {
  if (PM25AQI_exists) {
    PM25_AQI_Data aqid;

    Output("AQS:WAKEUP");
    digitalWrite(OP2_PIN, HIGH); // Wakeup Air Quality Sensor

    // Clear readings
    pm25aqi_clear();

    // Wait 30s for sensor to wakeup
    for (int i=0; i<AQSWarmUpTime; i++) {
      BackGroundWork(); // Delays 1s
      if (SerialConsoleEnabled) Serial.print(".");  // Provide Serial Console some feedback as we loop and wait til next observation
      OLED_spin();
    }
    if (SerialConsoleEnabled) Serial.println();

    Output("AQS:Take Reading");
    
    pmaq.read(&aqid); // Toss 1st reading after wakeup
    
    for (int i=0; i<11; i++) {
      delay(800); // sensor takes reading every 1s, so wait for the next
      if (pmaq.read(&aqid)) {
        pm25aqi_obs.count++;
        pm25aqi_obs.e10  += aqid.pm10_env;
        pm25aqi_obs.e25  += aqid.pm25_env;
        pm25aqi_obs.e100 += aqid.pm100_env;
      }
      else {
        pm25aqi_obs.fail_count++;
      }
    }
    Output("AQS:SLEEP");
    digitalWrite(OP2_PIN, LOW); // Put to Sleep Air Quality Sensor

    if (pm25aqi_obs.fail_count > pm25aqi_obs.count) {
      // Fail if half our sample reads failed. - I think this is reasonable - rjb
      Output("AQS:FAIL");
      pm25aqi_obs.e10 = -999;
      pm25aqi_obs.e25 = -999;
      pm25aqi_obs.e100 = -999;
    }
    else {
      // Do average
      Output("AQS:OK");
      pm25aqi_obs.e10  = (pm25aqi_obs.e10 / pm25aqi_obs.count);
      pm25aqi_obs.e25  = (pm25aqi_obs.e25 / pm25aqi_obs.count);
      pm25aqi_obs.e100 = (pm25aqi_obs.e100 / pm25aqi_obs.count); 
    }
  }
}

/* 
 *=======================================================================================================================
 * pm25aqi_obs_do() - 
 *=======================================================================================================================
 */
void pm25aqi_obs_do(int &sidx) {
  // Do AQ before others since we have a 30s wakeup time when we are in AQS mode
  if (PM25AQI_exists) {
    // Air Quality station, wake up sensor, wait 30s use 2nd reading, put sensor to sleep
    if (scv.aqs) {
      pm25aqi_TakeReading_AQS(); // This does Over Sampling and sleep management
      obs.ts = (time_t) rtc_unixtime(); // Get time after the above wakeup and reading of sensor.
    }
    else {
      pm25aqi_Produce_1m_Average();
    }

    // Atmospheric Environmental PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.e10;
    obs.sensor[sidx++].inuse = true;

    // Atmospheric Environmental PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.e25;
    obs.sensor[sidx++].inuse = true;

    // Atmospheric Environmental PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.e100;
    obs.sensor[sidx++].inuse = true;

    // Clear readings
    pm25aqi_clear();
  }
}

/* 
 *=======================================================================================================================
 * pm25aqi_info() - add to msg htu sensor, Passing msg ptr, msg bubber sz, and pointer to comma
 *=======================================================================================================================
 */
void pm25aqi_info(char *rest, int size, const char *&comma) {

  if (PM25AQI_exists) {
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
 * pm25aqi_initialize() - air quality sensor
 *=======================================================================================================================
 */
void pm25aqi_initialize() {
  Output("PM25AQI:INIT");
  Wire.beginTransmission(PM25AQI_ADDRESS);
  if (Wire.endTransmission()) {
    msgp = (char *) "PM:NF";
    PM25AQI_exists = false;
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
      pm25aqi_1m_clear();
    }
  }
  Output (msgp);
}