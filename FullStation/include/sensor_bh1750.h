/*
 * ======================================================================================================================
 *  sensor_bh1750.h - DFRobot SEN0562 - I2C - Ambient Light Sensor 1-65535lx - BH1750
 * ======================================================================================================================
 */
#include <BH1750.h> // DFRobot SEN0562 Ambient Light Sensor

/*
 * ======================================================================================================================
 *  DFRobot claims a measurement range of 1 to 65,535 lx with 1.2 lx accuracy and IP68 waterproofing.
 *  https://www.dfrobot.com/product-2664.html
 * ======================================================================================================================
 */
#define BH1750_ADDRESS   0x23
extern bool BH1750_exists;
extern BH1750 bh1750_lux;

// Function prototype
void bh1750_obs_do(int &sidx);
void bh1750_info(char *rest, int size, const char *&comma);
void bh1750_initialize();