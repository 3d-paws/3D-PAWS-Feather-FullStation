/*
 * ======================================================================================================================
 *  sensor_rgop.cpp - Rain and OptionPin Functions
 * ======================================================================================================================
 */
#include <Arduino.h>

#include "include/qc.h"
#include "include/output.h"
#include "include/nvcf.h"
#include "include/eeprom.h"
#include "include/sdcard.h"
#include "include/time.h"
#include "include/output.h"
#include "include/feather.h"
#include "include/support.h"
#include "include/sensor_pm25aqi.h"
#include "include/obs.h"
#include "include/main.h"
#include "include/sensor_rgop.h"

 /*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
volatile bool TurnLedOff = false;                 // Set true in rain gauge interrupt

/*
 * ======================================================================================================================
 *  Rain Gauge 1 - Optipolar Hall Effect Sensor SS451A
 * ======================================================================================================================
 */
volatile unsigned int raingauge1_interrupt_count=0;
uint64_t raingauge1_interrupt_stime; // Send Time
volatile uint64_t raingauge1_interrupt_ltime; // Last Time
uint64_t raingauge1_interrupt_toi;   // Time of Interrupt

/*
 * ======================================================================================================================
 *  Rain Gauge 2 - Optipolar Hall Effect Sensor SS451A
 * ======================================================================================================================
 */
volatile unsigned int raingauge2_interrupt_count=0;
uint64_t raingauge2_interrupt_stime; // Send Time
volatile uint64_t raingauge2_interrupt_ltime; // Last Time
uint64_t raingauge2_interrupt_toi;   // Time of Interrupt

/*
 * =======================================================================================================================
 *  Distance Gauge
 * =======================================================================================================================
 */
/*
 * Distance Sensors
 * The 5-meter sensors (MB7360, MB7369, MB7380, and MB7389) use a scale factor of (Vcc/5120) per 1-mm.
 * Particle 12bit resolution (0-4095),  Sensor has a resolution of 0 - 5119mm,  Each unit of the 0-4095 resolution is 1.25mm
 * Feather has 10bit resolution (0-1023), Sensor has a resolution of 0 - 5119mm, Each unit of the 0-1023 resolution is 5mm
 *
 * The 10-meter sensors (MB7363, MB7366, MB7383, and MB7386) use a scale factor of (Vcc/10240) per 1-mm.
 * Particle 12bit resolution (0-4095), Sensor has a resolution of 0 - 10239mm, Each unit of the 0-4095 resolution is 2.5mm
 * Feather has 10bit resolution (0-1023), Sensor has a resolution of 0 - 10239mm, Each unit of the 0-1023 resolution is 10mm
 *
 * The distance sensor will report as type sg  for Snow, Stream, or Surge gauge deployments.
 * A Median value based on 60 samples 250ms apart is obtain. Then subtracted from ds_baseline for the observation.
 */
