/*
 * ======================================================================================================================
 *  WRD.h - Wind Rain Distance Functions
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Wind Related Setup
 * 
 *  NOTE: With interrupts tied to the anemometer rotation we are essentually sampling all the time.  
 *        We record the interrupt count, ms duration and wind direction every second.
 *        One revolution of the anemometer results in 2 interrupts. There are 2 magnets on the anemometer.
 * 
 *        Station observations are logged every minute
 *        Wind and Direction are sampled every second producing 60 samples 
 *        The one second wind speed sample are produced from the interrupt count and ms duration.
 *        Wind Observations a 
 *        Reported Observations
 *          Wind Speed = Average of the 60 samples.
 *          Wind Direction = Average of the 60 vectors from Direction and Speed.
 *          Wind Gust = Highest 3 consecutive samples from the 60 samples. The 3 samples are then averaged.
 *          Wind Gust Direction = Average of the 3 Vectors from the Wind Gust samples.
 * 
 * Distance Sensors
 * The 5-meter sensors (MB7360, MB7369, MB7380, and MB7389) use a scale factor of (Vcc/5120) per 1-mm.
 * Particle 12bit resolution (0-4095),  Sensor has a resolution of 0 - 5119mm,  Each unit of the 0-4095 resolution is 1.25mm
 * Feather has 10bit resolution (0-1023), Sensor has a resolution of 0 - 5119mm, Each unit of the 0-1023 resolution is 5mm
 * 
 * The 10-meter sensors (MB7363, MB7366, MB7383, and MB7386) use a scale factor of (Vcc/10240) per 1-mm.
 * Particle 12bit resolution (0-4095), Sensor has a resolution of 0 - 10239mm, Each unit of the 0-4095 resolution is 2.5mm
 * Feather has 10bit resolution (0-1023), Sensor has a resolution of 0 - 10239mm, Each unit of the 0-1023 resolution is 10mm
 * ======================================================================================================================
 */
#define WIND_READINGS       60       // One minute of 1s Samples

typedef struct {
  int direction;
  float speed;
} WIND_BUCKETS_STR;

typedef struct {
  WIND_BUCKETS_STR bucket[WIND_READINGS];
  int bucket_idx;
  float gust;
  int gust_direction;
} WIND_STR;
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
 *  Pin A4 State Setup
 * ======================================================================================================================
 */
#define A4_STATE_NULL       0
#define A4_STATE_DISTANCE   1
#define A4_STATE_RAIN       2
#define A4_STATE_RAW        3
char SD_A4_DIST_FILE[] = "A4DIST.TXT";         // File used to set pin A4 as a Distance Gauge
char SD_A4_RAIN_FILE[] = "A4RAIN.TXT";         // File used to set pin A4 as a 2nd Rain Gauge
char SD_A4_RAW_FILE[]  = "A4RAW.TXT";          // File used to set pin A4 as generic analog device connected
int A4_State = A4_STATE_NULL;                  // Default is not used

/*
 * ======================================================================================================================
 *  Pin A5 State Setup
 * ======================================================================================================================
 */
#define A5_STATE_NULL       0
#define A5_STATE_RAW        1
char SD_A5_RAW_FILE[]  = "A5RAW.TXT";          // File used to set pin A5 as generic analog device connected
int A5_State = A5_STATE_NULL;                  // Default is not used

/*
 * =======================================================================================================================
 *  Distance Gauge - Can be Distance or Stream
 * =======================================================================================================================
 */
#define DISTANCE_GAUGE_PIN  A4
#define DG_BUCKETS          60
char SD_5M_DIST_FILE[] = "5MDIST.TXT";        // Multiply by 1.25 for 5m Distance Gauge
unsigned int dg_bucket = 0;
float dg_adjustment = 2.5;                    // Default sensor is 10m
unsigned int dg_buckets[DG_BUCKETS];

// Interrupts
// NOTE All A and D pins (including TX, RX, and SPI) on Gen 3 devices can be used for interrupts, 
// however you can only attach interrupts to 8 pins at the same time. 

/*
 * ======================================================================================================================
 *  Optipolar Hall Effect Sensor SS451A - Anemometer
 * ======================================================================================================================
 */
volatile unsigned int anemometer_interrupt_count;
uint64_t anemometer_interrupt_stime;

/*
 * ======================================================================================================================
 *  anemometer_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
#define ANEMOMETER_IRQ_PIN  A2
void anemometer_interrupt_handler()
{
  anemometer_interrupt_count++;
}

/*
 * ======================================================================================================================
 *  Optipolar Hall Effect Sensor SS451A - Rain Gauge
 * ======================================================================================================================
 */
