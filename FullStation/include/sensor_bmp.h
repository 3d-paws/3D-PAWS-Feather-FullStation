/*
 * ======================================================================================================================
 *  sensor_bmp.h - Bosch I2C Sensors
 * ======================================================================================================================
 */
#include <Adafruit_BMP3XX.h>
#include <Adafruit_BMP5xx.h>

/*
 * ======================================================================================================================
 *  BMX280 humidity - I2C - Temperature, pressure sensor & altitude - Support 2 of any combination
 * 
 *  https://www.asknumbers.com/PressureConversion.aspx
 *  Pressure is returned in the SI units of Pascals. 100 Pascals = 1 hPa = 1 millibar. 
 *  Often times barometric pressure is reported in millibar or inches-mercury. 
 *  For future reference 1 pascal = 0.000295333727 inches of mercury, or 1 inch Hg = 3386.39 Pascal. 
 *
 *  Looks like you divide by 100 and you get millibars which matches NWS page
 * 
 *  Surface Observations and Station Elevation 
 *  https://forecast.weather.gov/product.php?issuedby=BOU&product=OSO&site=bou 
 * 
 * The Bosch BMPXXX sensors all have this issue of saving the last reading. 
 * The recommended solution is to always throw away the first reading after any long 
 * period of inactivity (sleep or power on).
 * SEE https://forums.adafruit.com/viewtopic.php?t=209906
 * ======================================================================================================================
 */
#define BMP_TATION_ELEVATION 1017.272     // default 1013.25
#define BMP390_ADDR_1     0x77        // BMP Default Address - Connecting SDO to GND will change BMP390 to 0x76
#define BMP390_ADDR_2     0x76
#define BMP581_ADDR_1     0x47        // BMP Default Address - Connecting SDO to GND will change BMP580 to 0x46
#define BMP581_ADDR_2     0x46
#define BMP390_CHIPID     0x60
#define BMP581_CHIPID     0x50        // Same chip ID as BMP580

#define BMP_TYPE_UNKNOWN      0
#define BMP_TYPE_BMP390       1
#define BMP_TYPE_BMP581       2

// Extern declarations for sensor objects
extern Adafruit_BMP3XX bmp31;
extern Adafruit_BMP3XX bmp32;
extern Adafruit_BMP5xx bmp51;
extern Adafruit_BMP5xx bmp52;

// Extern declarations for variables
extern byte BMP_1_chip_id;
extern byte BMP_2_chip_id;
extern bool BMP_1_exists;
extern bool BMP_2_exists;
extern byte BMP_1_type;
extern byte BMP_2_type;;
extern const char *bmptype[];
extern float bmp_1_pressure;

// Function prototype

void bmp1_read(float &p, float &t);
void bmp2_read(float &p, float &t);
void bmp_obs_do(int &sidx);
void bmp_info(char *rest, int size, const char *&comma);
void bmp_initialize();