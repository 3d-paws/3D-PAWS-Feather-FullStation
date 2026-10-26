/*
 * ======================================================================================================================
 *  feather.h - Feather Related Board Functions and Definations
 * ======================================================================================================================
 */
//#include <Arduino.h>

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

// lg = Adafruit Feather M0 Adalogger
// wf = Adafruit Feather M0 WiFi
// lw = Adafruit Feather M0 LoRaWAN
// nc = Blues NoteCard
typedef enum {
  bc_unkn, bc_lg, bc_lg_nc, bc_wf, bc_wf_nc, bc_lw, bc_lw_nc 
} BOARD_CONFIG;

// Extern variables
extern char DeviceID[17];
extern const char *board_config_tag[];
extern BOARD_CONFIG board_config;
extern const char* pinNames[];

// Function prototypes
float vbat_get();
void GetDeviceID();
void boardcfg_initialize();