volatile unsigned int raingauge1_interrupt_count;
uint64_t raingauge1_interrupt_stime; // Send Time
uint64_t raingauge1_interrupt_ltime; // Last Time
uint64_t raingauge1_interrupt_toi;   // Time of Interrupt

/*
 * ======================================================================================================================
 *  raingauge1_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
#define RAINGAUGE1_IRQ_PIN A3
void raingauge1_interrupt_handler()
{
  if ((System.millis() - raingauge1_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    raingauge1_interrupt_ltime = System.millis();
    raingauge1_interrupt_count++;
    digitalWrite(LED_PIN, HIGH);
    TurnLedOff = true;
  }   
}

/*
 * ======================================================================================================================
 *  Optipolar Hall Effect Sensor SS451A - Rain Gauge 2 - Optional pin A4
 * ======================================================================================================================
 */
volatile unsigned int raingauge2_interrupt_count;
uint64_t raingauge2_interrupt_stime; // Send Time
uint64_t raingauge2_interrupt_ltime; // Last Time
uint64_t raingauge2_interrupt_toi;   // Time of Interrupt

/*
 * ======================================================================================================================
 *  raingauge2_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
#define RAINGAUGE2_IRQ_PIN   A4
void raingauge2_interrupt_handler()
{
  if ((System.millis() - raingauge2_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    raingauge2_interrupt_ltime = System.millis();
    raingauge2_interrupt_count++;
    digitalWrite(LED_PIN, HIGH);
    TurnLedOff = true;
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
    SystemStatusBits |= SSB_AS5600;  // Turn On Bit
  }
  else {
    msgp = (char *) "WD:OK";
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * A4_Initialize()
 *=======================================================================================================================
 */
void A4_Initialize() {
  Output ("A4:INIT");
  if (SD_exists) {
    if (SD.exists(SD_A4_DIST_FILE)) {
      Output ("A4=DIST");
      A4_State = A4_STATE_DISTANCE;
      if (SD.exists(SD_5M_DIST_FILE)) {
        dg_adjustment = 1.25;
        Output ("DIST=5M");
      }
      else {
        dg_adjustment = 2.5;
        Output ("DIST=10M");
      }
    }
    else if (SD.exists(SD_A4_RAIN_FILE)) {
      Output ("A4=RAIN");
      A4_State = A4_STATE_RAIN;
      // Optipolar Hall Effect Sensor SS451A - Rain Gauge 2
      raingauge2_interrupt_count = 0;
      raingauge2_interrupt_stime = System.millis();
      raingauge2_interrupt_ltime = 0;  // used to debounce the tip
      attachInterrupt(RAINGAUGE2_IRQ_PIN, raingauge2_interrupt_handler, FALLING);
    }
    else if (SD.exists(SD_A4_RAW_FILE)) {
      Output ("A4=RAW");
      A4_State = A4_STATE_RAW;
    }
    else {
      Output ("A4=NULL");
    }
  }
  else {
    Output ("A4=NULL,SD NF");
  }
}

/* 
 *=======================================================================================================================
 * A5_Initialize()
 *=======================================================================================================================
 */
