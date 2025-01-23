/*
 * ======================================================================================================================
 *  OBS.h - Observation Handeling
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Observation storage
 * ======================================================================================================================
 */
#define MAX_SENSORS         96
#define MAX_ONE_MINUTE_OBS  17 // Want more OBS space than our OBSERVATION_TRANSMIT_INTERVAL (For 15m interval use 17)
                              // This prevents OBS from filling and being written to N2S file while we are Connecting

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
  float           css;                  // Cell Signal Strength
  unsigned long   hth;                  // System Status Bits
  SENSOR          sensor[MAX_SENSORS];
} OBSERVATION_STR;
OBSERVATION_STR obs[MAX_ONE_MINUTE_OBS];


/*
 * ======================================================================================================================
 * OBS_Clear() - Set OBS to not in use
 * ======================================================================================================================
 */
void OBS_Clear(int i) {
  obs[i].inuse =false;
  for (int s=0; s<MAX_SENSORS; s++) {
    obs[i].sensor[s].inuse = false;
  }
}

/*
 * ======================================================================================================================
 * OBS_Init() - Set all OBS to not in use
 * ======================================================================================================================
 */
void OBS_Init() {
  // Scan OBS for free spot
  for (int i=0; i<MAX_ONE_MINUTE_OBS; i++){
    OBS_Clear(i);
  }
}

/*
 * ======================================================================================================================
 * OBS_N2S_Add() - Save OBS to N2S file
 * ======================================================================================================================
 */
