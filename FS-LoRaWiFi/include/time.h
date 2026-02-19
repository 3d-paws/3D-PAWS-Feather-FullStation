/*
 * ======================================================================================================================
 *  time.h - Time Management Definations
 * ======================================================================================================================
 */
#include <ctime>  // Provides the tm structure
#include <RTClib.h>

#define PCF8523_ADDRESS         0x68       // I2C address for PCF8523
#define TM_VALID_YEAR_START     2026
#define TM_VALID_YEAR_END       2033

 // Extern variables
extern RTC_PCF8523 rtc;
extern DateTime now;
extern char timestamp[128];
extern bool RTC_valid;
extern bool RTC_exists;
extern unsigned long LastTimeUpdate;
extern unsigned long NoClockRecheckTime;

 // Function prototypes
 uint32_t rtc_unixtime();
 void rtc_timestamp();
 void rtc_initialize();
 bool rtc_readserial();
 

