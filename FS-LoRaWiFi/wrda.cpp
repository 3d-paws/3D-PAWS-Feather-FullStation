/*
 * ======================================================================================================================
 *  wrda.cpp - Wind Rain Distance Functions
 * ======================================================================================================================
 */
#include <Arduino.h>

#include "include/ssbits.h"
#include "include/output.h"
#include "include/cf.h"
#include "include/eeprom.h"
#include "include/sdcard.h"
#include "include/time.h"
#include "include/output.h"
#include "include/feather.h"
#include "include/support.h"
#include "include/sensors.h"
#include "include/main.h"
#include "include/wrda.h"

 /*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Wind
 * ======================================================================================================================
 */
WIND_STR wind;

/*
 * ======================================================================================================================
 *  Wind Direction - AS5600 Sensor
 * ======================================================================================================================
 */
bool      AS5600_exists     = true;
int       AS5600_ADR        = 0x36;
const int AS5600_raw_ang_hi = 0x0c;
const int AS5600_raw_ang_lo = 0x0d;

/*
 * ======================================================================================================================
 *  Wind Speed Calibration
 * ======================================================================================================================
 */
float ws_calibration = 2.64;       // From wind tunnel testing
float ws_radius = 0.079;           // In meters

bool ws_refresh = false;           // Set to true when we have delayed too long sending observations

/*
 * ======================================================================================================================
 *  Optipolar Hall Effect Sensor SS451A - Interrupt 1 - Anemometer
 * ======================================================================================================================
 */
volatile unsigned int anemometer_interrupt_count;
unsigned long anemometer_interrupt_stime;

/*
 * ======================================================================================================================
 *  Rain Gauge 1 - Optipolar Hall Effect Sensor SS451A
 * ======================================================================================================================
 */
volatile unsigned int raingauge1_interrupt_count;
uint64_t raingauge1_interrupt_stime; // Send Time
uint64_t raingauge1_interrupt_ltime; // Last Time
uint64_t raingauge1_interrupt_toi;   // Time of Interrupt

/*
 * ======================================================================================================================
 *  Rain Gauge 2 - Optipolar Hall Effect Sensor SS451A
 * ======================================================================================================================
 */
volatile unsigned int raingauge2_interrupt_count;
uint64_t raingauge2_interrupt_stime; // Send Time
uint64_t raingauge2_interrupt_ltime; // Last Time
uint64_t raingauge2_interrupt_toi;   // Time of Interrupt

/*
 * =======================================================================================================================
 *  Distance Gauge
 * =======================================================================================================================
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
 *  anemometer_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void anemometer_interrupt_handler()
{
  anemometer_interrupt_count++;
}

/*
 * ======================================================================================================================
 *  raingauge1_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void raingauge1_interrupt_handler()
{
  if ((millis() - raingauge1_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    digitalWrite(LED_PIN, HIGH);
    raingauge1_interrupt_ltime = millis();
    raingauge1_interrupt_count++;
    TurnLedOff = true;
  }   
}

/*
 * ======================================================================================================================
 *  raingauge2_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void raingauge2_interrupt_handler()
{
  if ((millis() - raingauge2_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    digitalWrite(LED_PIN, HIGH);
    raingauge2_interrupt_ltime = millis();
    raingauge2_interrupt_count++;
    TurnLedOff = true;
  }   
}

/* 
 *=======================================================================================================================
 * Wind_SampleSpeed() - Return a wind speed based on interrupts and duration wind
 * 
 * Optipolar Hall Effect Sensor SS451A - Anemometer
 * speed  = (( (signals/2) * (2 * pi * radius) ) / time) * calibration_factor
 * speed in m/s =  (   ( (interrupts/2) * (2 * 3.14156 * 0.079) )  / (time_period in ms / 1000)  )  * 2.64
 *=======================================================================================================================
 */
