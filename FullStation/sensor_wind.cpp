/*
 * ======================================================================================================================
 *  sensor_wind.cpp - Wind Sensor Functions
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
#include "include/sensor_wind.h"

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
volatile unsigned int anemometer_interrupt_count=0;
uint64_t anemometer_interrupt_stime;

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
 *=======================================================================================================================
 * Wind_SampleSpeed() - Return a wind speed based on interrupts and duration wind
 * 
 * Optipolar Hall Effect Sensor SS451A - Anemometer
 * speed  = (( (signals/2) * (2 * pi * radius) ) / time) * calibration_factor
 * speed in m/s =  (   ( (interrupts/2) * (2 * 3.14159 * 0.079) )  / (time_period in ms / 1000)  )  * 2.64
 *=======================================================================================================================
 */
float Wind_SampleSpeed() {
  unsigned long time_ms, delta_ms, count;
  float wind_speed;

  noInterrupts();
  count = anemometer_interrupt_count;
  anemometer_interrupt_count = 0;
  time_ms = millis();
  interrupts();

  // Unsigned subtraction naturally wraps on rollover, so if time_ms has rolled past zero and anemometer_interrupt_stime 
  // is still the old large value, the subtraction still produces the correct elapsed time.
  delta_ms = time_ms - anemometer_interrupt_stime;
  anemometer_interrupt_stime = time_ms;

  if (count && delta_ms > 0) {
    // wind_speed = (  ( (count/2) * (2 * 3.14159 * ws_radius) )  / (float)( (float)delta_ms / 1000)  ) * ws_calibration;
    
    wind_speed = ((count * 3.14159f * ws_radius) / ((float)delta_ms / 1000.0f)) * ws_calibration;
  } else {
    wind_speed = 0.0f;
  }

  return wind_speed;
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
    if (AS5600_exists) {
      Output ("WD Offline_L");
    }
    AS5600_exists = false;
  }
  else if (Wire.requestFrom(AS5600_ADR, 1)) {
    int AS5600_lo_raw = Wire.read();
  
    // Read Raw Angle High Byte
    Wire.beginTransmission(AS5600_ADR);
    Wire.write(AS5600_raw_ang_hi);
    if (Wire.endTransmission()) {
      if (AS5600_exists) {
        Output ("WD Offline_H");
      }
      AS5600_exists = false;
    }
    else if (Wire.requestFrom(AS5600_ADR, 1)) {
      word AS5600_hi_raw = Wire.read();
      
      if (!AS5600_exists) {
        Output ("WD Online");
      }
      AS5600_exists = true;           // We made it 

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
 * Wind_TakeReading() - Wind direction and speed, measure every second             
 * ======================================================================================================================
 */
void Wind_TakeReading() {
  wind.bucket[wind.bucket_idx].direction = (int) Wind_SampleDirection();
  wind.bucket[wind.bucket_idx].speed = Wind_SampleSpeed();
  wind.bucket_idx = (++wind.bucket_idx) % WIND_READINGS; // Advance bucket index for next reading
}

/* 
 *=======================================================================================================================
 * Wind_obs_do() - 
 *=======================================================================================================================
 */
void Wind_obs_do(int &sidx) {
  if (scv.wind) {
    float ws = 0.0;
    int wd = 0;

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

/* 
 *=======================================================================================================================
 * Wind_info() - add to info msg, Passing msg ptr, msg bubber sz, and pointer to comma
 *=======================================================================================================================
 */
void Wind_info(char *rest, int size, const char *&comma) {
  int used = strlen(rest);

  if (scv.wind) {
    sprintf (Buffer32Bytes, "WIND,WS(%s)", pinNames[ANEMOMETER_IRQ_PIN]);

    if (AS5600_exists) {
      sprintf (Buffer32Bytes+strlen(Buffer32Bytes), ",AS5600");
    }
    else {
      sprintf (Buffer32Bytes+strlen(Buffer32Bytes), ",!AS5600");
    }
  }
  else {
    sprintf (Buffer32Bytes, "!WIND");
  }

  // can we add with out over flowing?
  if ((used + strlen(Buffer32Bytes)) < size) {
    sprintf(rest + used, "%s%s", comma, Buffer32Bytes);
    comma = ",";
  }
  else {
    return;
  }
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
 * Wind_Distance_Air_Initialize()
 *=======================================================================================================================
 */
void Wind_Distance_Air_Initialize() {
  Output ("WDA:Init()");

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
  if (AS5600_exists || PM25AQI_exists || (nv_op1_state() == OP1_STATE_DIST_5M) || (nv_op1_state() == OP1_STATE_DIST_10M)) {
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
    sprintf (Buffer32Bytes, "WS:%.2f WD:%d", ws, Wind_DirectionVector());
    Output (Buffer32Bytes);
  }
  
  if (PM25AQI_exists) {
    sprintf (Buffer32Bytes, "pm1e10:%d", pm25aqi_obs.e10);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1e25:%d", pm25aqi_obs.e25);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1e100:%d", pm25aqi_obs.e100);
    Output (Buffer32Bytes);
  }

  if ((nv_op1_state() == OP1_STATE_DIST_5M) || (nv_op1_state() == OP1_STATE_DIST_10M)) {
    sprintf (Buffer32Bytes, "DS:%d", DS_Median());
    Output (Buffer32Bytes);
  }
}