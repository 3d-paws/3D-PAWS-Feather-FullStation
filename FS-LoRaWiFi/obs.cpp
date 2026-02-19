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
#include "include/lorawan.h"
#include "include/support.h"
#include "include/time.h"
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

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * SendMsg() - Log Observation or Information, return result status
 * ======================================================================================================================
 */
int SendMsg(char *msg)
{
  // Handle LoRaWAN
  if (LW_valid) {
    if (LW_Send(msg)) {
      return(1);
    }
    else {
      return(0);
    }
  }

  // Handle WiFi can retutn 0=not sent, -500=ErrorCode Not Sent, 1=Sent
  else if (WiFi_valid) {
    // Determine the type of message and which server to send to INFO or OBS
    if (strstr(msg, "\"MT\":\"INFO\"") != NULL) {
      return (WiFi_Send(msg, cf_info_server, cf_info_server_port, cf_info_urlpath, METHOD_POST, cf_info_apikey));
    }
    else {
      return (WiFi_Send(msg, cf_webserver, cf_webserver_port, cf_urlpath, METHOD_GET, cf_apikey));
    }
  }
  
  else {
    Output("No Valid Network");
    return (0);   
  }
  
}

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
 * OBS_N2S_Add() - Save OBS to N2S file
 * ======================================================================================================================
 */
