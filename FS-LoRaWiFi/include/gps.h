/*
 * ======================================================================================================================
 *  GPS.h - GPS Definations
 * ======================================================================================================================
 */
#include <SparkFun_I2C_GPS_Arduino_Library.h>
#include <TinyGPS++.h>

#define GPS_ADDRESS 0x10

// Extern variables
extern bool gps_exists;
extern bool gps_valid;
extern char gps_timestamp[32];
extern double gps_lat;
extern double gps_lon;
extern double gps_altm;
extern double gps_altf;
extern int    gps_sat;

// Function prototypes
void gps_displayInfo();
void gps_aquire();
void gps_initialize();


