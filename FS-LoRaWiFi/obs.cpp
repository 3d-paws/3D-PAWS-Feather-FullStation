/*
 * ======================================================================================================================
 *  obs.cpp - Observation Reporting Functions
 * ======================================================================================================================
 */

#include "include/qc.h"
#include "include/ssbits.h"
#include "include/feather.h"
#include "include/eeprom.h"
#include "include/wifi.h"
#include "include/wrda.h"
#include "include/cf.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/mux.h"
#include "include/dsmux.h"
#include "include/lorawan.h"
#include "include/support.h"
#include "include/time.h"
#include "include/sensors_i2c_44_47.h"
#include "include/sensors.h"
#include "include/main.h"
#include "include/obs.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
OBSERVATION_STR obs;
char obsbuf[MAX_OBS_SIZE];      // Url that holds observations for HTTP GET
char *obsp;                     // Pointer to obsbuf
float bmx_1_pressure = 0.0;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * OBS_Clear() - Set OBS to not in use
 * ======================================================================================================================
 */
void OBS_Clear() {
  obs.inuse =false;
  for (int s=0; s<MAX_SENSORS; s++) {
    obs.sensor[s].inuse = false;
  }
}

/*
 * ======================================================================================================================
 * OBS_N2S_Add_WiFi() - Save OBS to N2S file
 * ======================================================================================================================
 */
void OBS_N2S_Add_WiFi() {
  if (obs.inuse) {     // Sanity check
    char ts[32];
   
    memset(obsbuf, 0, sizeof(obsbuf));

    // Modify System Status and Set From Need to Send file bit
    obs.hth |= SSB_FROM_N2S; // Turn On Bit

    tm *dt = gmtime(&obs.ts);
    sprintf (obsbuf, "%s?key=%s&instrument_id=%d&",
      cf_urlpath, cf_apikey, cf_instrument_id);
    sprintf (obsbuf+strlen(obsbuf), "devid=%s&at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
      DeviceID,
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
    sprintf (obsbuf+strlen(obsbuf), "&bv=%.2f",obs.bv); 
    sprintf (obsbuf+strlen(obsbuf), "&hth=%d", obs.hth);
   
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }
    Serial_writeln (obsbuf);
    SD_NeedToSend_Add(obsbuf); // Save to N2F File
    Output("OBS-> N2S");
  }
  else {
    Output("OBS->N2S OBS:Empty");
  }
}

/*
 * ======================================================================================================================
 * OBS_LOG_Add() - Create observation in obsbuf and save to SD card.
 * 
 * {"at":"2022-02-13T17:26:07","css":18,"hth":0,"bcs":2,"bpc":63.2695,.....,"mt2":20.5625}
 * ======================================================================================================================
 */
void OBS_LOG_Add() {
  Output("OBS_LOG_ADD()");
    
  if (obs.inuse) {     // Sanity check

    memset(obsbuf, 0, sizeof(obsbuf));

    // Save the Observation in JSON format
    
    sprintf (obsbuf, "{");

    tm *dt = gmtime(&obs.ts); 
    
    sprintf (obsbuf+strlen(obsbuf), "\"at\":\"%d-%02d-%02dT%02d:%02d:%02d\"",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
      
    sprintf (obsbuf+strlen(obsbuf), ",\"bv\":%.2f", obs.bv); 
    sprintf (obsbuf+strlen(obsbuf), ",\"hth\":%d", obs.hth);
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }
    sprintf (obsbuf+strlen(obsbuf), "}");
    
    Output("OBS->SD");
    Serial_writeln (obsbuf);
    SD_LogObservation(obsbuf); 
  }
  else {
    Output("OBS->SD OBS:Empty");
  }
}

/*
 * ======================================================================================================================
 * OBS_Take() - Take Observations - Should be called once a minute - fill data structure
 * ======================================================================================================================
 */