unsigned int dg_bucket = 0;
unsigned int dg_resolution_adjust = 2.5;                 // Default (2.5) is 10m sensor, (5 = 5m sensor)
unsigned int dg_buckets[DG_BUCKETS];

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  raingauge1_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void raingauge1_interrupt_handler()
{
  unsigned long now = millis();
  if ((now - raingauge1_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    raingauge1_interrupt_ltime = now;
    raingauge1_interrupt_count++;
    digitalWrite(LED_PIN, HIGH);
    TurnLedOff = true;
  }
}

/*
 * ======================================================================================================================
 *  raingauge1_sample() - return rain amount since last sample
 * ======================================================================================================================
 */
float raingauge1_sample() {
  unsigned long time_ms, rg1ds, count;
  float rg1;

  noInterrupts();
  time_ms = millis();
  rg1ds = (time_ms - raingauge1_interrupt_stime) / 1000;
  count = raingauge1_interrupt_count;
  raingauge1_interrupt_count = 0;
  raingauge1_interrupt_stime = time_ms;
  raingauge1_interrupt_ltime = 0;
  interrupts();

  rg1 = count * 0.2f;
  rg1 = (isnan(rg1) || (rg1 < QC_MIN_RG) || (rg1 > (((float)rg1ds / 60.0f) * QC_MAX_RG))) ? QC_ERR_RG : rg1;

  return rg1;
}

/*
 * ======================================================================================================================
 *  raingauge2_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void raingauge2_interrupt_handler()
{
  unsigned long now = millis();
  if ((now - raingauge2_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    raingauge2_interrupt_ltime = now;
    raingauge2_interrupt_count++;
    digitalWrite(LED_PIN, HIGH);
    TurnLedOff = true;
  }
}

/*
 * ======================================================================================================================
 *  raingauge2_sample() - return rain amount since last sample
 * ======================================================================================================================
 */
float raingauge2_sample() {
  unsigned long time_ms, rg2ds, count;
  float rg2;

  noInterrupts();
  time_ms = millis();
  rg2ds = (time_ms - raingauge2_interrupt_stime) / 1000;
  count = raingauge2_interrupt_count;
  raingauge2_interrupt_count = 0;
  raingauge2_interrupt_stime = time_ms;
  raingauge2_interrupt_ltime = 0;
  interrupts();

  rg2 = count * 0.2f;
  rg2 = (isnan(rg2) || (rg2 < QC_MIN_RG) || (rg2 > (((float)rg2ds / 60.0f) * QC_MAX_RG))) ? QC_ERR_RG : rg2;

  return rg2;
}

/* 
 * =======================================================================================================================
 * RainEnabled()
 * =======================================================================================================================
 */
bool RainEnabled() {
  if (scv.rg1 || (nv_op1_state() == OP1_STATE_RAIN)) {
    return (true);
  }
  else {
    return (false);
  }
}

/* 
 *=======================================================================================================================
 * Pin_ReadAvg()
 *=======================================================================================================================
 */
float Pin_ReadAvg(int pin) {
  int numReadings = 5;
  int totalValue = 0;
  for (int i = 0; i < numReadings; i++) {
    totalValue += analogRead(pin);
    delay(10);  // Short delay between readings
  }
  return(totalValue / numReadings);
}

/* 
 *=======================================================================================================================
 * VoltaicVoltage() - Breakout the Voltaic Cell Voltage from the UCB-C 
 * 
 * Analog A0-A6	0-3.6V	ADC input range
 * 
 * Info: https://blog.voltaicsystems.com/reading-charge-level-of-voltaic-usb-battery-packs/ for  D+ 
 * Info: https://blog.voltaicsystems.com/updated-usb-c-pd-and-always-on-for-v25-v50-v75-batteries/ for SBU
 * 
 * Newer V25/V50/V75 firmware (post-2023) moved this cell voltage signal to SBU pins (A8/B8) on USB-C for better USB 
 * compliance. Older units used D+. Which conflicted with data transfer. 
 * 
 * Using both A8/B8 SBU pins ensures compatibility regardless of USB-C cable orientation (flipped or not)
 * 
 * Voltaic’s docs say the pack reports half the cell voltage.
 * The expected voltage range on Voltaic V25's monitor pins (D+ on older firmware, 
 *   SBU1 (A8) and SBU2 (B8) on newer) is 1.6V to 2.1V
 *=======================================================================================================================
 */
float VoltaicVoltage(int pin) {
  int numReadings = 5;
  int totalValue = 0;
  for (int i = 0; i < numReadings; i++) {
    totalValue += analogRead(pin);
    delay(10);  // Short delay between readings
  }
  float voltage = (3.3 * (totalValue / (float)numReadings)) / 4095.0; 
  return(voltage);
}

/*
 *=======================================================================================================================
 * VoltaicVoltage() - Voltaic Cell Percent Charge 
 *   Full charge: 4.2V cell → 2.1V on SBU (100%)  
 *   75% charge:  ~3.9V cell → ~1.95V on SBU  
 *   50% charge:  ~3.7V cell → ~1.85V on SBU  
 *   25% charge:  ~3.4V cell → ~1.7V on SBU  
 *   Empty:       3.2V cell → 1.6V on SBU (0%)
 *=======================================================================================================================
 */
float VoltaicPercent(float half_cell_voltage) {
  float cellV = half_cell_voltage * 2.0;
  
  if (cellV >= 4.20) return 100.0;
  if (cellV <= 3.20) return 0.0;
  
  // Simple linear approximation over Voltaic's specified 3.2-4.2V range
  // (Li-ion curve is flat in middle, so voltage alone is rough anyway)
  float percent = ((cellV - 3.20) / (4.20 - 3.20)) * 100.0;
  return constrain(percent, 0, 100);
}

/*
 * ======================================================================================================================
 * DS_TakeReading() - measure every second             
 * ======================================================================================================================
 */
void DS_TakeReading() {
  dg_buckets[dg_bucket] = (int) analogRead(DISTANCE_GAUGE_PIN) * dg_resolution_adjust;
  dg_bucket = (++dg_bucket) % DG_BUCKETS; // Advance bucket index for next reading
}

/* 
 *=======================================================================================================================
 * DS_Median()
 *=======================================================================================================================
 */
float DS_Median() {
  int i;
  
  mysort(dg_buckets, DG_BUCKETS);
  i = (DG_BUCKETS+1) / 2 - 1; // -1 as array indexing in C starts from 0
  
  return (dg_buckets[i]); 
}

/* 
 *=======================================================================================================================
 * OPT_AQS_Initialize() - Check SD Card for file to determine if we are a Air Quality Station
 *=======================================================================================================================
 */
void OPT_AQS_Initialize() {
  if (scv.aqs) {
    Output ("OBSAQS:INIT");
    // Ware are a Air Quality Station so Clear Rain Totals from EEPROM
    uint32_t current_time = rtc_unixtime();
    EEPROM_ClearRainTotals(current_time);

    pinMode (OP2_PIN, OUTPUT);
    digitalWrite(OP2_PIN, HIGH); // Turn on Air Quality Sensor

    AQS_Correction = (AQSWarmUpTime + 10) * 1000;  // In ms. Correction to be subtracted from mainloop poll interval 
                                                     // to account for the AQS warmup time and 10s for sampling
  }
}

/* 
 *=======================================================================================================================
 * raingauge_obs_do() - 
 *=======================================================================================================================
 */
void raingauge_obs_do(int &sidx) {
  if (RainEnabled()) {
    float rg1 = 0.0;
    float rg2 = 0.0;

    // Rain Gauge 1 - Each tip is 0.2mm of rain
    if (scv.rg1) {
      rg1 = raingauge1_sample();
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

    // Rain Gauge 2 - Each tip is 0.2mm of rain
    if (nv_op1_state() == OP1_STATE_RAIN) {
      rg2 = raingauge2_sample();
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
  }
}

/* 
 *=======================================================================================================================
 * op_obs_do() - 
 *=======================================================================================================================
 */
void op_obs_do(int &sidx) {
  if (nv_op1_state() == OP1_STATE_RAW) {
    // OP1 Raw
    strcpy (obs.sensor[sidx].id, "op1r");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = Pin_ReadAvg(OP1_PIN);
    obs.sensor[sidx++].inuse = true;
  }

  if ((nv_op1_state() == OP1_STATE_DIST_5M) || (nv_op1_state() == OP1_STATE_DIST_10M)) {
    float ds_median, ds_median_raw;
    
    ds_median = ds_median_raw = DS_Median();
    if (scv.ds_baseline > 0) {
      ds_median = scv.ds_baseline - ds_median_raw;
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

  if (nv_op2_state() == OP2_STATE_RAW) {
    // OP2 Raw
    strcpy (obs.sensor[sidx].id, "op2r");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = Pin_ReadAvg(OP2_PIN);
    obs.sensor[sidx++].inuse = true;
  }

  if (nv_op2_state() == OP2_STATE_VOLTAIC) {
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
}
