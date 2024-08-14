/*
 * ======================================================================================================================
 *  OBS.h - Observation Handeling
 * ======================================================================================================================
 */
void BackGroundWork();   // Prototype this function to aviod compile function unknown issue.

#define OBSERVATION_INTERVAL      60   // Seconds
#define MAX_SENSORS         48

typedef enum {
  F_OBS, 
  I_OBS, 
  U_OBS
} OBS_TYPE;

typedef struct {
  char          id[6];       // Suport 4 character length observation names
  int           type;
  float         f_obs;
  int           i_obs;
  unsigned long u_obs;
  bool          inuse;
} SENSOR;

typedef struct {
  bool            inuse;                // Set to true when an observation is stored here         
  time_t          ts;                   // TimeStamp
  float           bv;                   // Lipo Battery Voltage
  unsigned long   hth;                  // System Status Bits
  SENSOR          sensor[MAX_SENSORS];
} OBSERVATION_STR;

OBSERVATION_STR obs;

unsigned long Time_of_obs = 0;              // unix time of observation
unsigned long Time_of_next_obs = 0;         // time of next observation


void OBS_N2S_Publish();   // Prototype this function to aviod compile function unknown issue.

/*
 * ======================================================================================================================
 * OBS_Send() - Do a GET request to log observation, process returned text for result code and set return status
 * ======================================================================================================================
 */
