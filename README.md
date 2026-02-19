# 3D-PAWS-Feather-FullStation

## Features
- Supports LoRaWAN connectivity via [Adafruit Feather M0 LoRaWAN Board](https://www.adafruit.com/product/3178)
- Supports WiFi connectivity via [Adafruit Feather M0 WiFi Board](ttps://www.adafruit.com/product/3010)
- Duplicates (as close as possible) the features from the [3D-PAWS Particle Full Station](https://github.com/3d-paws/3D-PAWS-Particle-FullStation) project.

## Release FSLW-260219
- Code cleanup with proper .cpp and .h files
- Added lora wan options to config file
- Added SDU boot
- Clear rain totals from EEPROM at boot if CRT.TXT exists
- Added INFO support
- Library Update and Changes
  - Changed SD to SdFat 2.3.0
  - Add Adafruit_ZeroDMA, LeafArduinoI2c, i2cArduino, i2cMultiArd
  - MCCI LoRaWAN LMIC library 4.1.1 -> 5.0.1
  - BMP280 2.6.2 -> 2.6.8
  - BMP3XX 2.1.2 -> 2.1.6
  - BusIO 1.11.6 -> 1.17.4
  - GFX 1.11.1 -> 1.12.4
  - HTU21DF 1.1.0 -> 1.1.2
  - MCP9808 2.0.0 -> 2.0.2
  - PM25_AQI 1.0.6 -> 2.0.0
  - SHT31 2.2.0 -> 2.2.2
  - SI1145 1.2.0 -> 1.2.2
  - SSD1306 2.5.3-> 2.5.16
  - Unified_Sensor 1.1.5 -> 1.1.15
  - VEML7700 2.1.2 -> 2.1.6
  - RTClib 2.0.3 -> 2.1.4
- Fixed bug in Wind_SampleDirection(), if AS5600 went offline we would stop doing wind.
- Moved A4/A5 to OP1/OP2
- Added OP2 option to read voltaic voltage
- Documentation Added

## READMEs
### [Arduino Compile Notes](docs/CompileNotes.md)
### [Configuration](docs/Configuration.md)
### [LoRa Antenna Options](docs/LoRaAntennaOptions.md)
### [Pin Mappings](docs/PinMappings.md)
### [SD Card Information](docs/SD.md)
### [INFO (Station Information) WiFi](docs/INFO_WiFi.md)
### [INFO (Station Information) LoRaWAN](docs/INFO_LoRa.md)
### [WiFi Board Preparation](docs/WiFiBoardPrep.md)
### [Connecting a Device to The Things Network (TTN)](docs/ConnectToTTN.md)
### [Adding a Webhook to The Things Network (TTN)](docs/AddWebhookToTTN.md)
### [Sensor Information](docs/Sensors.md)
### [System Health Bits (hth)](docs/SystemHealthBits.md)
### [WatchDog Board](docs/WatchDog.md)
### [Daily Reboot](docs/DailyReboot.md)
### [Serial Monitor](docs/SerialMonitor.md)
### [Adding a Gateway to The Things Network (TTN)](docs/AddGatewayToTTN.md)


