/*
 * ======================================================================================================================
 *  sensor_sht.h - Sensirion I2C Sensors
 * ======================================================================================================================
 */
#include <Adafruit_SHT31.h>
#include <Adafruit_SHT4x.h>

#define SHT_ADDR_1     0x44        // SHT Default Address
#define SHT_ADDR_2     0x45

#define SHT_TYPE_UNKNOWN    0
#define SHT_TYPE_SHT31      1
#define SHT_TYPE_SHT45      2

// Extern declarations for sensor objects
extern Adafruit_SHT31 sht31;
extern Adafruit_SHT31 sht32;
extern Adafruit_SHT4x sht4;

// Extern declarations for variables
extern bool SHT_1_exists;
extern bool SHT_2_exists;
extern byte SHT_1_type;
extern byte SHT_2_type;
extern uint32_t SHT_1_sn;
extern uint32_t SHT_2_sn;
extern const char *shttype[];
extern float sht1_temp;
extern float sht1_humid;

// Function prototype
void sht1_read(float &h, float &t);
void sht2_read(float &h, float &t);
void sht_obs_do(int &sidx);
void sht_info(char *rest, int size, const char *&comma);
void sht_initialize();