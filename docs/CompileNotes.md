# Compile Notes
[←Top](../README.md)<BR>

The code is provided at [Github - https://github.com/3d-paws/3D-PAWS-Feather-FullStation](https://github.com/3d-paws/3D-PAWS-Feather-FullStation)

Libraries are provided and need to be copied to your Arduino's library directory.

### Setting Country Frequencies
You will need to modify library MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h and configure for LoRa frequency plan for the country you are deploying in. This needs to be done prior to compiling. US and EU options have be set in country specific config files. You just need to copy these files to the actual config file.
```
Compile for EU Frequencies 
cd MCCI_LoRaWAN_LMIC_library/project_config
cp lmic_project_config.h-eu lmic_project_config.h

Compile for US Frequencies 
cd MCCI_LoRaWAN_LMIC_library/project_config
cp lmic_project_config.h-us lmic_project_config.h
```
Below is an example of lmic_project_config.h for US Frequencies.
```C 
// project-specific definitions
//#define CFG_eu868 1
#define CFG_us915 1
//#define CFG_au915 1
//#define CFG_as923 1
//#define LMIC_COUNTRY_CODE LMIC_COUNTRY_CODE_JP      /* for as923-JP; also define CFG_as923 */
//#define CFG_kr920 1
//#define CFG_in866 1
#define CFG_sx1276_radio 1
//#define CFG_sx1261_radio 1
//#define CFG_sx1262_radio 1
//#define ARDUINO_heltec_wifi_lora_32_V3
//#define LMIC_USE_INTERRUPTS
 ```

Notes:
- The software will determine if you are using a Adafruit Feather WiFi or LoRa board.
- To put the Feather in firmware down load mode, press the button on the board twice quickly.
- Once board is running 3D-PAWS code, there after you can place the firmware file in a file called UPDATE.BIN on the SD card. Then reboot. After update, the UPDATE.BIN will be removed from the SD card.