float Wind_SampleSpeed() {
  unsigned long delta_ms, time_ms;
  float wind_speed;
  
  time_ms = millis();

  // Handle the clock rollover after about 50 days. (Should not be an issue since we reboot every day)
  if (time_ms < anemometer_interrupt_stime) {
    delta_ms = (0xFFFFFFFF - anemometer_interrupt_stime) + time_ms;
  }
  else {
    delta_ms = millis()-anemometer_interrupt_stime;
  }
  
  if (anemometer_interrupt_count && (delta_ms>0)) {
    wind_speed = ( ( anemometer_interrupt_count * 3.14156 * ws_radius)  / 
      (float)( (float)delta_ms / 1000) )  * ws_calibration;
  }
  else {
    wind_speed = 0.0;
  }

  anemometer_interrupt_count = 0;
  anemometer_interrupt_stime = millis(); 
  return (wind_speed);
} 

/* 
 *=======================================================================================================================
 * Wind_SampleDirection() -- Talk i2c to the AS5600 sensor and get direction
 *=======================================================================================================================
 */
int Wind_SampleDirection() {
  int degree;
  
  // Read Raw Angle Low Byte
  Wire.beginTransmission(AS5600_ADR);
  Wire.write(AS5600_raw_ang_lo);
  if (Wire.endTransmission()) {
    Output ("WD RD_LOW_ERR");
  }
  else if (Wire.requestFrom(AS5600_ADR, 1)) {
    int AS5600_lo_raw = Wire.read();
  
    // Read Raw Angle High Byte
    Wire.beginTransmission(AS5600_ADR);
    Wire.write(AS5600_raw_ang_hi);
    if (Wire.endTransmission()) {
      Output ("WD RD_HI_ERR");
    }
    else if (Wire.requestFrom(AS5600_ADR, 1)) {
      word AS5600_hi_raw = Wire.read();
      AS5600_hi_raw = AS5600_hi_raw << 8; //shift raw angle hi 8 left
      AS5600_hi_raw = AS5600_hi_raw | AS5600_lo_raw; //AND high and low raw angle value

      // Do data integ check
      degree = (int) AS5600_hi_raw * 0.0879;
      if ((degree >=0) && (degree <= 360)) {
        return (degree);
      }
      else {
        return (-1);
      }
    }
  }
  return (-1); // Not the best value to return 
}

/* 
 *=======================================================================================================================
 * Wind_DirectionVector()
 *=======================================================================================================================
 */
int Wind_DirectionVector() {
  double NS_vector_sum = 0.0;
  double EW_vector_sum = 0.0;
  double r;
  float s;
  int d, i, rtod;
  bool ws_zero = true;

  for (i=0; i<WIND_READINGS; i++) {
    d = wind.bucket[i].direction;

    // if at any time 1 of the 60 wind direction readings is -1
    // then the sensor was offline and we need to invalidate or data
    // until it is clean with out any -1's
    if (d == -1) {
      return (-1);
    }
    
    s = wind.bucket[i].speed;

    // Flag we have wind speed
    if (s > 0) {
      ws_zero = false;  
    }
    r = (d * 71) / 4068.0;
    
    // North South Direction 
    NS_vector_sum += cos(r) * s;
    EW_vector_sum += sin(r) * s;
  }
  rtod = (atan2(EW_vector_sum, NS_vector_sum)*4068.0)/71.0;
  if (rtod<0) {
    rtod = 360 + rtod;
  }

  // If all the winds speeds are 0 then we return current wind direction or 0 on failure of that.
  if (ws_zero) {
    return (Wind_SampleDirection()); // Can return -1
  }
  else {
    return (rtod);
  }
}

/* 
 *=======================================================================================================================
 * Wind_SpeedAverage()
 *=======================================================================================================================
 */
float Wind_SpeedAverage() {
  float wind_speed = 0.0;
  for (int i=0; i<WIND_READINGS; i++) {
    // sum wind speeds for later average
    wind_speed += wind.bucket[i].speed;
  }
  return( wind_speed / (float) WIND_READINGS);
}

/* 
 *=======================================================================================================================
 * Wind_Gust()
 *=======================================================================================================================
 */
float Wind_Gust() {
  return(wind.gust);
}

/* 
 *=======================================================================================================================
 * Wind_GustDirection()
 *=======================================================================================================================
 */
int Wind_GustDirection() {
  return(wind.gust_direction);
}

