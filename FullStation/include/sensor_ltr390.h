/*
 * ======================================================================================================================
 *  sensor_ltr390.h - Adafruit LTR390 UV Light Sensor - I2C
 * ======================================================================================================================
 */
#include <Adafruit_LTR390.h>

/*
 * ======================================================================================================================
 *  Adafruit LTR390 UV Light Sensor
 *  UVA sensing with a peak spectral response between 300 and 350nm
 *  https://www.adafruit.com/product/4831
 * 
 *  See OptoElectronics LiteOn https://optoelectronics.liteon.com/upload/download/DS86-2015-0004/LTR-390UV-01_Final_%20DS_V1.7.PDF
 *  Section 7 ALS/UVI Formula
 * 
 *  UVI (UV Index) = UVS counts / 2300F
 *  Above assumes LTR390_GAIN_18 and LTR390_RESOLUTION_20BIT
 * 
 *  SEE https://github.com/adafruit/Adafruit_CircuitPython_LTR390/issues/2
*  UVI (UV Index) = UVS counts / 95F
 *  Above assumes LTR390_GAIN_3 and LTR390_RESOLUTION_16BIT
 * 
 *  Will report as ltruvc and ltruvi
 * ======================================================================================================================
 */
#define LTR390_ADDRESS   0x53
extern bool LTR390_exists;
extern Adafruit_LTR390 ltr390; 

// Function prototype
void ltr390_obs_do(int &sidx);
void ltr390_info(char *rest, int size, const char *&comma);
void ltr390_initialize();