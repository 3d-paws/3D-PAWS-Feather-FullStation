/*
 * ======================================================================================================================
 *  sensor_pm25aqi.h - PM25AQI I2C Sensor
 * ======================================================================================================================
 */
#include <Adafruit_PM25AQI.h>

 /*
 * ======================================================================================================================
 *  HTU21D-F - I2C - Humidity & Temp Sensor Address 0x40
 * ======================================================================================================================
 */
/*
 * ======================================================================================================================
 *  PM25AQI - I2C - Air Quality Sensor
 * 
 *  PM25_AQI_Data variables returned are unsigned integers
 *    pm10_standard     < Standard Particle PM1.0 concentration unit µg 𝑚3
 *    pm25_standard     < Standard Particle PM2.5 concentration unit µg 𝑚3
 *    pm100_standard    < Standard Particle PM10.0 concentration unit µg 𝑚3
 *    pm10_env          < Atmospheric Environmental PM1.0 concentration unit µg 𝑚3
 *    pm25_env          < Atmospheric Environmental PM2.5 concentration unit µg 𝑚3
 *    pm100_env         < Atmospheric Environmental PM10.0 concentration unit µg 𝑚3
 *    particles_03um    < Particles with diameter beyond 0.3 µ 𝑚 in 0.1L of air
 *    particles_05um    < Particles with diameter beyond 0.5 µ 𝑚 in 0.1L of air
 *    particles_10um    < Particles with diameter beyond 1.0 µ 𝑚 in 0.1L of air
 *    particles_25um    < Particles with diameter beyond 2.5 µ 𝑚 in 0.1L of air
 *    particles_50um    < Particles with diameter beyond 5.0 µ 𝑚 in 0.1L of air
 *    particles_100um   < Particles with diameter beyond 10.0 µ 𝑚 in 0.1L of air
 * 
 *  pms = Particulate Matter Standard
 *  pme = Particulate Matter Environmental
 * 
 *  Variable Tags for what we monitor and report on
 *    pm1e10
 *    pm1e25
 *    pm1e100
 * 
 * How the sensor works internally
 * Mainly output is the quality and number of each particles with different size per unit volume, the unit volume of 
 * particle number is 0.1L and the unit of mass concentration is μg/m³
 * Internal sampling is divided into two sub modes: stable mode and fast mode. If the concentration change is small 
 * the sensor would run at stable mode with the real interval of 2.3s. And if the change is big the sensor would be 
 * changed to fast mode automatically with the interval of 200~800ms, the higher of the concentration, the shorter of
 * the interval. 
 * ======================================================================================================================
 */
#define PM25AQI_ADDRESS   0x12
#define PM25AQI_1M_BUCKETS  60

typedef struct {
  int32_t e10, e25, e100;
} PM25AQI_OBS;

extern PM25AQI_OBS pm25aqi_1m_obs[PM25AQI_1M_BUCKETS];
extern int pm25aqi_1m_bucket;

typedef struct {
  int32_t e10, e25, e100;
  int count=0;
  int fail_count=0;
} PM25AQI_OBS_STR;

extern PM25AQI_OBS_STR pm25aqi_obs;
extern Adafruit_PM25AQI pmaq;
extern bool PM25AQI_exists;

extern int AQSWarmUpTime;
extern int AQS_Correction;

// Function prototype
void pm25aqi_1m_clear();
void pm25aqi_clear();
void pm25aqi_Produce_1m_Average();
void pm25aqi_TakeReading();
void pm25aqi_TakeReading_AQS();
void pm25aqi_obs_do(int &sidx);
void pm25aqi_info(char *rest, int size, const char *&comma);
void pm25aqi_initialize();

