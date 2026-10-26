/*
 * ======================================================================================================================
 *  sensor_tsm.h - Tinovi Soil Moisture I2C Sensor
 * ======================================================================================================================
 */
#include <i2cArduino.h>

/*
 * ======================================================================================================================
 *  Tinovi Soil Moisture initialize - Chip ID = 0x63
 * ======================================================================================================================
 */
#define TSM_ADDRESS     0x63

extern SVCS3 tsm;
extern bool TSM_exists;

// Function prototype
void tsm_obs_do(int &sidx);
void tsm_info(char *rest, int size, const char *&comma);
void tsm_initialize();