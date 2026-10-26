/*
 * ======================================================================================================================
 *  sensor_tlw.h - Tinovi Leaf Wetness I2C Sensor
 * ======================================================================================================================
 */
#include <LeafSens.h>


/*
 * ======================================================================================================================
 *  Tinovi Leaf Wetness - Chip ID = 0x61,  Library init checks this.
 * ======================================================================================================================
 */
#define TLW_ADDRESS     0x61

extern LeafSens tlw;
extern bool TLW_exists;

// Function prototype
void tlw_obs_do(int &sidx);
void tlw_info(char *rest, int size, const char *&comma);
void tlw_initialize();