/* 
 *=======================================================================================================================
 * Wind_GustUpdate()
 *   Wind Gust = Highest 3 consecutive samples from the 60 samples. The 3 samples are then averaged.
 *   Wind Gust Direction = Average of the 3 Vectors from the Wind Gust samples.
 * 
 *   Note: To handle the case of 2 or more gusts at the same speed but different directions
 *          Sstart with oldest reading and work forward to report most recent.
 * 
 *   Algorithm: 
 *     Start with oldest reading.
 *     Sum this reading with next 2.
 *     If greater than last, update last 
 * 
 *=======================================================================================================================
 */
void Wind_GustUpdate() {
  int bucket = wind.bucket_idx; // Start at next bucket to fill (aka oldest reading)
  float ws_sum = 0.0;
  int ws_bucket = bucket;
  float sum;

  for (int i=0; i<(WIND_READINGS-2); i++) {  // subtract 2 because we are looking ahead at the next 2 buckets
    // sum wind speeds 
    sum = wind.bucket[bucket].speed +
          wind.bucket[(bucket+1) % WIND_READINGS].speed +
          wind.bucket[(bucket+2) % WIND_READINGS].speed;
    if (sum >= ws_sum) {
      ws_sum = sum;
      ws_bucket = bucket;
    }
    bucket = (++bucket) % WIND_READINGS;
  }
  wind.gust = ws_sum/3;
  
  // Determine Gust Direction 
  double NS_vector_sum = 0.0;
  double EW_vector_sum = 0.0;
  double r;
  float s;
  int d, i, rtod;
  bool ws_zero = true;

  bucket = ws_bucket;
  for (i=0; i<3; i++) {
    d = wind.bucket[bucket].direction;

    // if at any time any wind direction readings is -1
    // then the sensor was offline and we need to invalidate or data
    // until it is clean with out any -1's
    if (d == -1) {
      ws_zero = true;
      break;
    }
    
    s = wind.bucket[bucket].speed;

    // Flag we have wind speed
    if (s > 0) {
      ws_zero = false;  
    }
    r = (d * 71) / 4068.0;
    
    // North South Direction 
    NS_vector_sum += cos(r) * s;
    EW_vector_sum += sin(r) * s;

    bucket = (++bucket) % WIND_READINGS;
  }

  rtod = (atan2(EW_vector_sum, NS_vector_sum)*4068.0)/71.0;
  if (rtod<0) {
    rtod = 360 + rtod;
  }

  // If all the winds speeds are 0 or we has a -1 direction then set -1 dor direction.
  if (ws_zero) {
    wind.gust_direction = -1;
  }
  else {
    wind.gust_direction = rtod;
  }
}

/*
 * ======================================================================================================================
 * Wind_ClearSampleCount()    
 * ======================================================================================================================
 */
void Wind_ClearSampleCount() {
  wind.sample_count = 0;
}

/*
 * ======================================================================================================================
 * Wind_TakeReading() - Wind direction and speed, measure every second             
 * ======================================================================================================================
 */
void Wind_TakeReading() {
  wind.bucket[wind.bucket_idx].direction = (int) Wind_SampleDirection();
  wind.bucket[wind.bucket_idx].speed = Wind_SampleSpeed();
  wind.bucket_idx = (++wind.bucket_idx) % WIND_READINGS; // Advance bucket index for next reading
  wind.sample_count++;
}

/* 
 *=======================================================================================================================
 * as5600_initialize() - wind direction sensor
 *=======================================================================================================================
 */