void OBS_N2S_Add(int i) {
  if (obs[i].inuse) {     // Sanity check
    char ts[32];
    
    memset(msgbuf, 0, sizeof(msgbuf));

    JSONBufferWriter writer(msgbuf, sizeof(msgbuf)-1);
    writer.beginObject();

    sprintf (ts, "%d-%02d-%02dT%02d:%02d:%02d",
      Time.year(obs[i].ts), Time.month(obs[i].ts), Time.day(obs[i].ts),
      Time.hour(obs[i].ts), Time.minute(obs[i].ts), Time.second(obs[i].ts));

    writer.name("at").value(ts);
    writer.name("css").value(obs[i].css, 4);

    // Modify System Status and Set From Need to Send file bit
    obs[i].hth |= SSB_FROM_N2S; // Turn On Bit
    writer.name("hth").value((int) obs[i].hth);

    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs[i].sensor[s].inuse) {
        switch (obs[i].sensor[s].type) {
          case F_OBS :
            writer.name(obs[i].sensor[s].id).value(obs[i].sensor[s].f_obs, 1);
            break;
          case I_OBS :
            writer.name(obs[i].sensor[s].id).value(obs[i].sensor[s].i_obs);
            break;
          case U_OBS :
            writer.name(obs[i].sensor[s].id).value((int) obs[i].sensor[s].u_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }
    writer.endObject();

    sprintf (msgbuf+strlen(msgbuf), ",FS");  // Add Particle Event Type after JSON structure
    SD_NeedToSend_Add(msgbuf); // Save to N2F File
    sprintf (Buffer32Bytes, "OBS->%d Add N2S", i);
    Output(Buffer32Bytes);
    Serial_write (msgbuf);
  }
}

/*
 * ======================================================================================================================
 * OBS_FS_Build_JSON() - Create JSON observation in msgbuf
 * ======================================================================================================================
 */
bool OBS_FS_Build_JSON(int i) {
  if (obs[i].inuse) {     // Sanity check
    char ts[32];
    
    memset(msgbuf, 0, sizeof(msgbuf));

    JSONBufferWriter writer(msgbuf, sizeof(msgbuf)-1);
    writer.beginObject();

    sprintf (ts, "%d-%02d-%02dT%02d:%02d:%02d",
      Time.year(obs[i].ts), Time.month(obs[i].ts), Time.day(obs[i].ts),
      Time.hour(obs[i].ts), Time.minute(obs[i].ts), Time.second(obs[i].ts));

    writer.name("at").value(ts);
    writer.name("css").value(obs[i].css, 4);
    writer.name("hth").value((int) obs[i].hth);

    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs[i].sensor[s].inuse) {
        switch (obs[i].sensor[s].type) {
          case F_OBS :
            // sprintf (Buffer32Bytes, "%s=%.4f", obs[i].sensor[s].id, obs[i].sensor[s].f_obs);
            // Output (Buffer32Bytes);
            writer.name(obs[i].sensor[s].id).value(obs[i].sensor[s].f_obs, 1);
            break;
          case I_OBS :
            writer.name(obs[i].sensor[s].id).value(obs[i].sensor[s].i_obs);
            break;
          case U_OBS :
            writer.name(obs[i].sensor[s].id).value((int) obs[i].sensor[s].u_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }
    writer.endObject();
    return (true);
  }
  else {
    return (false);
  }
}

/*
 * ======================================================================================================================
 * OBS_Relay_Build_JSON() - Create JSON observation in msgbuf, remove from relay structure
 *                          Return the message relay type we are preparing
 * ======================================================================================================================
 */
int OBS_Relay_Build_JSON() {
  LORA_MSG_RELAY_STR *m;
  int relay_type = 0;

  memset(msgbuf, 0, sizeof(msgbuf));
  JSONBufferWriter writer(msgbuf, sizeof(msgbuf)-1);

  // Locate message we need to log
  int i = lora_relay_need2log_idx();
  if (i >= 0) {
    m = &lora_msg_relay[i];
    strncpy (msgbuf, m->message, LORA_RELAY_MSG_LENGTH-1); // minus 1 so last byte in array will always be null
    relay_type = m->message_type;
    lora_relay_msg_free(m);
  }
  return (relay_type);
}

/*
 * ======================================================================================================================
 * OBS_Log() - Save OBS to Log file
 * ======================================================================================================================
 */
void OBS_Log(int i) {
  if (OBS_FS_Build_JSON(i)) {
    sprintf (Buffer32Bytes, "OBS[%d]->SD", i);
    Output(Buffer32Bytes);
    Serial_write (msgbuf);

    SD_LogObservation(msgbuf);
  }
}

/*
 * ======================================================================================================================
 * OBS_N2S_SaveAll() - Save All N Observations to Need2Send File
 * ======================================================================================================================
 */
void OBS_N2S_SaveAll() {
  int relay_type;

  // Save All Station Observations to N2S file
  for (int i=0; i<MAX_ONE_MINUTE_OBS; i++) {
    OBS_N2S_Add (i);
    OBS_Clear(i);
  }

  // Save All Rain and Soil LoRa Observations to N2S file
  while (lora_relay_need2log()) {
   relay_type = OBS_Relay_Build_JSON(); // This removed msg from relay structure and places it in msgbuf
    sprintf (msgbuf+strlen(msgbuf), ", %s", relay_msgtypes[relay_type]);  // Add Particle Event Type after JSON structure
    SD_NeedToSend_Add(msgbuf); // Save to N2F File
    Output("RS->N2S");
    Serial_write (msgbuf); 
  }
}

/*
 * ======================================================================================================================
 * OBS_Last() - Find Last OBS and return index or -1 if empty
 * ======================================================================================================================
 */
int OBS_Last() {
  // Scan OBS for free spot
  for (int i=0; i<MAX_ONE_MINUTE_OBS; i++){
    if (obs[i].inuse == false) {
      if (i == 0) {
        return (-1);
      }
      else {
        return (i-1);
      }
    }
  }
  // Arrary full, Never found an empty, return last in Array
  return (MAX_ONE_MINUTE_OBS-1);
}

/*
 * ======================================================================================================================
 * OBS_Full() - Return true if there are open spots
 * ======================================================================================================================
 */
bool OBS_Full() {
  // Scan OBS for free spot
  for (int i=0; i<MAX_ONE_MINUTE_OBS; i++){
    if (obs[i].inuse == false) {
      return (false);
    }
  }
  return (true);
}

/*
 * ======================================================================================================================
 * OBS_Open() - Find a free OBS and return index
 * ======================================================================================================================
 */
int OBS_Open() {
  // Scan OBS for free spot
  for (int i=0; i<MAX_ONE_MINUTE_OBS; i++){
    if (obs[i].inuse == false) {
      return (i);
    }
  }

  // All Observations are inuse
  Output ("OBS[ALL]->N2S");

  // Save All N Observations to Need2Send File
  OBS_N2S_SaveAll();

  // All OBS are Clear so return the first OBS 
  return (0);
}

/*
 * ======================================================================================================================
 * OBS_Do() - Get Observations - Should be called once a minute
 * ======================================================================================================================
 */
void OBS_Do() {
  int oidx;
  int sidx = 0;;
  float rain = 0.0;
  float rain2 = 0.0;
  float ws = 0.0;
  int wd = 0;
  unsigned long rgds;    // rain gauge delta seconds, seconds since last rain gauge observation logged
  unsigned long rg2ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  float BatteryPoC = 0.0; // Battery Percent of Charge
  float mcp1_temp = 0.0;  // air temperature
  float mcp3_temp = 0.0;  // globe temperature
  float wetbulb_temp = 0.0;
  float sht1_humid = 0.0;
  float heat_index = 0.0;

// Output("DB:OBS_Start");


  // Safty Check for Vaild Time
  if (!Time.isValid()) {
    Output ("OBS_Do: Time NV");
    return;
  }

  Wind_GustUpdate(); // Update Gust and Gust Direction readings
  
#if PLATFORM_ID == PLATFORM_ARGON
  int BatteryState = 0;
  WiFiSignal sig = WiFi.RSSI();
  byte cfr = 0;
#else
  int BatteryState = System.batteryState();
  CellularSignal sig = Cellular.RSSI();
  byte cfr = pmic.getFault(); // Get Battery Charger Failt Register
  if (BatteryState>0 && BatteryState<6) {
    BatteryPoC = System.batteryCharge();
  }
#endif

  oidx = OBS_Open();    // Get a free observation spot

  obs[oidx].inuse = true;
  obs[oidx].ts = Time.now();
  obs[oidx].css = sig.getStrength();

  // 00 Battery Charging State
  strcpy (obs[oidx].sensor[sidx].id, "bcs");
  obs[oidx].sensor[sidx].type = I_OBS;
  obs[oidx].sensor[sidx].i_obs = BatteryState;
  obs[oidx].sensor[sidx++].inuse = true;

  // 01 Battery Percent Charge
  strcpy (obs[oidx].sensor[sidx].id, "bpc");
  obs[oidx].sensor[sidx].type = F_OBS;
  obs[oidx].sensor[sidx].f_obs = BatteryPoC;
  obs[oidx].sensor[sidx++].inuse = true;

  // 02 Battery Charger Fault Register
  strcpy (obs[oidx].sensor[sidx].id, "cfr");
  obs[oidx].sensor[sidx].type = I_OBS;
  obs[oidx].sensor[sidx].i_obs = cfr;
  obs[oidx].sensor[sidx++].inuse = true;

  // 03 Rain Gauge - Each tip is 0.2mm of rain
  rgds = (System.millis()-raingauge1_interrupt_stime)/1000;
  rain = raingauge1_interrupt_count * 0.2;
  rain = (isnan(rain) || (rain < QC_MIN_RG) || (rain > ((rgds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rain;
  raingauge1_interrupt_count = 0;
  raingauge1_interrupt_stime = System.millis();
  raingauge1_interrupt_ltime = 0; // used to debounce the tip

  if (A4_State == A4_STATE_RAIN) {
    rg2ds = (System.millis()-raingauge2_interrupt_stime)/1000;
    rain2 = raingauge2_interrupt_count * 0.2;
    rain2 = (isnan(rain2) || (rain2 < QC_MIN_RG) || (rain2 > ((rg2ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rain2;
    raingauge2_interrupt_count = 0;
    raingauge2_interrupt_stime = System.millis();
    raingauge2_interrupt_ltime = 0; // used to debounce the tip
  }

// Output("DB:OBS_URT");
  EEPROM_UpdateRainTotals(rain, rain2);
// Output("DB:OBS_URTx");

  // 04 Rain Gauge
  strcpy (obs[oidx].sensor[sidx].id, "rg");
  obs[oidx].sensor[sidx].type = F_OBS;
  obs[oidx].sensor[sidx].f_obs = rain;
  obs[oidx].sensor[sidx++].inuse = true;

  // 05 Rain Gauge Delta Seconds
  // strcpy (obs[oidx].sensor[sidx].id, "rgs");
  // obs[oidx].sensor[sidx].type = U_OBS;
  // obs[oidx].sensor[sidx].u_obs = rgds;
  // obs[oidx].sensor[sidx++].inuse = true;

  // 06 Rain Gauge Total
  strcpy (obs[oidx].sensor[sidx].id, "rgt");
  obs[oidx].sensor[sidx].type = F_OBS;
  obs[oidx].sensor[sidx].f_obs = eeprom.rgt1;
  obs[oidx].sensor[sidx++].inuse = true;

  // 07 Rain Gauge  Prior Day
  strcpy (obs[oidx].sensor[sidx].id, "rgp");
  obs[oidx].sensor[sidx].type = F_OBS;
  obs[oidx].sensor[sidx].f_obs = eeprom.rgp1;
  obs[oidx].sensor[sidx++].inuse = true;

  // 08 Wind Speed (Global)
  strcpy (obs[oidx].sensor[sidx].id, "ws");
  obs[oidx].sensor[sidx].type = F_OBS;
  ws = Wind_SpeedAverage();
  ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
  obs[oidx].sensor[sidx].f_obs = ws;
  obs[oidx].sensor[sidx++].inuse = true;

  // 09 Wind Direction
  strcpy (obs[oidx].sensor[sidx].id, "wd");
  obs[oidx].sensor[sidx].type = I_OBS;
  wd = Wind_DirectionVector();
  wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
  obs[oidx].sensor[sidx].i_obs = wd;
  obs[oidx].sensor[sidx++].inuse = true;

  // 10 Wind Gust (Global)
  strcpy (obs[oidx].sensor[sidx].id, "wg");
  obs[oidx].sensor[sidx].type = F_OBS;
  ws = Wind_Gust();
  ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
  obs[oidx].sensor[sidx].f_obs = ws;
  obs[oidx].sensor[sidx++].inuse = true;

  // 11 Wind Gust Direction (Global)
  strcpy (obs[oidx].sensor[sidx].id, "wgd");
  obs[oidx].sensor[sidx].type = I_OBS;
  wd = Wind_GustDirection();
  wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
  obs[oidx].sensor[sidx].i_obs = wd;
  obs[oidx].sensor[sidx++].inuse = true;

// Output("DB:OBS_I2C");

  //
  // Add I2C Sensors
  //
  if (BMX_1_exists) {
// Output("DB:OBS_BMX1");

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
    
    // 12 BMX1 Preasure
    strcpy (obs[oidx].sensor[sidx].id, "bp1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = p;
    obs[oidx].sensor[sidx++].inuse = true;

    // 13 BMX1 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "bt1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;

    // 14 BMX1 Humidity
    if (BMX_1_type == BMX_TYPE_BME280) {
      strcpy (obs[oidx].sensor[sidx].id, "bh1");
      obs[oidx].sensor[sidx].type = F_OBS;
      obs[oidx].sensor[sidx].f_obs = h;
      obs[oidx].sensor[sidx++].inuse = true;
    }
// Output("DB:OBS_BMX1x");
  }

  if (BMX_2_exists) {
// Output("DB:OBS_BMX2");

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

    // 15 BMX2 Preasure
    strcpy (obs[oidx].sensor[sidx].id, "bp2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = p;
    obs[oidx].sensor[sidx++].inuse = true;

    // 16 BMX2 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "bt2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;

    // 17 BMX2 Humidity
    if (BMX_2_type == BMX_TYPE_BME280) {
      strcpy (obs[oidx].sensor[sidx].id, "bh2");
      obs[oidx].sensor[sidx].type = F_OBS;
      obs[oidx].sensor[sidx].f_obs = h;
      obs[oidx].sensor[sidx++].inuse = true;
    }
// Output("DB:OBS_BMX2x");
  }

  if (HTU21DF_exists) {
// Output("DB:OBS_HTU");

    float t = 0.0;
    float h = 0.0;

    // 18 HTU Humidity
    strcpy (obs[oidx].sensor[sidx].id, "hh1");
    obs[oidx].sensor[sidx].type = F_OBS;
    h = htu.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs[oidx].sensor[sidx].f_obs = h;
    obs[oidx].sensor[sidx++].inuse = true;

    // 19 HTU Temperature
    strcpy (obs[oidx].sensor[sidx].id, "ht1");
    obs[oidx].sensor[sidx].type = F_OBS;
    t = htu.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_HTUx");
  }

  if (SHT_1_exists) {
// Output("DB:OBS_SHT1");

    float t = 0.0;
    float h = 0.0;

    // 20 SHT1 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "st1");
    obs[oidx].sensor[sidx].type = F_OBS;
    t = sht1.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;

    // 21 SHT1 Humidity
    strcpy (obs[oidx].sensor[sidx].id, "sh1");
    obs[oidx].sensor[sidx].type = F_OBS;
    h = sht1.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs[oidx].sensor[sidx].f_obs = h;
    obs[oidx].sensor[sidx++].inuse = true;

    sht1_humid = h; // save for derived observations
// Output("DB:OBS_SHT1x");
  }

  if (SHT_2_exists) {
// Output("DB:OBS_SHT2");

    float t = 0.0;
    float h = 0.0;

    // 22 SHT2 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "st2");
    obs[oidx].sensor[sidx].type = F_OBS;
    t = sht2.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;

    // 23 SHT2 Humidity
    strcpy (obs[oidx].sensor[sidx].id, "sh2");
    obs[oidx].sensor[sidx].type = F_OBS;
    h = sht2.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs[oidx].sensor[sidx].f_obs = h;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_SSHt2x");
  }

  if (HDC_1_exists) {
// Output("DB:OBS_HDC1");

    double t = -999.9;
    double h = -999.9;

    if (hdc1.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0)) {
      t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
      h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
      SystemStatusBits &= ~ SSB_HDC_1;  // Turn Off Bit
    }
    else {
      Output ("ERR:HDC1 Read");
      SystemStatusBits |= SSB_HDC_1;  // Turn On Bit
    }

    // 24 HDC1 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "hdt1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) t;
    obs[oidx].sensor[sidx++].inuse = true;

    // 25 HDC1 Humidity
    strcpy (obs[oidx].sensor[sidx].id, "hdh1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) h;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_HDC1x");

  }

  if (HDC_2_exists) {
// Output("DB:OBS_HDC2");

    double t = -999.9;
    double h = -999.9;

    if (hdc2.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0)) {
      t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
      h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
      SystemStatusBits &= ~ SSB_HDC_2;  // Turn Off Bit
    }
    else {
      Output ("ERR:HDC1 Read");
      SystemStatusBits |= SSB_HDC_2;  // Turn On Bit
    }

    // 26 HDC2 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "hdt2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) t;
    obs[oidx].sensor[sidx++].inuse = true;

    // 27 HDC2 Humidity
    strcpy (obs[oidx].sensor[sidx].id, "hdh2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) h;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_HDC2x");

  }

  if (LPS_1_exists) {
// Output("DB:OBS_LPS1");
    float t = lps1.readTemperature();
    float p = lps1.readPressure();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;

    // 28 LPS1 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "lpt1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) t;
    obs[oidx].sensor[sidx++].inuse = true;

    // 29 LPS1 Pressure
    strcpy (obs[oidx].sensor[sidx].id, "lpp1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) p;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_LPS1x");
  }

  if (LPS_2_exists) {
// Output("DB:OBS_LPS2");
    float t = lps2.readTemperature();
    float p = lps2.readPressure();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;

    // 30 LPS1 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "lpt2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) t;
    obs[oidx].sensor[sidx++].inuse = true;

    // 31 LPS1 Pressure
    strcpy (obs[oidx].sensor[sidx].id, "lpp2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) p;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_LPS2x");
  }

  if (HIH8_exists) {
// Output("DB:OBS_HIH");
    float t = 0.0;
    float h = 0.0;

    bool status = hih8_getTempHumid(&t, &h);
    if (!status) {
      t = -999.99;
      h = 0.0;
    }
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;

    // 32 HIH8 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "ht2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;

    // 33 HIH8 Humidity
    strcpy (obs[oidx].sensor[sidx].id, "hh2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = h;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_HIHx");
  }

  if (SI1145_exists) {
// Output("DB:OBS_SII");
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

    // 34 SI Visible
    strcpy (obs[oidx].sensor[sidx].id, "sv1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = si_vis;
    obs[oidx].sensor[sidx++].inuse = true;

    // 35 SI IR
    strcpy (obs[oidx].sensor[sidx].id, "si1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = si_ir;
    obs[oidx].sensor[sidx++].inuse = true;

    // 36 SI UV
    strcpy (obs[oidx].sensor[sidx].id, "su1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = si_uv;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_SIIx");
  }

  if (MCP_1_exists) {
// Output("DB:OBS_MCP1");
    float t = 0.0;

    // 37 MCP1 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "mt1");
    obs[oidx].sensor[sidx].type = F_OBS;
    t = mcp1.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;

    mcp1_temp = t; // save for derived observations
// Output("DB:OBS_MCP1x");
  }

  if (MCP_2_exists) {
// Output("DB:OBS_MCP2");
    float t = 0.0;

    // 38 MCP2 Temperature
    strcpy (obs[oidx].sensor[sidx].id, "mt2");
    obs[oidx].sensor[sidx].type = F_OBS;
    t = mcp2.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_MCP2x");
  }

  if (MCP_3_exists) {
// Output("DB:OBS_MCP3");
    float t = 0.0;

    // 39 MCP3 Globe Temperature
    strcpy (obs[oidx].sensor[sidx].id, "gt1");
    obs[oidx].sensor[sidx].type = F_OBS;
    t = mcp3.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;

    mcp3_temp = t; // globe temperature
// Output("DB:OBS_MCP3x");
  }

  if (MCP_4_exists) {
// Output("DB:OBS_MCP4");
    float t = 0.0;

    // 40 MCP4 Globe Temperature
    strcpy (obs[oidx].sensor[sidx].id, "gt2");
    obs[oidx].sensor[sidx].type = F_OBS;
    t = mcp4.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs[oidx].sensor[sidx].f_obs = t;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_MCP4x");
  }

  if (VEML7700_exists) {
// Output("DB:OBS_VEML");
    float lux = veml.readLux(VEML_LUX_AUTO);
    lux = (isnan(lux) || (lux < QC_MIN_VLX)  || (lux > QC_MAX_VLX))  ? QC_ERR_VLX  : lux;

    // 41 VEML7700 Auto Lux Value
    strcpy (obs[oidx].sensor[sidx].id, "vlx");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = lux;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_VEMLx");
  }

  if (BLX_exists) {
// Output("DB:OBS_BLX");
    float lux=blx_takereading();
    lux = (isnan(lux) || (lux < QC_MIN_BLX)  || (lux > QC_MAX_BLX))  ? QC_ERR_BLX  : lux;

    // 42 DFR BLUX30 Auto Lux Value
    strcpy (obs[oidx].sensor[sidx].id, "blx");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = lux;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_BLXx");
  }

  if (A4_State == A4_STATE_DISTANCE) {
// Output("DB:OBS_A4D");
    // 43 Distance Guage
    strcpy (obs[oidx].sensor[sidx].id, "sg"); // sg = snow or stream
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = DistanceGauge_Median();
    obs[oidx].sensor[sidx++].inuse = true;
  }
  if (A4_State == A4_STATE_RAW) {
// Output("DB:OBS_A4R");
    // 44 A4 Raw
    strcpy (obs[oidx].sensor[sidx].id, "a4r");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = Pin_ReadAvg(A4);
    obs[oidx].sensor[sidx++].inuse = true;
  }
  else if (A4_State == A4_STATE_RAIN) {
// Output("DB:OBS_A4R");
    // 45 Rain Guage 2
    strcpy (obs[oidx].sensor[sidx].id, "rg2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = rain2;
    obs[oidx].sensor[sidx++].inuse = true;

    // 46 Rain Gauge 2 Total - Not Implemented
    strcpy (obs[oidx].sensor[sidx].id, "rgt2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = eeprom.rgt2;
    obs[oidx].sensor[sidx++].inuse = true;

    // 47 Rain Gauge 2 Prior Day - Not Implemented
    strcpy (obs[oidx].sensor[sidx].id, "rgp2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = eeprom.rgp2;
    obs[oidx].sensor[sidx++].inuse = true;
  }

  if (A5_State == A5_STATE_RAW) {
// Output("DB:OBS_A5R");
    // 44 A4 Raw
    strcpy (obs[oidx].sensor[sidx].id, "a5r");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = Pin_ReadAvg(A5);
    obs[oidx].sensor[sidx++].inuse = true;
  }

  if (PM25AQI_exists) {
// Output("DB:OBS_PM");
    // 49 Standard Particle PM1.0 concentration unit µg 𝑚3
    strcpy (obs[oidx].sensor[sidx].id, "pm1s10");
    obs[oidx].sensor[sidx].type = I_OBS;
    obs[oidx].sensor[sidx].i_obs = pm25aqi_obs.max_s10;
    obs[oidx].sensor[sidx++].inuse = true;

    // 50 Standard Particle PM2.5 concentration unit µg 𝑚3
    strcpy (obs[oidx].sensor[sidx].id, "pm1s25");
    obs[oidx].sensor[sidx].type = I_OBS;
    obs[oidx].sensor[sidx].i_obs = pm25aqi_obs.max_s25;
    obs[oidx].sensor[sidx++].inuse = true;

    // 51 Standard Particle PM10.0 concentration unit µg 𝑚3
    strcpy (obs[oidx].sensor[sidx].id, "pm1s100");
    obs[oidx].sensor[sidx].type = I_OBS;
    obs[oidx].sensor[sidx].i_obs = pm25aqi_obs.max_s100;
    obs[oidx].sensor[sidx++].inuse = true;

    // 52 Atmospheric Environmental PM1.0 concentration unit µg 𝑚3
    strcpy (obs[oidx].sensor[sidx].id, "pm1e10");
    obs[oidx].sensor[sidx].type = I_OBS;
    obs[oidx].sensor[sidx].i_obs = pm25aqi_obs.max_e10;
    obs[oidx].sensor[sidx++].inuse = true;

    // 53 Atmospheric Environmental PM2.5 concentration unit µg 𝑚3
    strcpy (obs[oidx].sensor[sidx].id, "pm1e25");
    obs[oidx].sensor[sidx].type = I_OBS;
    obs[oidx].sensor[sidx].i_obs = pm25aqi_obs.max_e25;
    obs[oidx].sensor[sidx++].inuse = true;

    // 54  Atmospheric Environmental PM10.0 concentration unit µg 𝑚3
    strcpy (obs[oidx].sensor[sidx].id, "pm1e100");
    obs[oidx].sensor[sidx].type = I_OBS;
    obs[oidx].sensor[sidx].i_obs = pm25aqi_obs.max_e100;
    obs[oidx].sensor[sidx++].inuse = true;

    // Clear readings
    pm25aqi_clear();
// Output("DB:OBS_PMx");
  }

  // 55 Heat Index Temperature
  if (HI_exists) {
// Output("DB:OBS_HI");
    heat_index = hi_calculate(mcp1_temp, sht1_humid);
    strcpy (obs[oidx].sensor[sidx].id, "hi");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) heat_index;
    obs[oidx].sensor[sidx++].inuse = true;    
  } 

  // 56 Wet Bulb Temperature
  if (WBT_exists) {
// Output("DB:OBS_WBT");
    wetbulb_temp = wbt_calculate(mcp1_temp, sht1_humid);
    strcpy (obs[oidx].sensor[sidx].id, "wbt");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) wetbulb_temp;
    obs[oidx].sensor[sidx++].inuse = true;  
// Output("DB:OBS_WBTx");  
  }

  // 57 Wet Bulb Globe Temperature
  if (WBGT_exists) {
// Output("DB:OBS_WBGT");
    float wbgt = 0.0;
    if (MCP_3_exists) {
      wbgt = wbgt_using_wbt(mcp1_temp, mcp3_temp, wetbulb_temp); // TempAir, TempGlobe, TempWetBulb
    }
    else {
      wbgt = wbgt_using_hi(heat_index);
    }
    strcpy (obs[oidx].sensor[sidx].id, "wbgt");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) wbgt;
    obs[oidx].sensor[sidx++].inuse = true;    
// Output("DB:OBS_WBGTx");
  }

  // 58,59 Tinovi Leaf Wetness
  if (TLW_exists) {
// Output("DB:OBS_TLW");
    tlw.newReading();
    delay(100);
    float w = tlw.getWet();
    float t = tlw.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    strcpy (obs[oidx].sensor[sidx].id, "tlww");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) w;
    obs[oidx].sensor[sidx++].inuse = true; 

    strcpy (obs[oidx].sensor[sidx].id, "tlwt");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) t;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_TLWx");
  }

  // 60-63 Tinovi Soil Moisture
  if (TSM_exists) {
// Output("DB:OBS_TSM");
    tsm.newReading();
    delay(100);
    float e25 = tsm.getE25();
    float ec = tsm.getEC();
    float vwc = tsm.getVWC();
    float t = tsm.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    strcpy (obs[oidx].sensor[sidx].id, "tsme25");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) e25;
    obs[oidx].sensor[sidx++].inuse = true;

    strcpy (obs[oidx].sensor[sidx].id, "tsmec");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) ec;
    obs[oidx].sensor[sidx++].inuse = true;

    strcpy (obs[oidx].sensor[sidx].id, "tsmvwc");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) vwc;
    obs[oidx].sensor[sidx++].inuse = true; 

    strcpy (obs[oidx].sensor[sidx].id, "tsmt");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) t;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_TSMx");
  }

   // 64-69 Tinovi Multi Level Soil Moisture
  if (TMSM_exists) {
// Output("DB:OBS_TMSM");
    soil_ret_t multi;
    float t;

    tmsm.newReading();
    delay(100);
    tmsm.getData(&multi);

    strcpy (obs[oidx].sensor[sidx].id, "tmsms1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) multi.vwc[0];
    obs[oidx].sensor[sidx++].inuse = true;

    strcpy (obs[oidx].sensor[sidx].id, "tmsms2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) multi.vwc[1];
    obs[oidx].sensor[sidx++].inuse = true;

    strcpy (obs[oidx].sensor[sidx].id, "tmsms3");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) multi.vwc[2];
    obs[oidx].sensor[sidx++].inuse = true;

    strcpy (obs[oidx].sensor[sidx].id, "tmsms4");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) multi.vwc[3];
    obs[oidx].sensor[sidx++].inuse = true;

    t = multi.temp[0];
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    strcpy (obs[oidx].sensor[sidx].id, "tmsmt1");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) t;
    obs[oidx].sensor[sidx++].inuse = true;

    t = multi.temp[1];
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    strcpy (obs[oidx].sensor[sidx].id, "tmsmt2");
    obs[oidx].sensor[sidx].type = F_OBS;
    obs[oidx].sensor[sidx].f_obs = (float) t;
    obs[oidx].sensor[sidx++].inuse = true;
// Output("DB:OBS_TMSMx");
  } 

  // Set this after we read all sensors. So we capture if their state changes 
  obs[oidx].hth = SystemStatusBits;

  // Save Observation to SD Card
  OBS_Log(oidx);

  lastOBS = System.millis();
// Output("DB:OBS_Exit");
}

/*
 * ======================================================================================================================
 * Particle_Publish() - Publish to Particle what is in msgbuf
 * ======================================================================================================================
 */
bool Particle_Publish(char *EventName) {
  // Calling Particle.publish() when the cloud connection has been turned off will not publish an event. 
  // This is indicated by the return success code of false. If the cloud connection is turned on and 
  // trying to connect to the cloud unsuccessfully, Particle.publish() may block for up to 20 seconds 
  // (normal conditions) to 10 minutes (unusual conditions). Checking Particle.connected() 
  // before calling Particle.publish() can help prevent this.
  if (Particle.connected()) {
    uint64_t start_ts = System.millis();
    if (Particle.publish(EventName, msgbuf, WITH_ACK)) {  // PRIVATE flag is always used even when not specified

      // A safty check, If we got hung up for N seconds or more sending, 
      // let's invalidate our wind data and force a reinit in the main loop()
      if ((System.millis() - start_ts) >= 2000) {
        ws_refresh = true;
      }
      // Currently, a device can publish at rate of about 1 event/sec, with bursts of up to 4 allowed in 1 second. 
      // Back to back burst of 4 messages will take 4 seconds to recover.
      // Our BackGroundWork() provides a delay between observations of 1 second 
      BackGroundWork();
      return(true);
    }
  }
  else {
    Output ("Particle:NotReady");
  }
  return(false);
}

/*
 * ======================================================================================================================
 * OBS_FS_Publish() - obs[i].inuse for this observation must be true prior to calling
 * ======================================================================================================================
 */
bool OBS_FS_Publish(int i) {
  OBS_FS_Build_JSON(i);  
  if (Particle_Publish((char *) "FS")) {
    Serial_write (msgbuf);
    sprintf (Buffer32Bytes, "FS[%d]->PUB OK[%d]", i, strlen(msgbuf)+1);
    Output(Buffer32Bytes);
    return(true);
  }
  else {
    sprintf (Buffer32Bytes, "FS[%d]->PUB ERR", i);
    Output(Buffer32Bytes);
    return(false);
  }
}

/*
 * ======================================================================================================================
 * OBS_Relay_Publish()
 * ======================================================================================================================
 */
bool OBS_Relay_Publish(int relay_type) {
  if (relay_type>0) {  // little safty check. Should not be 0
    Serial_write (msgbuf);
    if (Particle_Publish((char *) relay_msgtypes[relay_type])) {
      sprintf (Buffer32Bytes, "RELAY[%s]->PUB OK", relay_msgtypes[relay_type]);
      Output(Buffer32Bytes);
      return(true);
    }
    else {
      sprintf (Buffer32Bytes, "RELAY[%s]->PUB ERR", relay_msgtypes[relay_type]);
      Output(Buffer32Bytes);       
      return(false);
    }
  }
  else {
    sprintf (Buffer32Bytes, "RELAY TYPE[%d] INVALID", relay_type);
    Output(Buffer32Bytes);
    return(false);
  }
}

/*
 * ======================================================================================================================
 * OBS_PublishAll() - Send to logging site
 * 
 * If we take to long sending observations code restarts the 60 samples for wind and direction.
 * So we might miss a one minute observation
 * 
 * Function Particle_Publish() calls BackGroundWork() after each transmission
 * 
 * If sending an observation takes 2 or more seconds ws_refresh is set and 
 *   the main program loop calls Wind_Distance_Air_Initialize() 
 * ======================================================================================================================
 */
void OBS_PublishAll() {
  bool OK2Send=true;
  int relay_type;

  // Update Cell Signal Strength On Last (Most Current) OBS Since Cell is turned to get reading
  int last = OBS_Last();
  if (last >= 0) {
#if PLATFORM_ID == PLATFORM_ARGON
    WiFiSignal sig = WiFi.RSSI();
#else
    CellularSignal sig = Cellular.RSSI();
#endif
    obs[last].css = sig.getStrength();
  }

  // Go through the saved 1 minute observers and send them
  for (int i=0; i<MAX_ONE_MINUTE_OBS; i++) {
    if (obs[i].inuse) {
      if (OBS_FS_Publish(i) == false) {
        OBS_N2S_Add (i);
        // Don't try to send any N2S because we just added to the file
        OK2Send = false;
      }
      OBS_Clear(i);
    }
  }

  // Publish LoRa Relay Observations   
  if (LORA_exists) {
    // We want to transmit all the LoRa                                                                                                                  msgs or save them to N2S file if we can not transmit them.
    while (lora_relay_need2log()) {
      relay_type = OBS_Relay_Build_JSON(); // This removed msg from relay structure and places it in msgbuf
      if (relay_type<=0) {
        sprintf (Buffer32Bytes, "RELAY TYPE[%d] INVALID", relay_type);
        Output(Buffer32Bytes);
      }
      else {
        if (OK2Send && (relay_type>0)) {
         OK2Send = OBS_Relay_Publish(relay_type);  // Note a new LoRa RS msgs could be received as we are sending    
        }
        if (!OK2Send) {
          sprintf (msgbuf+strlen(msgbuf), ",%s", relay_msgtypes[relay_type]);  // Add Particle Event Type after JSON structure
          SD_NeedToSend_Add(msgbuf); // Save to N2F File
          Output("RELAY->N2S");
          Serial_write (msgbuf); 
        }
      }
    }
  }

  // Check if we have any N2S only if we have not added to the file while trying to send OBS
  if (OK2Send) {
    SD_N2S_Publish(); 
  }
}