void OBS_Take() {
  int sidx = 0;
  float rg1 = 0.0;
  float rg2 = 0.0;
  unsigned long rg1ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  unsigned long rg2ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  float ws = 0.0;
  int wd = 0;
  float mcp3_temp = 0.0;  // globe temperature
  float wetbulb_temp = 0.0;
  float heat_index = 0.0;

  Output("OBS_TAKE()");
  
  // Safty Check for Vaild Time
  if (!RTC_valid) {
    Output ("OBS_Take: TM Invalid");
    return;
  }
  
  OBS_Clear(); // Just do it again as a safty check

  Wind_GustUpdate(); // Update Gust and Gust Direction readings

  obs.inuse = true;
  // obs.ts = rtc_unixtime();
  obs.ts = (time_t) Time_of_obs;

  obs.bv = vbat_get();

  if (!AQS_Enabled) {
    // Rain Gauge 1 - Each tip is 0.2mm of rain
    if (cf_rg1_enable) {
      rg1 = raingauge1_sample();
    }

    // Rain Gauge 2 - Each tip is 0.2mm of rain
    if (cf_op1 == OP1_STATE_RAIN) {
      rg2 = raingauge2_sample();
    }

    if (RainEnabled()) {
      EEPROM_UpdateRainTotals(rg1, rg2);
    }
 
    // Rain Gauge 1
    if (cf_rg1_enable) {
      strcpy (obs.sensor[sidx].id, "rg1");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = rg1;
      obs.sensor[sidx++].inuse = true;
  
      strcpy (obs.sensor[sidx].id, "rgt1");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = eeprom.rgt1;
      obs.sensor[sidx++].inuse = true;

      strcpy (obs.sensor[sidx].id, "rgp1");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = eeprom.rgp1;
      obs.sensor[sidx++].inuse = true;
    }

    // Rain Gauge 2
    if (cf_op1 == OP1_STATE_RAIN) {
      strcpy (obs.sensor[sidx].id, "rg2");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = rg2;
      obs.sensor[sidx++].inuse = true;
  
      strcpy (obs.sensor[sidx].id, "rgt2");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = eeprom.rgt2;
      obs.sensor[sidx++].inuse = true;

      strcpy (obs.sensor[sidx].id, "rgp2");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = eeprom.rgp2;
      obs.sensor[sidx++].inuse = true;
    }

    if (cf_op1 == OP1_STATE_RAW) {
      // OP1 Raw
      strcpy (obs.sensor[sidx].id, "op1r");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = Pin_ReadAvg(OP1_PIN);
      obs.sensor[sidx++].inuse = true;
    }

    if ((cf_op1 == OP1_STATE_DIST_5M) || (cf_op1 == OP1_STATE_DIST_10M)) {
      float ds_median, ds_median_raw;
    
      ds_median = ds_median_raw = DS_Median();
      if (cf_ds_baseline > 0) {
        ds_median = cf_ds_baseline - ds_median_raw;
      }

      strcpy (obs.sensor[sidx].id, "ds");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = ds_median;
      obs.sensor[sidx++].inuse = true;

      strcpy (obs.sensor[sidx].id, "dsr");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = ds_median_raw;
      obs.sensor[sidx++].inuse = true;
    }

    if (cf_op2 == OP2_STATE_RAW) {
      // OP2 Raw
      strcpy (obs.sensor[sidx].id, "op2r");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = Pin_ReadAvg(OP2_PIN);
      obs.sensor[sidx++].inuse = true;
    }

    if (cf_op2 == OP2_STATE_VOLTAIC) {
      // OP2 Voltaic Battery Voltage
      float vbv = VoltaicVoltage(OP2_PIN);
      strcpy (obs.sensor[sidx].id, "vbv");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = vbv;
      obs.sensor[sidx++].inuse = true;

      strcpy (obs.sensor[sidx].id, "vpc");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = VoltaicPercent(vbv);
      obs.sensor[sidx++].inuse = true;
    }

    if (!cf_nowind) {
      // Wind Speed
      ws = Wind_SpeedAverage();
      ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
      strcpy (obs.sensor[sidx].id, "ws");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = ws;
      obs.sensor[sidx++].inuse = true;

      // Wind Direction
      wd = Wind_DirectionVector();
      wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
      strcpy (obs.sensor[sidx].id, "wd");
      obs.sensor[sidx].type = I_OBS;
      obs.sensor[sidx].i_obs = Wind_DirectionVector();
      obs.sensor[sidx++].inuse = true;

      // Wind Gust
      ws = Wind_Gust();
      ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
      strcpy (obs.sensor[sidx].id, "wg");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = ws;
      obs.sensor[sidx++].inuse = true;

      // Wind Gust Direction (Global)
      wd = Wind_GustDirection();
      wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
      strcpy (obs.sensor[sidx].id, "wgd");
      obs.sensor[sidx].type = I_OBS;
      obs.sensor[sidx].i_obs = wd;
      obs.sensor[sidx++].inuse = true;
    }
  }

  // Do AQ before others since we have a 30s wakeup time when we are in AQS mode
  if (PM25AQI_exists) {
    // Air Quality station, wake up sensor, wait 30s use 2nd reading, put sensor to sleep
    if (AQS_Enabled) {
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

  if (BMX_1_exists) {
    float p,t,h;
    bmx1_read(p, t, h);
    
    // BMX1 Preasure
    strcpy (obs.sensor[sidx].id, "bp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = p;
    obs.sensor[sidx++].inuse = true;

    // BMX1 Temperature
    strcpy (obs.sensor[sidx].id, "bt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // BMX1 Humidity
    if (BMX_1_type == BMX_TYPE_BME280) {
      strcpy (obs.sensor[sidx].id, "bh1");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = h;
      obs.sensor[sidx++].inuse = true;
    }

    bmx_1_pressure = p; // Used later for mslp calc
  }
  
  if (BMX_2_exists) {
    float p,t,h;
    bmx2_read(p, t, h);

    // BMX2 Preasure
    strcpy (obs.sensor[sidx].id, "bp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = p;
    obs.sensor[sidx++].inuse = true;

    // BMX2 Temperature
    strcpy (obs.sensor[sidx].id, "bt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // BMX2 Humidity
    if (BMX_2_type == BMX_TYPE_BME280) {
      strcpy (obs.sensor[sidx].id, "bh2");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = h;
      obs.sensor[sidx++].inuse = true;
    }
  }

  // Do Sensor observations for SHT31, SHT45, BMP581, HDC302x
  sensor_i2c_44_47_obs_do(sidx);      
  
  if (HTU21DF_exists) {
    float t = 0.0;
    float h = 0.0;
    
    // HTU Humidity
    strcpy (obs.sensor[sidx].id, "hh1");
    obs.sensor[sidx].type = F_OBS;
    h = htu.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;

    // HTU Temperature
    strcpy (obs.sensor[sidx].id, "ht1");
    obs.sensor[sidx].type = F_OBS;
    t = htu.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (LPS_1_exists) {
    float t = lps1.readTemperature();
    float p = lps1.readPressure();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;

    // LPS1 Temperature
    strcpy (obs.sensor[sidx].id, "lpt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // LPS1 Pressure
    strcpy (obs.sensor[sidx].id, "lpp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) p;
    obs.sensor[sidx++].inuse = true;
  }

  if (LPS_2_exists) {
    float t = lps2.readTemperature();
    float p = lps2.readPressure();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;

    // LPS1 Temperature
    strcpy (obs.sensor[sidx].id, "lpt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // LPS1 Pressure
    strcpy (obs.sensor[sidx].id, "lpp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) p;
    obs.sensor[sidx++].inuse = true;
  }

  if (HIH8_exists) {
    float t = 0.0;
    float h = 0.0;
    bool status = hih8_getTempHumid(&t, &h);
    if (!status) {
      t = -999.99;
      h = 0.0;
    }
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;

    // HIH8 Temperature
    strcpy (obs.sensor[sidx].id, "ht2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // HIH8 Humidity
    strcpy (obs.sensor[sidx].id, "hh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }
  
  if (SI1145_exists) {
    float si_vis = uv.readVisible();
    float si_ir = uv.readIR();
    float si_uv = uv.readUV()/100.0;

    // Additional code to force sensor online if we are getting 0.0s back.
    if ( ((si_vis+si_ir+si_uv) == 0.0) && ((si_last_vis+si_last_ir+si_last_uv) != 0.0) ) {
      // Let Reset The SI1145 and try again
      Output ("SI RESET");
      if (uv.begin()) {
        SI1145_exists = true;
        Output ("SI ONLINE");
        si_vis = uv.readVisible();
        si_ir = uv.readIR();
        si_uv = uv.readUV()/100.0;
      }
      else {
        SI1145_exists = false;
        Output ("SI OFFLINE");
      }
    }

    // Save current readings for next loop around compare
    si_last_vis = si_vis;
    si_last_ir = si_ir;
    si_last_uv = si_uv;

    // QC Checks
    si_vis = (isnan(si_vis) || (si_vis < QC_MIN_VI)  || (si_vis > QC_MAX_VI)) ? QC_ERR_VI  : si_vis;
    si_ir  = (isnan(si_ir)  || (si_ir  < QC_MIN_IR)  || (si_ir  > QC_MAX_IR)) ? QC_ERR_IR  : si_ir;
    si_uv  = (isnan(si_uv)  || (si_uv  < QC_MIN_UV)  || (si_uv  > QC_MAX_UV)) ? QC_ERR_UV  : si_uv;

    // SI Visible
    strcpy (obs.sensor[sidx].id, "sv1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = si_vis;
    obs.sensor[sidx++].inuse = true;

    // SI IR
    strcpy (obs.sensor[sidx].id, "si1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = si_ir;
    obs.sensor[sidx++].inuse = true;

    // SI UV
    strcpy (obs.sensor[sidx].id, "su1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = si_uv;
    obs.sensor[sidx++].inuse = true;
  }
    
  if (MCP_1_exists) {
    float t = 0.0;
   
    // MCP1 Temperature
    strcpy (obs.sensor[sidx].id, "mt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp1.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_2_exists) {
    float t = 0.0;
    
    // MCP2 Temperature
    strcpy (obs.sensor[sidx].id, "mt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp2.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_3_exists) {
    float t = 0.0;

    // MCP3 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp3.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    mcp3_temp = t; // globe temperature
  }

  if (MCP_4_exists) {
    float t = 0.0;

    // MCP4 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp4.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (BLX_exists) {
    float lux=blx_takereading();
    lux = (isnan(lux) || (lux < QC_MIN_BLX)  || (lux > QC_MAX_BLX))  ? QC_ERR_BLX  : lux;

    // DFR BLUX30 Auto Lux Value
    strcpy (obs.sensor[sidx].id, "blx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = lux;
    obs.sensor[sidx++].inuse = true;
  }

  // Heat Index Temperature
  if (HI_exists) {
    heat_index = hi_calculate(sht1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "hi");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) heat_index;
    obs.sensor[sidx++].inuse = true;    
  }
  
  // Wet Bulb Temperature
  if (WBT_exists) {
    strcpy (obs.sensor[sidx].id, "wbt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbt_calculate(sht1_temp, sht1_humid);
    obs.sensor[sidx++].inuse = true;    
  }

  // Wet Bulb Globe Temperature
  if (WBGT_exists) {
    float wbgt = 0.0;
    if (MCP_3_exists) {
      wbgt = wbgt_using_wbt(sht1_temp, mcp3_temp, wetbulb_temp); // TempAir, TempGlobe, TempWetBulb
    }
    else {
      wbgt = wbgt_using_hi(heat_index);
    }
    
    strcpy (obs.sensor[sidx].id, "wbgt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbgt;
    obs.sensor[sidx++].inuse = true;    
  }

  if (MSLP_exists) {
    float mslp = (float) mslp_calculate(sht1_temp, sht1_humid, bmx_1_pressure, cf_elevation);
    strcpy (obs.sensor[sidx].id, "mslp");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) mslp;
    obs.sensor[sidx++].inuse = true;  
  }

  // Tinovi Leaf Wetness
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

  // Tinovi Soil Moisture
  mux_obs_do(sidx);

  // Dallas Sensors Temperature on mux
  dsmux_obs_do(sidx);

  // Set this after we read all sensors. So we capture if their state changes
  obs.hth = SystemStatusBits;

  Output("OBS_TAKE(DONE)");
}

/*
 * ======================================================================================================================
 * OBS_Build_WiFi() - Create observation in obsbuf from structure obs for sending
 * 
 * Example at=2022-05-17T17%3A40%3A04&hth=8770 .....
 * ======================================================================================================================
 */
bool OBS_Build_WiFi() {
  Output("OBSBLDWIFI()"); 
  if (obs.inuse) {     // Sanity check  
    memset(obsbuf, 0, sizeof(obsbuf));
    tm *dt = gmtime(&obs.ts);

    sprintf (obsbuf, "%s?key=%s&instrument_id=%d&devid=%s",
      cf_urlpath, cf_apikey, cf_instrument_id, DeviceID);

    sprintf (obsbuf+strlen(obsbuf), "at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
    sprintf (obsbuf+strlen(obsbuf), "&bv=%.2f", obs.bv); 
    sprintf (obsbuf+strlen(obsbuf), "&hth=%d", 
      obs.hth);  

    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }

    Output("OBSBLDWIFI:OK");
    Serial_writeln (obsbuf);
    return (true);
  }
  else {
    Output("OBSBLDWIFI:NOTINUSE");
    return (false);
  }
}

/*
 * ======================================================================================================================
 * OBS_Send_LoRaWAN() - From obs structure build and send 1 or more LoRaWAN packets as needed
 * ======================================================================================================================
 */
void OBS_Send_LoRaWAN() {
  char header[32];
  char hthbits[32];
  char loramsg[256];
  char sensor[16];
  int count=0;
  bool checkN2S=true;
  int sensor_payload;  // LORA_PAYLOAD Buffer Size

  Output("OBS_Send_LW)");
  if (obs.inuse) {                           // Sanity check set by OBS_Take()
    tm *dt = gmtime(&obs.ts);                // OBS_Take() has already obtained the time

    memset(header, 0, sizeof(header));       // Header is included in every lora message
    memset(loramsg, 0, sizeof(loramsg));
    memset(hthbits, 0, sizeof(hthbits));
    memset(obsbuf, 0, sizeof(obsbuf));       // This will be the lora payload we send

    // Consider adding health to header so we can set the from N2S bit.
    
    // Header is included in every lora message
    sprintf (header, "at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",         // 23 bytes
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
    sprintf (hthbits, "&hth=%d", obs.hth);                           // Call it 9 bytes

    sensor_payload = LORA_PAYLOAD - (strlen (header) + strlen (hthbits));

    sprintf (loramsg, "bv=%.2f", obs.bv); 
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        // sprintf (Buffer32Bytes, "PROCESSOBS=%d", s);
        // Output(Buffer32Bytes); 
        count++;  
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (sensor, "&%s=%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (sensor, "&%s=%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (sensor, "&%s=%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
        
        // Will this Sensor observation fit into loramsg - This is how we do not overflow the 256 byte loramsg structure
        if ( (strlen(sensor) + strlen(loramsg)) <= sensor_payload) {
          // Add the sensor to the sensors
          sprintf (loramsg+strlen(loramsg), "%s", sensor);
        }
        else {       
          Output("OBS_Send_LW:SENDING");
          sprintf (obsbuf, "%s%s%s", header, hthbits, loramsg); // Put the parts together and send
          Output (obsbuf);
          if (LW_Send(obsbuf)) {
            Output("OBS_Send_LW:SENT");
          }
          else {
            Output("OBS_Send_LW:PUB FAILED");
            sprintf (hthbits, "&hth=%d", (obs.hth | SSB_FROM_N2S)); // Turn On Bit

            sprintf (obsbuf, "%s%s%s", header, hthbits, loramsg);
            Output (obsbuf);
            SD_NeedToSend_Add(obsbuf);
            checkN2S = false;
          }

          // Success or fail we continue on trying to send the rest of the observations

          delay(500); // Its Recommended before sending another message

          // Clear sensors and add the one that would not fit along with the header
          count = 1;
          memset(loramsg, 0, sizeof(loramsg));
          memset(obsbuf, 0, sizeof(obsbuf));
        }
      }
      else {
        // sprintf (Buffer32Bytes, "NOT INUSE=%d", s);
        // Output(Buffer32Bytes);          
      }
    } // for

    // Send remainding obs if any
    if (count) {
      Output("OBS_Send_LW:SENDING-LAST");
      sprintf (obsbuf, "%s%s%s", header, hthbits, loramsg); // Put the parts together and send
      Output (obsbuf);

      if (LW_Send(obsbuf)) {
        Output("OBS_Send_LW:SENT");
      }
      else {
        Output("OBS_Send_LW:PUB FAILED");
        sprintf (hthbits, "&hth=%d", (obs.hth | SSB_FROM_N2S)); // Turn On Bit

        sprintf (obsbuf, "%s%s%s", header, hthbits, loramsg);
        Output (obsbuf);
        SD_NeedToSend_Add(obsbuf);
        checkN2S = false;
      }
    }
    
    OBS_Clear(); 

    if (checkN2S) {
      // Check if we have any N2S and if we have not added to the file while trying to send OBS, check
      SD_N2S_Publish();     
    }
  }
  else {
    Output("OBS_Send_LW:NOTINUSE");
  }
}

/*
 * ======================================================================================================================
 * OBS_Do() - Do Observation Processing, 222 bytes max for LoRaWAN
 * ======================================================================================================================
 */
void OBS_Do() {
  Output("OBS_DO()");

  OBS_Take();          // Take an observation

  // At this point, the obs data structure has been filled in with observation data
  OBS_LOG_Add();        // Save Observation Data to Log file.

  // Handle LoRaWAN
  if (LW_valid) {
    OBS_Send_LoRaWAN();
  }
  else if (WiFi_valid) {
    if (OBS_Build_WiFi()) {  // Build Observation to Send in obsbuf
      // WiFi_Send() returns 0=not sent, -500=ErrorCode Not Sent, 1=Sent
      if (WiFi_Send(obsbuf, cf_info_server, cf_info_server_port, cf_info_urlpath, METHOD_POST, cf_info_apikey) != 1) {
        Output("FS->PUB FAILED");
        OBS_N2S_Add_WiFi();
        OBS_Clear();
      }
      else {
        Output("FS->PUB OK");
        OBS_Clear();
        // Check if we have any N2S only if we have not added to the file while trying to send OBS
        SD_N2S_Publish(); 
      }
    }
  }
  else {
    Output("No Valid Network");
  }
}