void A5_Initialize() {
  Output ("A5:INIT");
  if (SD_exists) {
    if (SD.exists(SD_A5_RAW_FILE)) {
      Output ("A5=RAW");
      A5_State = A5_STATE_RAW;
    }
    else {
      Output ("A5=NULL");
    }
  }
  else {
    Output ("A5=NULL,SD NF");
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
 * ======================================================================================================================
 * DistanceGauge_TakeReading() - measure every second             
 * ======================================================================================================================
 */
void DistanceGauge_TakeReading() {
  dg_buckets[dg_bucket] = (int) analogRead(DISTANCE_GAUGE_PIN) * dg_adjustment;
  dg_bucket = (++dg_bucket) % DG_BUCKETS; // Advance bucket index for next reading
}

/* 
 *=======================================================================================================================
 * DistanceGauge_Median()
 *=======================================================================================================================
 */
float DistanceGauge_Median() {
  int i;
  
  mysort(dg_buckets, DG_BUCKETS);
  i = (DG_BUCKETS+1) / 2 - 1; // -1 as array indexing in C starts from 0
  
  return (dg_buckets[i]); 
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
  uint64_t delta_ms;
  float wind_speed;
  
  delta_ms = System.millis()-anemometer_interrupt_stime;

  if (anemometer_interrupt_count) {
    // wind_speed = (  ( (anemometer_interrupt_count/2) * (2 * 3.14156 * ws_radius) )  / 
    //  (float)( (float)delta_ms / 1000)  )  * ws_calibration;

    wind_speed = ( ( anemometer_interrupt_count * 3.14156 * ws_radius)  / 
        (float)( (float)delta_ms / 1000) )  * ws_calibration;
  }
  else {
    wind_speed = 0.0;
  }

  anemometer_interrupt_count = 0;
  anemometer_interrupt_stime = System.millis();
  
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
      SystemStatusBits &= ~SSB_AS5600; // Turn Off Bit
      
      AS5600_hi_raw = AS5600_hi_raw << 8; //shift raw angle hi 8 left
      AS5600_hi_raw = AS5600_hi_raw | AS5600_lo_raw; //AND high and low raw angle value
      degree = (int) AS5600_hi_raw * 0.0879;
      if ((degree >=0) && (degree <= 360)) {
        return (degree);
      }
      else {
        return (-1);
      }
    }
  }
  SystemStatusBits |= SSB_AS5600;  // Turn On Bit
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

  // If all the winds speeds are 0 or we have a -1 direction then set -1 for direction.
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
 * Wind_Distance_Air_Initialize()
 *=======================================================================================================================
 */
void Wind_Distance_Air_Initialize() {
  Output ("WindDist Init()");

  // Clear windspeed counter  
  anemometer_interrupt_count = 0;
  anemometer_interrupt_stime = System.millis();
  
  // Init default values.
  wind.gust = 0.0;
  wind.gust_direction = -1;
  wind.bucket_idx = 0;

  // Take N 1s samples of wind speed and direction and fill arrays with values.
  for (int i=0; i< WIND_READINGS; i++) {
    lora_msg_poll(); // 750ms Second Delay
    HeartBeat();     // Provides a 250ms delay
    Wind_TakeReading();
    if (A4_State == A4_STATE_DISTANCE) {
      DistanceGauge_TakeReading();
    }
    if (PM25AQI_exists) {
      pm25aqi_TakeReading();
    }
    if (SerialConsoleEnabled) Serial.print(".");  // Provide Serial Console some feedback as we loop and wait til next observation
    OLED_spin();
  }

  // Now we have N readings we can compute other wind related global varibles
  Wind_TakeReading();
  if (A4_State == A4_STATE_DISTANCE) {
    DistanceGauge_TakeReading();
  }

  if (SerialConsoleEnabled) Serial.println();  // Send a newline out to cleanup after all the periods we have been logging

  ws_refresh = false; // Set to false since we have just initialized wind speed data.
}

/*
 * ======================================================================================================================
 * I2C_Check_Sensors() - See if each I2C sensor responds on the bus and take action accordingly             
 * ======================================================================================================================
 */
void I2C_Check_Sensors() {

  // BMX_1 Barometric Pressure 
  if (I2C_Device_Exist (BMX_ADDRESS_1)) {
    // Sensor online but our state had it offline
    if (BMX_1_exists == false) {
      if (BMX_1_chip_id == BMP280_CHIP_ID) {
        if (bmp1.begin(BMX_ADDRESS_1)) { 
          BMX_1_exists = true;
          Output ("BMP1 ONLINE");
          SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
        } 
      }
      else if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
        if (BMX_1_type == BMX_TYPE_BME280) {
          if (bme1.begin(BMX_ADDRESS_1)) { 
            BMX_1_exists = true;
            Output ("BME1 ONLINE");
            SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
          } 
        }
        if (BMX_1_type == BMX_TYPE_BMP390) {
          if (bm31.begin_I2C(BMX_ADDRESS_1)) {
            BMX_1_exists = true;
            Output ("BMP390_1 ONLINE");
            SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
          }
        }        
      }
      else {
        if (bm31.begin_I2C(BMX_ADDRESS_1)) { 
          BMX_1_exists = true;
          Output ("BM31 ONLINE");
          SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
        }                  
      }      
    }
  }
  else {
    // Sensor offline but our state has it online
    if (BMX_1_exists == true) {
      BMX_1_exists = false;
      Output ("BMX1 OFFLINE");
      SystemStatusBits |= SSB_BMX_1;  // Turn On Bit 
    }    
  }

  // BMX_2 Barometric Pressure 
  if (I2C_Device_Exist (BMX_ADDRESS_2)) {
    // Sensor online but our state had it offline
    if (BMX_2_exists == false) {
      if (BMX_2_chip_id == BMP280_CHIP_ID) {
        if (bmp2.begin(BMX_ADDRESS_2)) { 
          BMX_2_exists = true;
          Output ("BMP2 ONLINE");
          SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
        } 
      }
      else if (BMX_2_chip_id == BME280_BMP390_CHIP_ID) {
        if (BMX_2_type == BMX_TYPE_BME280) {
          if (bme1.begin(BMX_ADDRESS_2)) { 
            BMX_2_exists = true;
            Output ("BME2 ONLINE");
            SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
          } 
        }
        if (BMX_2_type == BMX_TYPE_BMP390) {
          if (bm31.begin_I2C(BMX_ADDRESS_2)) {
            BMX_1_exists = true;
            Output ("BMP390_1 ONLINE");
            SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
          }
        }        
      }
      else {
         if (bm32.begin_I2C(BMX_ADDRESS_2)) { 
          BMX_2_exists = true;
          Output ("BM32 ONLINE");
          SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
        }                         
      }     
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (BMX_2_exists == true) {
      BMX_2_exists = false;
      Output ("BMX2 OFFLINE");
      SystemStatusBits |= SSB_BMX_2;  // Turn On Bit 
    }    
  }

  // HTU21DF Humidity & Temp Sensor
  if (I2C_Device_Exist (HTU21DF_I2CADDR)) {
    // Sensor online but our state had it offline
    if (HTU21DF_exists == false) {
      // See if we can bring sensor online
      if (htu.begin()) {
        HTU21DF_exists = true;
        Output ("HTU ONLINE");
        SystemStatusBits &= ~SSB_HTU21DF; // Turn Off Bit
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (HTU21DF_exists == true) {
      HTU21DF_exists = false;
      Output ("HTU OFFLINE");
      SystemStatusBits |= SSB_HTU21DF;  // Turn On Bit
    }   
  }

#ifdef NOWAY    // MCP9808 Sensors fails to update temperature if this code is enabled
  // MCP9808 Precision I2C Temperature Sensor
  if (I2C_Device_Exist (MCP_ADDRESS_1)) {
    // Sensor online but our state had it offline
    if (MCP_1_exists == false) {
      // See if we can bring sensor online
      if (mcp1.begin(MCP_ADDRESS_1)) {
        MCP_1_exists = true;
        Output ("MCP ONLINE");
        SystemStatusBits &= ~SSB_MCP_1; // Turn Off Bit
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (MCP_1_exists == true) {
      MCP_1_exists = false;
      Output ("MCP OFFLINE");
      SystemStatusBits |= SSB_MCP_1;  // Turn On Bit
    }   
  }
#endif

  // SI1145 UV index & IR & Visible Sensor
  if (I2C_Device_Exist (SI1145_ADDR)) {
    // Sensor online but our state had it offline
    if (SI1145_exists == false) {
      // See if we can bring sensore online
      if (uv.begin()) {
        SI1145_exists = true;
        Output ("SI ONLINE");
        SystemStatusBits &= ~SSB_SI1145; // Turn Off Bit
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (SI1145_exists == true) {
      SI1145_exists = false;
      Output ("SI OFFLINE");
      SystemStatusBits |= SSB_SI1145;  // Turn On Bit
    }   
  }

  // AS5600 Wind Direction
  if (I2C_Device_Exist (AS5600_ADR)) {
    // Sensor online but our state had it offline
    if (AS5600_exists == false) {
      AS5600_exists = true;
      Output ("WD ONLINE");
      SystemStatusBits &= ~SSB_AS5600; // Turn Off Bit
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (AS5600_exists == true) {
      AS5600_exists = false;
      Output ("WD OFFLINE");
      SystemStatusBits |= SSB_AS5600;  // Turn On Bit
    }   
  }

  // VEML7700 Lux 
  if (I2C_Device_Exist (VEML7700_ADDRESS)) {
    // Sensor online but our state had it offline
    if (VEML7700_exists == false) {
      // See if we can bring sensor online
      if (veml.begin()) {
        VEML7700_exists = true;
        Output ("VLX ONLINE");
        SystemStatusBits &= ~SSB_VLX; // Turn Off Bit
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (VEML7700_exists == true) {
      VEML7700_exists = false;
      Output ("VLX OFFLINE");
      SystemStatusBits |= SSB_VLX;  // Turn On Bit
    }   
  }

  // PM25AQI
  if (I2C_Device_Exist (PM25AQI_ADDRESS)) {
    // Sensor online but our state had it offline
    if (PM25AQI_exists == false) {
      // See if we can bring sensor online
      if (pmaq.begin_I2C()) {
        PM25AQI_exists = true;
        Output ("PM ONLINE");
        SystemStatusBits &= ~SSB_PM25AQI; // Turn Off Bit
        pm25aqi_clear();
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (PM25AQI_exists == true) {
      PM25AQI_exists = false;
      Output ("PM OFFLINE");
      SystemStatusBits |= SSB_PM25AQI;  // Turn On Bit
    }   
  }
}