void OBS_N2S_Add() {
  if (obs.inuse) {     // Sanity check
    char ts[32];
   
    memset(obsbuf, 0, sizeof(obsbuf));

    // Modify System Status and Set From Need to Send file bit
    obs.hth |= SSB_FROM_N2S; // Turn On Bit

    // If WiFi add additional itemslike Chords url.
    if (WiFi_valid) {
      tm *dt = gmtime(&obs.ts);
      sprintf (obsbuf, "%s?key=%s&instrument_id=%d&",
        cf_urlpath, cf_apikey, cf_instrument_id, DeviceID);
      sprintf (obsbuf+strlen(obsbuf), "devid=%s&at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
        DeviceID,
        dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
        dt->tm_hour, dt->tm_min, dt->tm_sec);
      sprintf (obsbuf+strlen(obsbuf), "&bv=%d.%02d",
       (int)obs.bv, (int)(obs.bv*100)%100); 
      sprintf (obsbuf+strlen(obsbuf), "&hth=%d", obs.hth);
    }
    else {
      // Do not send device id for LoraWAN, use LoRa ID to identify device
      // ABP mode: lw_devaddr, OTAA: lw_deveui 
      // Time as hex = 8 bytes, verses 18 bytes
      sprintf (obsbuf, "at=%08X", (uint32_t) obs.ts);
      sprintf (obsbuf+strlen(obsbuf), "&v=%d.%02d",
       (int)obs.bv, (int)(obs.bv*100)%100); 
      sprintf (obsbuf+strlen(obsbuf), "&h=%d", obs.hth);
    }
   
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            // sprintf (obsbuf+strlen(obsbuf), "&%s=%d%%2E%d", obs.sensor[s].id, 
            //  (int)obs.sensor[s].f_obs,  (int)(obs.sensor[s].f_obs*1000)%1000);
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
      
    sprintf (obsbuf+strlen(obsbuf), ",\"bv\":%d.%02d", (int)obs.bv, (int)(obs.bv*100)%100); 
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
 * OBS_Build() - Create observation in obsbuf from structure obs for sending
 * 
 * Example at=2022-05-17T17%3A40%3A04&hth=8770 .....
 * ======================================================================================================================
 */
bool OBS_Build() {  
  if (obs.inuse) {     // Sanity check  
    memset(obsbuf, 0, sizeof(obsbuf));
    tm *dt = gmtime(&obs.ts);

    // If WiFi add additional itemslike Chords url.
    if (WiFi_valid) {
      sprintf (obsbuf, "%s?key=%s&instrument_id=%d&devid=%s",
        cf_urlpath, cf_apikey, cf_instrument_id, DeviceID);
    }
    else {
      // Do not send device id for LoraWAN, use LoRa ID to identify device
      // ABP mode: lw_devaddr, OTAA: lw_deveui 
      // Time as hex = 8 bytes, verses 18 bytes

      // The code at IOT website will determine this as a observation.
      // Add what is necessary to forward to chord
      sprintf (obsbuf,"");
    }

    sprintf (obsbuf+strlen(obsbuf), "at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
    sprintf (obsbuf+strlen(obsbuf), "&bv=%d.%02d",
      (int)obs.bv, (int)(obs.bv*100)%100); 
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

    Output("OBSBLD:OK");
    Serial_writeln (obsbuf);
    return (true);
  }
  else {
    Output("OBSBLD:INUSE");
    return (false);
  }
}

/*
 * ======================================================================================================================
 * OBS_N2S_Save() - Save Observations to Need2Send File
 * ======================================================================================================================
 */
void OBS_N2S_Save() {

  // Save Station Observations to N2S file
  OBS_N2S_Add();
  OBS_Clear();
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
  float sht1_humid = 0.0;
  float sht1_temp = 0.0;
  float heat_index = 0.0;
  float bmx_1_pressure = 0.0;

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
      rg1ds = (millis()-raingauge1_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
      rg1 = raingauge1_interrupt_count * 0.2;
      raingauge1_interrupt_count = 0;
      raingauge1_interrupt_stime = millis();
      raingauge1_interrupt_ltime = 0; // used to debounce the tip
      // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
      rg1 = (isnan(rg1) || (rg1 < QC_MIN_RG) || (rg1 > (((float)rg1ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg1;
    }

    // Rain Gauge 2 - Each tip is 0.2mm of rain
    if (cf_op1 == 2) {
      rg2ds = (millis()-raingauge2_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
      rg2 = raingauge2_interrupt_count * 0.2;
      raingauge2_interrupt_count = 0;
      raingauge2_interrupt_stime = millis();
      raingauge2_interrupt_ltime = 0; // used to debounce the tip
      // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
      rg2 = (isnan(rg2) || (rg2 < QC_MIN_RG) || (rg2 > (((float)rg2ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg2;
    }

    if (cf_rg1_enable || (cf_op1 == 2)) {
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
    if (cf_op1 == 2) {
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

    if (cf_op1 == 1) {
      // OP1 Raw
      strcpy (obs.sensor[sidx].id, "op1r");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = Pin_ReadAvg(OP1_PIN);
      obs.sensor[sidx++].inuse = true;
    }

    if ((cf_op1 == 5) || (cf_op1 == 10)) {
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

    if (cf_op2 == 1) {
      // OP2 Raw
      strcpy (obs.sensor[sidx].id, "op2r");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = Pin_ReadAvg(OP2_PIN);
      obs.sensor[sidx++].inuse = true;
    }

    if (cf_op2 == 2) {
      // OP2 Voltaic Battery Voltage
      strcpy (obs.sensor[sidx].id, "vbv");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = VoltaicVoltage(OP2_PIN);
      obs.sensor[sidx++].inuse = true;
    }

    if (AS5600_exists) {
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

      Wind_ClearSampleCount(); // Clear Counter, Counter maintain how many samples since last obs sent
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

    // Standard Particle PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.s10;
    obs.sensor[sidx++].inuse = true;

    // Standard Particle PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.s25;
    obs.sensor[sidx++].inuse = true;

    // Standard Particle PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.s100;
    obs.sensor[sidx++].inuse = true;

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

  //
  // Add I2C Sensors
  //
  if (BMX_1_exists) {
    float p = 0.0;
    float t = 0.0;
    float h = 0.0;

    if (BMX_1_chip_id == BMP280_CHIP_ID) {
      p = bmp1.readPressure()/100.0F;       // bp1 hPa
      t = bmp1.readTemperature();           // bt1
    }
    else if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
      if (BMX_1_type == BMX_TYPE_BME280) {
        p = bme1.readPressure()/100.0F;     // bp1 hPa
        t = bme1.readTemperature();         // bt1
        h = bme1.readHumidity();            // bh1 
      }
      if (BMX_1_type == BMX_TYPE_BMP390) {
        p = bm31.readPressure()/100.0F;     // bp1 hPa
        t = bm31.readTemperature();         // bt1 
      }    
    }
    else { // BMP388
      p = bm31.readPressure()/100.0F;       // bp1 hPa
      t = bm31.readTemperature();           // bt1
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    
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
    float p = 0.0;
    float t = 0.0;
    float h = 0.0;

    if (BMX_2_chip_id == BMP280_CHIP_ID) {
      p = bmp2.readPressure()/100.0F;       // bp2 hPa
      t = bmp2.readTemperature();           // bt2
    }
    else if (BMX_2_chip_id == BME280_BMP390_CHIP_ID) {
      if (BMX_2_type == BMX_TYPE_BME280) {
        p = bme2.readPressure()/100.0F;     // bp2 hPa
        t = bme2.readTemperature();         // bt2
        h = bme2.readHumidity();            // bh2 
      }
      if (BMX_2_type == BMX_TYPE_BMP390) {
        p = bm32.readPressure()/100.0F;     // bp2 hPa
        t = bm32.readTemperature();         // bt2       
      }
    }
    else { // BMP388
      p = bm32.readPressure()/100.0F;       // bp2 hPa
      t = bm32.readTemperature();           // bt2
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;

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
  
  if (SHT_1_exists) {                                                                               
    float t = 0.0;
    float h = 0.0;

    // SHT1 Temperature
    strcpy (obs.sensor[sidx].id, "st1");
    obs.sensor[sidx].type = F_OBS;
    t = sht1.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    
    // SHT1 Humidity   
    strcpy (obs.sensor[sidx].id, "sh1");
    obs.sensor[sidx].type = F_OBS;
    h = sht1.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;

    sht1_humid = h; // save for derived observations
  }

  if (SHT_2_exists) {
    float t = 0.0;
    float h = 0.0;

    // SHT2 Temperature
    strcpy (obs.sensor[sidx].id, "st2");
    obs.sensor[sidx].type = F_OBS;
    t = sht2.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    
    // SHT2 Humidity   
    strcpy (obs.sensor[sidx].id, "sh2");
    obs.sensor[sidx].type = F_OBS;
    h = sht2.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }
  
  if (HDC_1_exists) {
    double t = -999.9;
    double h = -999.9;

    if (hdc1.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0)) {
      t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
      h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    }
    else {
      Output ("ERR:HDC1 Read");
    }

    // HDC1 Temperature
    strcpy (obs.sensor[sidx].id, "hdt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // HDC1 Humidity
    strcpy (obs.sensor[sidx].id, "hdh1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) h;
    obs.sensor[sidx++].inuse = true;

  }

  if (HDC_2_exists) {
    double t = -999.9;
    double h = -999.9;

    if (hdc2.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0)) {
      t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
      h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    }
    else {
      Output ("ERR:HDC1 Read");
    }

    // HDC2 Temperature
    strcpy (obs.sensor[sidx].id, "hdt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // HDC2 Humidity
    strcpy (obs.sensor[sidx].id, "hdh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) h;
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


  if (VEML7700_exists) {
    float lux = veml.readLux(VEML_LUX_AUTO);
    lux = (isnan(lux) || (lux < QC_MIN_VLX)  || (lux > QC_MAX_VLX))  ? QC_ERR_VLX  : lux;

    // VEML7700 Auto Lux Value
    strcpy (obs.sensor[sidx].id, "vlx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = lux;
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
  
    // Tinovi Multi Level Soil Moisture
  if (TMSM_exists) {
    soil_ret_t multi;
    float t;

    tmsm.newReading();
    delay(100);
    tmsm.getData(&multi);

    strcpy (obs.sensor[sidx].id, "tmsms1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[0];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[1];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms3");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[2];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms4");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[3];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms5");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[4];
    obs.sensor[sidx++].inuse = true;
    
    t = multi.temp[0];
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    strcpy (obs.sensor[sidx].id, "tmsmt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    t = multi.temp[1];
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    strcpy (obs.sensor[sidx].id, "tmsmt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  } 

  // Tinovi Soil Moisture
  mux_obs_do(sidx);

  // Set this after we read all sensors. So we capture if their state changes
  obs.hth = SystemStatusBits;

  Output("OBS_TAKE(DONE)");
}

/*
 * ======================================================================================================================
 * OBS_Do() - Do Observation Processing
 * ======================================================================================================================
 */
void OBS_Do() {
  Output("OBS_DO()");

  OBS_Take();          // Take an observation

  // At this point, the obs data structure has been filled in with observation data
  OBS_LOG_Add();        // Save Observation Data to Log file.

  // Build Observation to Send
  Output("OBS_BUILD()");
  OBS_Build();

  Output("OBS_SEND()");
  if (SendMsg(obsbuf) != 1) {  
    Output("FS->PUB FAILED");
    OBS_N2S_Save(); // Saves Main observations
  }
  else {
    bool OK2Send = true;
        
    Output("FS->PUB OK");

    // Check if we have any N2S only if we have not added to the file while trying to send OBS
    if (OK2Send) {
      SD_N2S_Publish(); 
    }
  }
}

