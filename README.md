# 3D-PAWS-Feather-FullStation
- Support LoRaWAN connectivity via Adafruit Feather M0 LoRa board
- Suuport WiFi connectivity via Adafruit Feather M0 WiFi board

### LoRa Antenna Options
 - https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/antenna-options
 
### Setting Country Frequencies
```C 
MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h

// project-specific definitions
//#define CFG_eu868 1
#define CFG_us915 1
//#define CFG_au915 1
//#define CFG_as923 1
// #define LMIC_COUNTRY_CODE LMIC_COUNTRY_CODE_JP    // for as923-JP; also define CFG_as923
//#define CFG_kr920 1
//#define CFG_in866 1
#define CFG_sx1276_radio 1
//#define LMIC_USE_INTERRUPTS

Compile for EU Frequencies 
cd MCCI_LoRaWAN_LMIC_library/project_config
cp lmic_project_config.h-eu lmic_project_config.h

Compile for US Frequencies 
cd MCCI_LoRaWAN_LMIC_library/project_config
cp lmic_project_config.h-us lmic_project_config.h
 ```