void as5600_initialize() {
  Output("AS5600:INIT");
  Wire.beginTransmission(AS5600_ADR);
  if (Wire.endTransmission()) {
    msgp = (char *) "WD:NF";
    AS5600_exists = false;
  }
  else {
    msgp = (char *) "WD:OK";
  }
  Output (msgp);
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
 * VoltaicVoltage() - Breakout the UCB-C 
 * 
 * Newer V25/V50/V75 firmware (post-2023) moved this cell voltage signal to SBU pins (A8/B8) on USB-C for better USB 
 * compliance. Older units used D+. Which conflicted with data transfer. 
 * 
 * Using both A8/B8 SBU pins ensures compatibility regardless of USB-C cable orientation (flipped or not)
 * 
 * The expected voltage range on Voltaic V25's monitor pins (D+ on older firmware, SBU1 (A8) and SBU2 (B8) on newer) is 1.6V to 2.1V
 *=======================================================================================================================
 */
float VoltaicVoltage(int pin) {
  int numReadings = 5;
  int totalValue = 0;
  for (int i = 0; i < numReadings; i++) {
    totalValue += analogRead(pin);
    delay(10);  // Short delay between readings
  }
  float voltage = (3.3 * (totalValue / numReadings)) / 1023.0; 
  return(voltage);
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
 * Wind_Distance_Air_Initialize()
 *=======================================================================================================================
 */
void Wind_Distance_Air_Initialize() {
  Output (F("WDA:Init()"));

  // Clear windspeed counter
  if (AS5600_exists) {
    anemometer_interrupt_count = 0;
    anemometer_interrupt_stime = millis();
  
    // Init default values.
    wind.gust = 0.0;
    wind.gust_direction = -1;
    wind.bucket_idx = 0;
  }

  // Take N 1s samples of wind speed and direction and fill arrays with values.
  if (AS5600_exists | PM25AQI_exists | (cf_op1 == 5) || (cf_op1 == 10)) {
    for (int i=0; i< WIND_READINGS; i++) {
      BackGroundWork(); // AirQ, Wind, Dist polls
    
      if (SerialConsoleEnabled) Serial.print(".");  // Provide Serial Console some feedback as we loop and wait til next observation
      OLED_spin();
    }
    if (SerialConsoleEnabled) Serial.println();  // Send a newline out to cleanup after all the periods we have been logging
  }

  // Now we have N readings we can compute other wind related global varibles

  if (AS5600_exists) {
    Wind_TakeReading();
    ws_refresh = false; // Set to false since we have just initialized wind speed data.

    float ws = Wind_SpeedAverage();
    sprintf (Buffer32Bytes, "WS:%d.%02d WD:%d", (int)ws, (int)(ws*100)%100, Wind_DirectionVector());
    Output (Buffer32Bytes);
  }
  
  if (PM25AQI_exists) {
    sprintf (Buffer32Bytes, "pm1s10:%d", pm25aqi_obs.s10);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1s25:%d", pm25aqi_obs.s25);
    Output (Buffer32Bytes); 
    
    sprintf (Buffer32Bytes, "pm1s100:%d", pm25aqi_obs.s100);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1e10:%d", pm25aqi_obs.e10);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1e25:%d", pm25aqi_obs.e25);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1e100:%d", pm25aqi_obs.e100);
    Output (Buffer32Bytes);
  }

  if ((cf_op1 == 5) || (cf_op1 == 10)) {
    sprintf (Buffer32Bytes, "DS:%d", DS_Median());
    Output (Buffer32Bytes);
  }
}

/* 
 *=======================================================================================================================
 * OPT_AQS_Initialize() - Check SD Card for file to determine if we are a Air Quality Station
 *=======================================================================================================================
 */
void OPT_AQS_Initialize() {
  Output ("OBSAQS:INIT");
  if (SD_exists) {
    if (SD.exists(SD_OPTAQS_FILE)) {
      Output ("OPTAQS Enabled");

      // Ware are a Air Quality Station so Clear Rain Totals from EEPROM
      uint32_t current_time = rtc_unixtime();
      EEPROM_ClearRainTotals(current_time);

      pinMode (OP2_PIN, OUTPUT);
      digitalWrite(OP2_PIN, HIGH); // Turn on Air Quality Sensor

      // We will only go in AQS mode if the sensor is truely there
      AQS_Enabled = true;
      AQS_Correction = (AQSWarmUpTime + 10) * 1000;  // In ms. Correction to be subtracted from mainloop poll interval 
                                                     // to account for the AQS warmup time and 10s for sampling
    }
    else {
      Output ("OPTAQS NF");
      AQS_Enabled = false;
    }
  }
}