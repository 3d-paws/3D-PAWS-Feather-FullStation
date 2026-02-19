/*
 * ======================================================================================================================
 *  feather.h - Feather Related Board Functions and Definations
 * ======================================================================================================================
 */
#include <Arduino.h>

/*
 * =======================================================================================================================
 *  Measuring Battery - SEE https://learn.adafruit.com/adafruit-feather-m0-adalogger/power-management
 *  
 *  This voltage will 'float' at 4.2V when no battery is plugged in, due to the lipoly charger output,  so its not a good 
 *  way to detect if a battery is plugged in or not (there is no simple way to detect if a battery is plugged in)
 * =======================================================================================================================
 */
#define VBATPIN      A7
#define LED_PIN      LED_BUILTIN

// Extern variables
extern char DeviceID[25];
extern const char* pinNames[];

// Function prototypes
float vbat_get();
void GetDeviceID();