int OBS_Send(char *obs)
{
  // Handle LoRaWAN
  if (LW_valid) {
    if (LW_Send(obs)) {
      return(1);
    }
    else {
      return(0);
    }
  }

  // Handle WiFi can retutn 0=not sent, -500=ErrorCode Not Sent, 1=Sent
  else if (WiFi_valid) {
    return (WiFi_Send(obs));
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

    tm *dt = gmtime(&obs.ts); 

    // If WiFi add additional items to be logged on a recording site like Chords.
    if (WiFi_valid) {
      sprintf (obsbuf, "%s?key=%s&instrument_id=%d", cf_urlpath, cf_apikey, cf_instrument_id);
    }
    
    sprintf (obsbuf+strlen(obsbuf), "&at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
      
    sprintf (obsbuf+strlen(obsbuf), "&bv=%d.%02d",
       (int)obs.bv, (int)(obs.bv*100)%100); 

    // Modify System Status and Set From Need to Send file bit
    obs.hth |= SSB_FROM_N2S; // Turn On Bit
    sprintf (obsbuf+strlen(obsbuf), "&hth=%d", obs.hth);
   
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
  Output("OBS_ADD()");
    
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
 * OBS_Build() - Create observation in obsbuf for sending to Chords
 * 
 * Example at=2022-05-17T17%3A40%3A04&hth=8770 .....
 * ======================================================================================================================
 */
bool OBS_Build() {  
  if (obs.inuse) {     // Sanity check  
    memset(obsbuf, 0, sizeof(obsbuf));

    tm *dt = gmtime(&obs.ts); 

    // If WiFi add additional items to be logged on a recording site like Chords.
    if (WiFi_valid) {
      sprintf (obsbuf, "%s?key=%s&instrument_id=%d", cf_urlpath, cf_apikey, cf_instrument_id);
    }
    
    sprintf (obsbuf+strlen(obsbuf), "at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);

    sprintf (obsbuf+strlen(obsbuf), "&bv=%d.%02d",
       (int)obs.bv, (int)(obs.bv*100)%100); 
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
  int sidx = 0;;
  float rg1 = 0.0;
  float rg2 = 0.0;
  unsigned long rg1ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  unsigned long rg2ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  float ws = 0.0;
  int wd = 0;
  float mcp1_temp = 0.0;
  float sht1_humid = 0.0;
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
  obs.ts = Time_of_obs;
  obs.hth = SystemStatusBits;

  obs.bv = vbat_get();

  Output("TAKE1");

  // Rain Gauge 1 - Each tip is 0.2mm of rain
  if (cf_rg1_enable) {
    rg1ds = (millis()-raingauge1_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rg1 = raingauge1_interrupt_count * 0.2;
    raingauge1_interrupt_count = 0;
    raingauge1_interrupt_stime = millis();
    raingauge1_interrupt_ltime = 0; // used to debounce the tip
    // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
    rg1 = (isnan(rg1) || (rg1 < QC_MIN_RG) || (rg1 > ((rg1ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg1;
  }

  // Rain Gauge 2 - Each tip is 0.2mm of rain
  if (cf_rg2_enable) {
    rg2ds = (millis()-raingauge2_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rg2 = raingauge2_interrupt_count * 0.2;
    raingauge2_interrupt_count = 0;
    raingauge2_interrupt_stime = millis();
    raingauge2_interrupt_ltime = 0; // used to debounce the tip
    // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
    rg2 = (isnan(rg2) || (rg2 < QC_MIN_RG) || (rg2 > ((rg2ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg2;
  }

  if (cf_rg1_enable || cf_rg2_enable) {
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
  if (cf_rg2_enable) {
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

  Output("TAKE2");

  if (cf_ds_enable) {
    strcpy (obs.sensor[sidx].id, "ds");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = DS_Median();
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

  Output("TAKE3");
  
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
    strcpy (obs.sensor[sidx].id, "bh1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }

  Output("TAKE4");
  
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
    strcpy (obs.sensor[sidx].id, "bh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }

  Output("TAKE5");
  
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

  Output("TAKE6");
  
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

  Output("TAKE7");
  
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

  Output("TAKE8");
  
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
        SystemStatusBits &= ~SSB_SI1145; // Turn Off Bit

        si_vis = uv.readVisible();
        si_ir = uv.readIR();
        si_uv = uv.readUV()/100.0;
      }
      else {
        SI1145_exists = false;
        Output ("SI OFFLINE");
        SystemStatusBits |= SSB_SI1145;  // Turn On Bit    
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

  Output("TAKE9");
    
  if (MCP_1_exists) {
    float t = 0.0;
   
    // MCP1 Temperature
    strcpy (obs.sensor[sidx].id, "mt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp1.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    mcp1_temp = t; // save for derived observations
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

  Output("TAKE10");
  
  if (VEML7700_exists) {
    float lux = veml.readLux(VEML_LUX_AUTO);
    lux = (isnan(lux) || (lux < QC_MIN_LX)  || (lux > QC_MAX_LX))  ? QC_ERR_LX  : lux;

    // VEML7700 Auto Lux Value
    strcpy (obs.sensor[sidx].id, "lx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = lux;
    obs.sensor[sidx++].inuse = true;
  }

  Output("TAKE11");
  
  if (PM25AQI_exists) {
    // Standard Particle PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s10;
    obs.sensor[sidx++].inuse = true;

    // Standard Particle PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s25;
    obs.sensor[sidx++].inuse = true;

    // Standard Particle PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s100;
    obs.sensor[sidx++].inuse = true;

    // Atmospheric Environmental PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e10;
    obs.sensor[sidx++].inuse = true;

    // Atmospheric Environmental PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e25;
    obs.sensor[sidx++].inuse = true;

    // Atmospheric Environmental PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e100;
    obs.sensor[sidx++].inuse = true;

    // Clear readings
    pm25aqi_clear();
  }

  Output("TAKE12");
  
  // Heat Index Temperature
  if (HI_exists) {
    heat_index = hi_calculate(mcp1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "hi");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) heat_index;
    obs.sensor[sidx++].inuse = true;    
  }
  
  // Wet Bulb Temperature
  if (WBT_exists) {
    strcpy (obs.sensor[sidx].id, "wbt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbt_calculate(mcp1_temp, sht1_humid);
    obs.sensor[sidx++].inuse = true;    
  }

  // Wet Bulb Globe Temperature
  if (WBGT_exists) {
    strcpy (obs.sensor[sidx].id, "wbgt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbgt_calculate(heat_index);
    obs.sensor[sidx++].inuse = true;    
  }
  Output("OBS_TAKE(DONE)");
}

/*
 * ======================================================================================================================
 * OBS_Do() - Do Observation Processing
 * ======================================================================================================================
 */
void OBS_Do() {
  Output("OBS_DO()");
  
  I2C_Check_Sensors(); // Make sure Sensors are online

  // Take an observation
  OBS_Take();

  // At this point, the obs data structure has been filled in with observation data
  
  // Save Observation Data to Log file.
  OBS_LOG_Add(); 

  // Build Observation to Send
  Output("OBS_BUILD()");
  OBS_Build();

  Output("OBS_SEND()");
  if (OBS_Send(obsbuf) != 1) {  
    Output("FS->PUB FAILED");
    OBS_N2S_Save(); // Saves Main observations
  }
  else {
    bool OK2Send = true;
        
    Output("FS->PUB OK");

    // Check if we have any N2S only if we have not added to the file while trying to send OBS
    if (OK2Send) {
      OBS_N2S_Publish(); 
    }
  }
}

/* 
 *=======================================================================================================================
 * OBS_N2S_Publish()
 *=======================================================================================================================
 */
void OBS_N2S_Publish() {
  File fp;
  char ch;
  int i;
  int sent=0;

  memset(obsbuf, 0, sizeof(obsbuf));

  Output ("OBS:N2S Publish");

  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  
  if (SD_exists && SD.exists(SD_n2s_file)) {
    Output ("OBS:N2S:Exists");

    fp = SD.open(SD_n2s_file, FILE_READ); // Open the file for reading, starting at the beginning of the file.

    if (fp) {
      // Delete Empty File or too small of file to be valid
      if (fp.size()<=20) {
        fp.close();
        Output ("OBS:N2S:Empty");
        SD_N2S_Delete();
      }
      else {
        if (eeprom.n2sfp) {
          if (fp.size()<=eeprom.n2sfp) {
            // Something wrong. Can not have a file position that is larger than the file
            eeprom.n2sfp = 0; 
          }
          else {
            fp.seek(eeprom.n2sfp);  // Seek to where we left off last time. 
          }
        } 

        // Loop through each line / obs and transmit
        
        // set timer on when we need to stop sending n2s obs
        uint64_t TimeFromNow = millis() + 45000; // 1 min obs
        if (cf_5m_enable) {  
          TimeFromNow = millis() + (4 * 60000);  
        }
        else if (cf_15m_enable) {    
          TimeFromNow = millis() + (14 * 60000);  
        }
        
        i = 0;
        while (fp.available() && (i < MAX_MSGBUF_SIZE )) {
          ch = fp.read();

          if (ch == 0x0A) {  // newline
            int send_result = OBS_Send(obsbuf);
            if (send_result == 1) { 
              sprintf (Buffer32Bytes, "OBS:N2S[%d]->PUB:OK", sent++);
              Output (Buffer32Bytes);
              Serial_writeln (obsbuf);

              // setup for next line in file
              i = 0;

              // file position is at the start of the next observation or at eof
              eeprom.n2sfp = fp.position();
              
              sprintf (Buffer32Bytes, "OBS:N2S[%d] Contunue", sent);
              Output (Buffer32Bytes); 

              if(millis() > TimeFromNow) {
                // need to break out so new obs can be made
                Output ("OBS:N2S->TIME2EXIT");
                break;                
              }
            }
            
            if (send_result == -500) { // HTTP/1.1 500 Internal Server Error
              // Suspect we have a bad N2S observation that webserver does not like, move past it.
              sprintf (Buffer32Bytes, "OBS:N2S[%d]->ERR:500", sent++);
              Output (Buffer32Bytes);
              Serial_writeln (obsbuf);

              // setup for next line in file
              i = 0;

              // file position is at the start of the next observation or at eof
              eeprom.n2sfp = fp.position();
              
              sprintf (Buffer32Bytes, "OBS:N2S[%d] Contunue", sent);
              Output (Buffer32Bytes); 

              if(millis() > TimeFromNow) {
                // need to break out so new obs can be made
                Output ("OBS:N2S->TIME2EXIT");
                break;                
              }             
            }
            else {
                sprintf (Buffer32Bytes, "OBS:N2S[%d]->PUB:ERR", sent);
                Output (Buffer32Bytes);
                // On transmit failure, stop processing file.
                break;
            }
            
            // At this point file pointer's position is at the first character of the next line or at eof
            
          } // Newline
          else if (ch == 0x0D) { // CR, LF follows and will trigger the line to be processed       
            obsbuf[i] = 0; // null terminate then wait for newline to be read to process OBS
          }
          else {
            obsbuf[i++] = ch;
          }

          // Check for buffer overrun
          if (i >= MAX_MSGBUF_SIZE) {
            sprintf (Buffer32Bytes, "OBS:N2S[%d]->BOR:ERR", sent);
            Output (Buffer32Bytes);
            fp.close();
            SD_N2S_Delete(); // Bad data in the file so delete the file           
            return;
          }
        } // end while 

        if (fp.available() <= 20) {
          // If at EOF or some invalid amount left then delete the file
          fp.close();
          SD_N2S_Delete();
        }
        else {
          // At this point we sent 0 or more observations but there was a problem.
          // eeprom.n2sfp was maintained in the above read loop. So we will close the
          // file and next time this function is called we will seek to eeprom.n2sfp
          // and start processing from there forward. 
          fp.close();
          EEPROM_Update(); // Update file postion in the eeprom.
        }
      }
    }
    else {
        Output ("OBS:N2S->OPEN:ERR");
    }
  }
}
