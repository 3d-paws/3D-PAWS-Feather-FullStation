PRODUCT_VERSION(39);
#define COPYRIGHT "Copyright [2024] [University Corporation for Atmospheric Research]"
#define VERSION_INFO "FSAC-250121v39"

/*
 *======================================================================================================================
 * FullStation (FS) - Always Connected (AC)
 *   Board Type : Particle BoronLTE https://docs.particle.io/datasheets/boron/boron-datasheet/
 *   Description: Monitor 3D-PAWS Full Station and transmit data to Particle Cloud
 *   Author: Robert Bubon
 *   Date:   2021-01-14 RJB Initial Development
 *           2021-02-10 RJB Work on SD code area
 *           2021-03-08 RJB LoRa added, 15min samples, Cell Power Off, Wind Observations Corrected
 *           2021-03-13 RJB Reworked N2S sending
 *           2021-04-20 RJB Clear Rain Gauge Counter
 *           2021-06-04 RJB Added Particle Theads
 *           2021-06-10 RJB Added Countdown timer on jumper, Modified Network Disconnect and Power Down disconnect
 *           2021-06-14 RJB When jumper set unit will continue the network connection to Particle, breathe cyan
 *           2021-06-21 RJB Correct bug in network connect and handle BME returning NAN.
 *           2021-07-12 RJB Always Connected and Debug Added
 *           2021-07-21 RJB Updated Libraries after HTU has infinate loop in code
 *           2021-10-06 RJB Version 2
 *                          Merged 15MC changes in, removed unneeded debug, 
 *                          formalized the Always Connected branch removed the 15m disconnect code 
 *           2021-10-22 RJB Version 3 
 *                          Remote Reboot Must pass "NOW" from Particle IDE
 *                          Battery Charger Fault Register, Variable name "CFR"
 *                          Adafruit BMP388 Support Added
 *           2022-01-03 RJB Version 4
 *                          Added Support for 2 MCP sensors
 *           2022-01-29 RJB Version 5
 *                          Spelling Corrections
 *                          Added support for Grove Shield
 *                          PIN CHANGES FOR GROVE SHIELD
 *                             LORA - D7 moved to A1
 *                             Rain - D8 moved to A3
 *                             Anemometer - D6 moved to A2
 *          2022-04-10  RJB Version 6
 *                          Added support for 3rd Party Sim
 *                          Changed guage to gauge - proper spelling
 *                          Added i2c power reset relay board ctrl on A0 
 *          2022-08-01  RJB Version 7
 *                          Wind direction check for 0-360 value
 *                          Nan Check on t,h,p sensor values, return -999.99 for bad t and 0.0 for h,p
 *          2022-09-23 RJB Version 8
 *                          Modified SystemStatusBits handling of SSB_FROM_N2S and SSB_N2S
 *                          Added Support for 8 line OLED - Special Compile
 *          2022-10-17 RJB Version 9
 *                          Increased N2S file size to 2 days and set float precision to 1/10th.
 *          2022-10-24 RJB  Support of Argon board added
 *                            Note: PMIC (BQ24195) not available on Argon (Gen 3)
 *                                  Argon uses XC6802
 *                                  No power off on low battery support with Argon
 *                          Wifi AuthType, SSID, Password stored in file WIFI.txt
 *          2022-10-31 RJB Larger OLED for Argon
 *          2022-11-03 RJB WIFI.TXT changed from space to comma as the separator
 *                         Wait for serial While loop put back
 *                         Added a 120s timeout to wait for serial.
 *          2022-11-17 RJB Merged changes from MKR NB1500 code base. A lot of the initialization
 *                         was moved out of setup() to function calls for the initialization.
 *                         Argon only - Added a System.reset() after 1 hour with out a WiFi Connection
 *          2022-12-21 RJB added clearing of SSB_HTU21DF in JPO_ClearBits()
 *          2023-01-13 RJB Added MMA Barbados Product ID
 *          2023-03-14 RJB Added VEML7700 light sensor support
 *          2023-04-19 RJB Added Support for Watchdog Monitor Relay Board.
 *                         Moved from 3.0.0 Firmware to 4.0.2 
 *          2023-04-19 RJB Version 11 
 *                         Clear SI and Lux Status bits if not found
 *          2023-06-07 RJB Rain gauge inturrupts turn on LED, Background work shuts it off
 *          2023-06-12 RJB Compiled for firmware 4.1.0
 *          2023-07-10 RJB Added Min Max Quality Controls to sensor values
 *          2023-07-11 RJB Added Daily Reboot
 *          2023-07-25 RJB Added Particle WITH_ACK check and refresh of wind data after sending
 *          2023-07-30 RJB Fixed bug to get RTC updated from network time
 *          2023-08-09 RJB Refresh of wind data only happend if it took 2 seconds or more to transmit.
 *                         Added EEPROM of N2S file position pointer and Total Rain Today and Prior Day
 *          2023-08-24 RJB Removed code supporting the non Grove shield option
 *          2023-08-31 RJB Moved to Particle function RESET (NOW) to DoAction(REBOOT). Added support
 *                         for DoAction(CRT) to Clear Rain Totals from EEPROM
 *          2023-10-05 RJB Version 18
 *                         Moved Serial Console Pin A4 to D8
 *                         Added Dynamic configuration for pin A4
 *                            If SD file A4DIST.TXT exists then Distance Gauge
 *                            If SD file A4RAIN.TXT exists then 2nd Rain Gauge
 *                         Added support for setting pin A4 from Particle
 *                           DoAction Commands  A4DIST, A4RAIN, A4CLR - Requires DoAction(REBOOT) after!
 *          2023-10-19 RJB Version 19
 *                         Added support for determining if oled is 32 or 64 pixels high
 *                         Added support for Air Quality Sensor PM25AQI
 *          2023-10-25 RJB Version 21 Actual Release
 *                         Bug Fix on Distance reading. 
 *                         Added Dynamic configuration for Distance Sensor and related distance calc
 *                               If SD file 5MDIST.TXT exists then distance reading is divided by 8
 *                               If SD file 5MDIST.TXT does NOT exist then distance is divided by 4, the default
 *                         Added support for setting distance meter type from Particle
 *                           DoAction Command 5MDIST - No reboot needed if return code to Particle is 0
 *                           If you send/resend A4DIST, the 5MDIST.TXT is deleted. 
 *          2023-12-19 RJB Version 22
 *                         Modified WiFi Credentials Setup 
 *                         Now save unreported rain before daily reboot
 *          2024-01-05 RJB Code fix for Daily Totals rotation
 *          2024-01-14 RJB Version 23
 *                         Code Fix added eeprom_valid = true; in the Initialize
 *          2024-01-31 RJB Version 24
 *                         Special for stations using A3 to rain/D8 and A2 to Wind/D6
 *          2024-02-23 RJB Version 25
 *                         Corrected Distance calculation
 *                         If file exists then we multiply by 1.25. If no file, then we multiply by 2.5 for the 10m Sensor
 *          2024-05-29 RJB Version 26
 *                         Bug fixes in WiFiChangeCheck() and SimChangeCheck()
 *                         Needed to add #if PLATFORM_ID == PLATFORM_BORON around the getting of IMSI information
 *                         Added output of the length of the particle payload when we send
 *                         Added support for wet bulb temperature (wbt)
 *          2024-06-11 RJB Version 27/28
 *                         Split code up into include files
 *                         Added CONFIG.TXT support
 *                         Moved LoRa configs to CONFIG.TXT
 *                         Added support for heat index (hi)
 *                         Added support for WBGT
 *                         Modified SF.h - isValidHexString(), hexStringToUint32(), SDC.h - SD_findKey()
 *          2024-06-11 RJB Version 29 Released
 *                         Bug Bix had = and not == on QC check for HI, WBT, WBGT
 *                         Bug Fix had 0.034 and and 0.0034 in WBGT eq.
 *                         Removed QC check for WBGT
 *                         Change QC for heat index to display a temperature
 * 
 *          Version 30 Released on 2024-09-14
 *          2024-06-20 RJB Now prints message when CONFIG.TXT not found on SD
 *                         Added Apache Copyright License
 *                         Added printing of Copyright to Serial Console
 *          2024-07-23 RJB Added dg_adjustment = 2.5 when removing 5MDIST.TXT
 *          2024-08-22 RJB Addded support for Globe temperature.
 *                         Added MCP_3 dedicated to Globe temperature will report as gt1
 *                         gt1 will be used when calling wbt_calculate()
 *          2024-09-09 RJB Added MCP_4 dedicated to Globe temperature will report as gt2
 *          2024-09-11 RJB Station Monitor changed to provide wind speed not interrupt count
 *                         When setting SIM to INTERNAL we now set changed = true to
 *                         report success and reboot message.
 *                         Changed Td to Ta in the wbgt_using_wbt() function
 *          2024-09-14 RJB Modified WiFi Support for UNSEC allowing no password Ex:  "UNSEC,ssid,"
 * 
 *          Version 31 Released on 2024-10-04
 *          2024-10-02 RJB Tag name changes rg2t -> rgt2 and rg2p -> rgp2
 * 
 *          Version 32 Released on 2024-10-07
 *          2024-10-07 RJB Improved hi_calculate() function. 
 * 
 *          Version 33 Released on 2024-11-04 (Early Relese to Argentina)
 *          2024-11-04 RJB Added INFO_Do() at boot and when called via Do_Action with "INFO"
 *                         Added Do_Action feature "SEND" to send OBS that are cued
 *                         Added support for HDC302x sensor, Reports as hdt1,hsh1,hdt2,hdh2
 *                         Added support for 5,10,15(default) minute obs transmit intervals.
 *                         Moved reporting of SystemStatusBits to after we have read all the sensors in OBS_Do()
 * 
 *          Version 34 Released on 2024-11-05 (Early Relese to Argentina)
 *          2024-11-05 RJB Discovered BMP390 first pressure reading is bad. Added read pressure to bmx_initialize()
 *                         SEE: https://forums.adafruit.com/viewtopic.php?t=209906
 *                         Bug fixes for 2nd BMP sensor in bmx_initialize() using first sensor data structure
 *                         Now will only send humidity if bmx sensor supports it.
 * 
 *          Version 35 Released on 2024-11-13
 *          2024-11-09 RJB HDC302x added sensor read in the sensor init
 *          2024-11-13 RJB DFRobot_B_LUX_V30B support added  0 lumens to 200,000 lumens. Reports a blx
 *                         Changed vmel lx tag vlx when sending to Particle
 *   
 *          Version 37 Released on 2024-12-11
 *          2024-11-19 RJB Store INFO information in INFO.TXT. Every INFO call will overwrite the files content.
 *          2024-11-26 RJB Some minor changes made in the code
 *                         INFO will now report DailyRebootCountDownTimer as drct
 *                         Improved blx_takereading() by adding a 1s timer to wait for the 4 bytes. Avoids infinite loop.
 *                         Modified Adafruit_HDC302x/src/Adafruit_HDC302x.cpp library. So sendCommandReadTRH() avoids infinite loop.
 *          2024-12-04 RJB Updated Adafruit_VEML7700 library to 2.1.6
 *          2024-12-06 RJB Added support for lps35hw pressure and temperature lpp1,lpt1,lpp2,lpt2
 *                         Upgrading to use deviceOS 6.1.1
 *          2024-12-09 RJB INFO msg now sent before powering down do to low lipo battery.
 * 
 *          Version 39 Released on 2025-01-21
 *          2025-01-07 RJB Moved LORA_IRQ_PIN from A5 to D6.
 *          2025-01-14 RJB Rebuilt the LoRa message handling. 
 *                         Now we store then forward to Particle LoRa message types: INFO, RS, SG.
 *                         On LoRa messages received, the JSON portion is forwarded 
 *                         Added RG and associated pin to INFO  
 *                         We now call OBS_PublishAll() before daily rebooting and powering down on low battery
 *                         We moved low battery powerdown from 10% to 15%.
 *          2025-01-21 RJB Added support for A4 to be configured for raw readings (Simple average of 5 samples) 10ms apart
 *                           DoAction A4RAW. Reports to Particle as a4r 
 *                         Added support for A5 to be configured for raw readings (Simple average of 5 samples) 10ms apart
 *                           DoAction A5RAW and A5CLR. Reports to Particle as a5r 
 *          2025-01-23 RJB Added support for Tinovi moisture sensors        
 *                                       
 * NOTES:
 * When there is a successful transmission of an observation any need to send obersavations will be sent. 
 * On transmit a failure of these need to send observations, processing is stopped and the file is deleted.
 * 
 * Calibration
 * Adding serial console jumper after boot will cause sensors to be read every 1 second and values printed.
 * Removing serial console jumper will resume normal operation
 * 
 * Required Libraries:
 *  adafruit-HTU21DF        https://github.com/adafruit/Adafruit_HTU21DF_Library - 1.1.0 - I2C ADDRESS 0x40
 *  adafruit-BusIO          https://github.com/adafruit/Adafruit_BusIO - 1.8.2
 *  Adafruit_MCP9808        https://github.com/adafruit/Adafruit_MCP9808_Library - 2.0.0 - I2C ADDRESS 0x18
 *  Adafruit_BME280         https://github.com/adafruit/Adafruit_BME280_Library - 2.1.4 - I2C ADDRESS 0x77  (SD0 to GND = 0x76)
 *  Adafruit_BMP280         https://github.com/adafruit/Adafruit_BMP280_Library - 2.3.0 -I2C ADDRESS 0x77  (SD0 to GND = 0x76)
 *  Adafruit_BMP3XX         https://github.com/adafruit/Adafruit_BMP3XX - 2.1.0 I2C ADDRESS 0x77 and (SD0 to GND = 0x76)
 *  Adafruit_GFX            https://github.com/adafruit/Adafruit-GFX-Library - 1.10.10
 *  Adafruit_Sensor         https://github.com/adafruit/Adafruit_Sensor - 1.1.4
 *  Adafruit_SHT31          https://github.com/adafruit/Adafruit_SHT31 - 2.2.0 I2C ADDRESS 0x44 and 0x45 when ADR Pin High
 *  Adafruit_VEML7700       https://github.com/adafruit/Adafruit_VEML7700/ - 2.1.2 I2C ADDRESS 0x10
 *  Adafruit_SI1145         https://github.com/adafruit/Adafruit_SI1145_Library - 1.1.1 - I2C ADDRESS 0x60
 *  Adafruit_SSD1306        https://github.com/adafruit/Adafruit_SSD1306 - 2.4.6 - I2C ADDRESS 0x3C  
 *  Adafruit_PM25AQI        https://github.com/adafruit/Adafruit_PM25AQI - 1.0.6 I2C ADDRESS 0x12 - Modified to Compile, Adafruit_PM25AQI.cpp" line 104
 *  Adafruit_HDC302x        https://github.com/adafruit/Adafruit_HDC302x - 1.0.2 I2C ADDRESS 0x46 and 0x47 ( SHT uses 0x44 and x045)
 *  Adafruit_LPS35HW        https://github.com/adafruit/Adafruit_LPS35HW - 1.0.6 I2C ADDRESS 0x5D and 0x5C
 *  DFRobot_B_LUX_V30B      https://github.com/DFRobot/DFRobot_B_LUX_V30B - 1.0.1 I2C ADDRESS 0x4A (Not Used Reference Only) SEN0390
 *                          https://wiki.dfrobot.com/Ambient_Light_Sensor_0_200klx_SKU_SEN0390
 *  RTCLibrary              https://github.com/adafruit/RTClib - 1.13.0
 *  SdFat                   https://github.com/greiman/SdFat.git - 1.0.16 by Bill Greiman
 *  RF9X-RK-SPI1            https://github.com/rickkas7/AdafruitDataLoggerRK - 0.2.0 - Modified RadioHead LoRa for SPI1
 *  AES-master              https://github.com/spaniakos/AES - 0.0.1 - Modified to make it compile
 *  CryptoLW-RK             https://github.com/rickkas7/CryptoLW-RK - 0.2.0
 *  HIH8000                 No Library, Local functions hih8_initialize(), hih8_getTempHumid() - rjb
 *  SENS0390                https://wiki.dfrobot.com/Ambient_Light_Sensor_0_200klx_SKU_SEN0390 - DFRobot_B_LUX_V30B - 1.0.1 I2C ADDRESS 0x94
 *  EEPROM                  https://docs.particle.io/reference/device-os/api/eeprom/eeprom/
 *                          On Gen 3 devices (Argon, Boron, B Series SoM, Tracker SoM, and E404X) 
 *                          the EEPROM emulation is stored as a file on the flash file system. 
 *                          Since the data is spread across a large number of flash sectors, 
 *                          flash erase-write cycle limits should not be an issue in general.
 * 
 *  Dallas OneWire          https://github.com/particle-iot/OneWireLibrary - Not supported on Boron/Argon
 *                          WARNING:This library has not been updated for the Argon, Boron, Tracker One, Photon 2/P2.
 *                          https://docs.particle.io/reference/device-os/libraries/o/OneWire/
 * 
 * Distance Sensors
 * The 5-meter sensors (MB7360, MB7369, MB7380, and MB7389) use a scale factor of (Vcc/5120) per 1-mm.
 * Particle 12bit resolution (0-4095),  Sensor has a resolution of 0 - 5119mm,  Each unit of the 0-4095 resolution is 1.25mm
 * 
 * Tinovi Moisture Sensors
 * Non-Contact Capacitive leaf wetness, Temperature sensor
 * https://tinovi.com/shop/i2c-non-contact-capacitive-leaf-wetness-temperature/
 *   https://github.com/tinovi/i2cArduino
 *   https://tinovi.com/wp-content/uploads/2022/08/PM-WCS-3-I2C.pdf
 *   i2c:0x61
 * 
 * PM-WCS-3-I2C I2C Non-Contact Capacitive Soil Moisture, Temperature sensor
 * https://tinovi.com/shop/i2c-capacitive-soil-moisture-temperature-and-ec-sensor-variation-cable/
 *   https://github.com/tinovi/LeafArduinoI2c
 *   https://tinovi.com/wp-content/uploads/2021/10/Leaf-Wetness-i2c-2021-10-11.pdf
 *   i2c:0x63
 * 
 * SOIL-MULTI-5-I2C I2C Capacitive multi level soil moisture, temperature sensor
 * https://tinovi.com/shop/soil-multi-5-i2c-i2c-capacitive-soil-moisture-temperature-sensor/
 *   https://github.com/tinovi/i2cMultiSoilArduino/tree/master/lib
 *   https://tinovi.com/wp-content/uploads/2024/07/SOIL-MULTI-5-I2C.pdf
 *   i2c: 0x65
 *
 * Battery Charger Status from System.batteryState()
 *  0 = BATTERY_STATE_UNKNOWN
 *  1 = BATTERY_STATE_NOT_CHARGING
 *  2 = BATTERY_STATE_CHARGING
 *  3 = BATTERY_STATE_CHARGED
 *  4 = BATTERY_STATE_DISCHARGING
 *  5 = BATTERY_STATE_FAULT
 *  6 = BATTERY_STATE_DISCONNECTED
 *
 * Publish to Particle
 *  Event Name: FS
 *  Event Variables:
 *  at      timestamp
 *  rg      rain gauge 1
 *  rgt     rain gauge 1 total today
 *  rgp     rain gauge 1 total prior
 *  rg2     rain gauge 2
 *  rgt2    rain gauge 2 total today
 *  rgp2    rain gauge 2 total prior
 *  ws      wind speed
 *  wd      wind direction
 *  wg      wind gust
 *  wgd     wind gust direction
 *  bp1     bmx_pressure
 *  bt1     bmx_temperature
 *  bh1     bmx_humidity
 *  bp2     bmx_pressure
 *  bt2     bmx_temperature
 *  bh2     bmx_humidity
 *  mt1     mcp_temp MCP1
 *  mt2     mcp_temp MCP2
 *  gt1     globe_temp MCP3
 *  gt2     globe_temp MCP4
 *  hh1     htu_humidity
 *  ht1     htu_temperature
 *  st1     sht_temperature
 *  sh1     sht_humidity
 *  st2     sht_temperature
 *  sh2     sht_humidity
 *  hdt1    hdc_temperature
 *  hdh1    hdc_humidity
 *  hdt2    hdc_temperature
 *  hdh2    hdc_humidity
 *  ht2     hih_temperature
 *  hh2     hih_humidity
 *  lpp1    lps35hw_humidity
 *  lpt1    lps35hw_temperature
 *  lpp2    lps35hw_humidity
 *  lpt2    lps35hw_temperature
 *  sv1     si_visible
 *  si1     si_infrared
 *  su1     su_ultraviolet
 *  vlx     veml_lux
 *  blx     blux30_lux
 *  sg      snow / stream gauge
 *  pm1s10  Standard Particle PM1.0
 *  pm1s25  Standard Particle PM2.5
 *  pm1s100 Standard Particle PM10.0 
 *  pm1e10  Atmospheric Environmental PM1.0
 *  pm1e25  Atmospheric Environmental PM2.5
 *  pm1e100 Atmospheric Environmental PM10.0
 *  hi      heat index
 *  wbt     wet bulb temperature
 *  wbgt    wet bub globe temperature
 *  tlwt    Tinovi Leaf Wetness temperature    
 *  tlww    Tinovi Leaf Wetness
 *  tsmt    Tinovi Soil Moisture temperature
 *  tsme25  Tinovi Soil Moisture e25
 *  tsmec   Tinovi Soil Moisture ec
 *  tsmvwc  Tinovi Soil Moisture vwc
 *  tmsmt   Tinovi Multi Level Soil Moisture
 *  tmsms1  Tinovi Multi Level Soil Moisture Soil Sensor 1 vwc
 *  tmsms2  Tinovi Multi Level Soil Moisture Soil Sensor 2 vwc
 *  tmsms3  Tinovi Multi Level Soil Moisture Soil Sensor 3 vwc
 *  tmsms4  Tinovi Multi Level Soil Moisture Soil Sensor 4 vwc
 * 
 * State of Health - Variables included with transmitted sensor readings
 *  bcs  = Battery Charger Status
 *  bpc  = Battery Percent Charge
 *  cfr    Charger Fault Register
 *  css  = Cell Signal Strength - percentage (0.0 - 100.0)
 *  hlth = Health 32bits - See System Status Bits in below define statements
 * 
 * AN002 Device Powerdown
 * https://support.particle.io/hc/en-us/articles/360044252554?input_string=how+to+handle+low+battery+and+recovery
 * 
 * NOTE: Compile Issues
 * If you have compile issues like multiple definations of functions then you need to clean the compile directory out
 *    ~/.particle/toolchains/deviceOS/2.0.1/build/target/user/...
 * 
 * Modified Libraries
 * In Adafruit_BMP280/src/Adafruit_BMP280.h Lines 39,40  Comment out the below 2 lines
 *   extern TwoWire Wire;
 *   extern SPIClass SPI;
 *   Uncommented code for takeForcedMeasurement()
 * In Adafruit_Sensor/src
 *   cp /dev/null Adafruit_Sensor.cpp
 * 
 * In Adafruit_HDC302x/src/Adafruit_HDC302x.cpp
 *   Read modified so while does not block
 *   int retries = 0;
 *   const int MAX_RETRIES = 10;
 *   while (!i2c_dev->read(buffer, 6)) {
 *     if (++retries >= MAX_RETRIES) {
 *       return false; // CRC check failed
 *     }
 *   }
 * 
 * DFRobot_B_LUX_V30B Library Not used it. It's bit banging with possible infinate loops - RJB
 * 
 * PIN Assignments
 * D8   = Serial Console (Ground Pin to Enable) - Not on Grove Shield
 * D7   = On Board LED - Lit when rain gauge tips, blinks when console connection needed
 * D6   = Reserved for Lora IRQ - Not on Grove Shield
 * D5   = SD Card Chip Select
 * D4   = SPI1 MSIO - Reserved for LoRa
 * D3   = SPI1 MOSI - Reserved for LoRa
 * D2   = SPI1 SCK  - Reserved for LoRa
 * D1   = I2C SCL
 * D0   = I2C SDA
 * 
 * A0   = WatchDog Monitor/Relay Reset Trigger
 * A1   = WatchDog Monitor Heartbeat
 * A2   = Wind Speed IRQ
 * A3   = Rain Gauge IRQ
 * A4   = 2nd Rain Gauge or Distance Gauge based on SD card file existing
 * A5   = Future
 * D13  = SPIO CLK   SD Card
 * D12  = SPI0 MOSI  SD Card
 * D11  = SPI0 MISO  SD Card
 * D10  = UART1 RX - Reserved for LoRa CS
 * D9   = UART1 TX - Reserved for LoRa RESET
 * 
 * Connection code based on below
 * https://community.particle.io/t/calling-particle-disconnect-after-a-failed-particle-connect-does-not-stop-led-from-blinking-green/19723/6
 * 
 * Max Particle message size 622 characters 
 * 
 * ========================================================
 * Support for 3rd Party Sim 
 * ========================================================
 *   SEE https://support.particle.io/hc/en-us/articles/360039741113-Using-3rd-party-SIM-cards
 *   SEE https://docs.particle.io/cards/firmware/cellular/setcredentials/
 *   Logic
 *     Output how sim is configured (internal or external)
 *     If console is enabled and SD found and SIM.TXT exists at the top level of SD card
 *       Read 1st line from SIM.TXT. Parse line for one of the below patterns
 *        INTERNAL
 *        AUP epc.tmobile.com username passwd
 *        UP username password
 *        APN epc.tmobile.com
 *      Perform appropriate actions to set sim
 *      Rename file to SIMOLD.TXT, so we don't do this on next boot
 *      Output notification to user to reboot then flash board led forever
 *
 * ========================================================
 * Support for Argon WiFi Boards
 * https://docs.particle.io/reference/device-os/api/wifi/wifi/
 * ========================================================
 * At the top level of the SD card make a file called WIFI.TXT
 * Add one line to the file
 * This line has 3 items that are comma separated Example
 * 
 * AuthType,ssid,password
 * 
 * Where AuthType is one of these keywords (WEP WPA WPA2 UNSEC)
 * Blank password is supported for UNSEC
 * ======================================================================================================================
 * 
 * Collecting Wind Data
 * Wind_SampleSpeed() - Return a wind speed based on how many interrupts and duration between calls to this function
 * Wind_SampleDirection() - Talk i2c to the AS5600 sensor and get direction
 * Wind_TakeReading() - Call this function every second. It calls wind direction and wind speed functions. Then saves samples in a circular buffer of 60 buckets.
 *
 * Creating the Wind Oobservations
 * Wind_DirectionVector() - Uses the 60 sample buckets of wind direction where speed is greater than zero to compute and return a wind vector.
 * Wind_SpeedAverage() - Use the 60 sample buckets of wind speed to return a wind speed average.
 * Wind_GustUpdate() - Uses the most current highest 3 consecutive samples of wind speed from the 60 samples. The 3 samples are then averaged for wind gust. 
 *   Variables wind.gust and wind.gust_directionare set by this function. 
 *   Call this function before calling Wind_Gust() and Wind_GustDirection()
 * Wind_Gust() - Returns wind.gust
 * Wind_GustDirection() - Returns wind.gust_direction
 * ======================================================================================================================
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_MCP9808.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_VEML7700.h>
#include <Adafruit_PM25AQI.h>
#include <Adafruit_HDC302x.h>
#include <Adafruit_LPS35HW.h>
#include <RTClib.h>
#include <SdFat.h>
#include <RH_RF95.h>
#include <AES.h>
#include <i2cArduino.h>
#include <LeafSens.h>
#include <i2cMultiSm.h>

/*
 * ======================================================================================================================
 *  Loop Timers
 * ======================================================================================================================
 */
#define DELAY_NO_RTC               60000    // Loop delay when we have no valided RTC
#define OBSERVATION_INTERVAL       60000    // 60000 = 1 minute
#define DEFAULT_OBS_TRANSMIT_INTERVAL 15    // Transmit observations every N minutes Set to 15 for 15min Transmits

/*
 * ======================================================================================================================
 *  Relay Power Control Pin
 * ======================================================================================================================
 */
#define REBOOT_PIN            A0  // Trigger Watchdog or external relay to cycle power
#define HEARTBEAT_PIN         A1  // Watchdog Heartbeat Keep Alive

/*
 * ======================================================================================================================
 * System Status Bits used for report health of systems - 0 = OK
 * An unsigned long is 32 bits
 * 
 * OFF =   SSB &= ~SSB_PWRON
 * ON =    SSB |= SSB_PWROFF
 * 
 * ======================================================================================================================
 */
#define SSB_PWRON           0x1       // Set at power on, but cleared after first observation
#define SSB_SD              0x2       // Set if SD missing at boot or other SD related issues
#define SSB_RTC             0x4       // Set if RTC missing at boot
#define SSB_OLED            0x8       // Set if OLED missing at boot, but cleared after first observation
#define SSB_N2S             0x10      // Set when Need to Send observations exist
#define SSB_FROM_N2S        0x20      // Set in transmitted N2S observation when finally transmitted
#define SSB_AS5600          0x40      // Set if wind direction sensor AS5600 has issues                                                        
#define SSB_BMX_1           0x80      // Set if Barometric Pressure & Altitude Sensor missing
#define SSB_BMX_2           0x100     // Set if Barometric Pressure & Altitude Sensor missing
#define SSB_HTU21DF         0x200     // Set if Humidity & Temp Sensor missing
#define SSB_SI1145          0x400     // Set if UV index & IR & Visible Sensor missing
#define SSB_MCP_1           0x800     // Set if MCP9808 I2C Temperature Sensor missing
#define SSB_MCP_2           0x1000    // Set if MCP9808 I2C Temperature Sensor missing
#define SSB_MCP_3           0x2000    // Set if MCP9808 I2C Temperature Sensor missing
#define SSB_LORA            0x4000    // Set if LoRa Radio missing at startup
#define SSB_SHT_1           0x8000    // Set if SHTX1 Sensor missing
#define SSB_SHT_2           0x10000   // Set if SHTX2 Sensor missing
#define SSB_HIH8            0x20000   // Set if HIH8000 Sensor missing
#define SSB_VLX             0x40000   // Set if VEML7700 Sensor missing
#define SSB_PM25AQI         0x80000   // Set if PM25AQI Sensor missing
#define SSB_HDC_1           0x100000  // Set if HDC302x I2C Temperature Sensor missing
#define SSB_HDC_2           0x200000  // Set if HDC302x I2C Temperature Sensor missing
#define SSB_BLX             0x400000  // Set if BLUX30 I2C Sensor missing
#define SSB_LPS_1           0x800000  // Set if LPS35HW I2C Sensor missing
#define SSB_LPS_2           0x1000000 // Set if LPS35HW I2C Sensor missing
#define SSB_TLW             0x2000000 // Set if Tinovi Leaf Wetness I2C Sensor missing
#define SSB_TSM             0x4000000 // Set if Tinovi Soil Moisture I2C Sensor missing
#define SSB_TMSM            0x8000000 // Set if Tinovi MultiLevel Soil Moisture I2C Sensor missing


/*
  0  = All is well, no data needing to be sent, this observation is not from the N2S file
  16 = There is N2S data, This observation is not from the N2S file
  32 = This observation is from the N2S file. And when it was saved to the N2S file, the N2S file did not exist. So it is the first observation from the file.
  48 = This observation is from the N2S file. And when it was saved to the N2S file, the N2S file existed and this observation was appended.
*/ 

#define MAX_MSGBUF_SIZE 1024

/*
 * ======================================================================================================================
 *  Globals
 * ======================================================================================================================
 */

char msgbuf[MAX_MSGBUF_SIZE]; // Used to hold messages
char *msgp;                   // Pointer to message text
char Buffer32Bytes[32];       // General storage
int  LED_PIN = D7;            // Built in LED
bool TurnLedOff = false;      // Set true in rain gauge interrupt
unsigned long SystemStatusBits = SSB_PWRON; // Set bit 1 to 1 for initial value power on. Is set to 0 after first obs
bool JustPoweredOn = true;    // Used to clear SystemStatusBits set during power on device discovery
bool SendSystemInformation = true; // Send System Information to Particle Cloud. True means we will send at boot.

uint64_t lastOBS = 0;         // time of next observation
int countdown = 600;          // Exit station monitor/mode - when countdown reaches 0
                              // Protects against burnt out pin or forgotten jumper

uint64_t LastTimeUpdate = 0;
uint64_t LastTransmitTime = 0;

int  cf_reboot_countdown_timer = 79200; // There is overhead transmitting data so take off 2 hours from 86400s
                                        // Set to 0 to disable feature
int DailyRebootCountDownTimer;

uint64_t obs_tx_interval = DEFAULT_OBS_TRANSMIT_INTERVAL;  // Default OBS Transmit interval 15 Minutes

char imsi[16] = "";  // International Mobile Subscriber Identity

#if PLATFORM_ID == PLATFORM_BORON
const char* pinNames[] = {
    "A0", "A1", "A2", "A3", "A4", "A5",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8",
    "D9", "D10", "D11", "D12", "D13", "D14", "D15",
    "SDA", "SCL", "TX", "RX", "MISO", "MOSI", "SCK", "SS",
    "WKP", "VUSB", "Li+"
};
#endif

#if PLATFORM_ID == PLATFORM_ARGON
const char* pinNames[] = {
    "A0", "A1", "A2", "A3", "A4", "A5",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8",
    "D9", "D10", "D11", "D12", "D13", "D14", "D15", "D16", "D17", "D18", "D19",
    "SDA", "SCL", "TX", "RX", "MISO", "MOSI", "SCK", "SS",
    "WKP", "VUSB", "Li+", "EN", "3V3", "GND"
};
#endif

/*
 * ======================================================================================================================
 *  SD Card Stuff
 * ======================================================================================================================
 */
#define SD_ChipSelect D5                // GPIO 10 is Pin 10 on Feather and D5 on Particle Boron Board
SdFat SD;                               // File system object.
File SD_fp;
char SD_obsdir[] = "/OBS";              // Store our observations in this directory. At Poewer on it is created if not exist
bool SD_exists = false;                 // Set to true if SD card found at boot
char SD_n2s_file[] = "N2SOBS.TXT";      // Need To Send Observation file
uint32_t SD_n2s_max_filesz = 512 * 60 * 48;  // Keep a little over 2 days. When it fills, it is deleted and we start over.

char SD_sim_file[] = "SIM.TXT";         // File used to set Ineternal or External sim configuration
char SD_simold_file[] = "SIMOLD.TXT";   // SIM.TXT renamed to this after sim configuration set

char SD_wifi_file[] = "WIFI.TXT";       // File used to set WiFi configuration

char SD_TX5M_FILE[]  = "TXI5M.TXT";     // Transmit every 5 Minutes 
char SD_TX10M_FILE[] = "TXI10M.TXT";    // Transmit every 10 Minutes

char SD_INFO_FILE[] = "INFO.TXT";       // Store INFO information in this file. Every INFO call will overwrite content


#if PLATFORM_ID == PLATFORM_BORON
/*
 * ======================================================================================================================
 *  Power Management IC (bq24195)
 * ======================================================================================================================
 */
PMIC pmic;
#endif

/*
 * ======================================================================================================================
 *  Local Code Includes - Do not change the order of the below 
 * ======================================================================================================================
 */
#include "QC.h"                   // Quality Control Min and Max Sensor Values on Surface of the Earth
#include "SF.h"                   // Support Functions
#include "OP.h"                   // OutPut support for OLED and Serial Console
#include "CF.h"                   // Configuration File Variables
#include "TM.h"                   // Time Management
#include "LoRa.h"                 // LoRa
#include "Sensors.h"              // I2C Based Sensors
#include "WRD.h"                  // Wind Rain Distance
#include "EP.h"                   // EEPROM
#include "SDC.h"                  // SD Card
#include "OBS.h"                  // Do Observation Processing
#include "SM.h"                   // Station Monitor
#include "PS.h"                   // Particle Support Functions
#include "INFO.h"                 // Station Information

/*
 * ======================================================================================================================
 * HeartBeat() - Burns 250 ms 
 * ======================================================================================================================
 */
void HeartBeat() {
  digitalWrite(HEARTBEAT_PIN, HIGH);
  delay(250);
  digitalWrite(HEARTBEAT_PIN, LOW);
}

/*
 * ======================================================================================================================
 * BackGroundWork() - Take Sensor Reading, Check LoRa for Messages, Delay 1 Second for use as timming delay            
 * ======================================================================================================================
 */
void BackGroundWork() {
  // Anything that needs sampling every second add below. Example Wind Speed and Direction, StreamGauge

  uint64_t OneSecondFromNow = System.millis() + 1000;

  Wind_TakeReading();

  if (A4_State == A4_STATE_DISTANCE) {
    DistanceGauge_TakeReading();
  }

  if (PM25AQI_exists) {
    pm25aqi_TakeReading();
  }

  HeartBeat();  // Burns 250ms

  if (LORA_exists) {
    lora_msg_poll(); // Provides a 750ms delay
  }

  int64_t TimeRemaining = (OneSecondFromNow - System.millis());
  if ((TimeRemaining > 0) && (TimeRemaining < 1000)) {
    delay (TimeRemaining);
  }

  if (TurnLedOff) {   // Turned on by rain gauge interrupt handler
    digitalWrite(LED_PIN, LOW);  
    TurnLedOff = false;
  }
}

// You must use SEMI_AUTOMATIC or MANUAL mode so the battery is properly reconnected on
// power-up. If you use AUTOMATIC, you may be unable to connect to the cloud, especially
// on a 2G/3G device without the battery.
SYSTEM_MODE(SEMI_AUTOMATIC);

// https://docs.particle.io/cards/firmware/system-thread/system-threading-behavior/
SYSTEM_THREAD(ENABLED);

/*
 * ======================================================================================================================
 * setup() - runs once, when the device is first turned on.
 * ======================================================================================================================
 */
void setup() {
  // The device has booted, reconnect the battery.
#if PLATFORM_ID == PLATFORM_BORON
	pmic.enableBATFET();
#endif

  // Set Default Time Format
  Time.setFormat(TIME_FORMAT_ISO8601_FULL);

  // WatchDog 
  pinMode (REBOOT_PIN, OUTPUT);
  pinMode (HEARTBEAT_PIN, OUTPUT);

  pinMode (LED_PIN, OUTPUT);
  
  Output_Initialize();
  delay(2000); // Prevents usb driver crash on startup, Arduino needed this so keeping for Particle

  Serial_write(COPYRIGHT);
  Output (VERSION_INFO);
  delay(4000);

  // Set Daily Reboot Timer
  DailyRebootCountDownTimer = cf_reboot_countdown_timer;

  // Initialize SD card if we have one.
  SD_initialize();

  // Report if we have Need to Send Observations
  if (SD_exists && SD.exists(SD_n2s_file)) {
    SystemStatusBits |= SSB_N2S; // Turn on Bit
    Output("N2S:Exists");
  }
  else {
    SystemStatusBits &= ~SSB_N2S; // Turn Off Bit
    Output("N2S:None");
  }

  if (SD_exists && SD.exists(CF_NAME)) {
    SD_ReadConfigFile();
  }
  else {
    sprintf(msgbuf, "CF:NO %s", CF_NAME); Output (msgbuf);
    Output(msgbuf);
  }

  // Display EEPROM Information 
  EEPROM_Dump();

  // Check if correct time has been maintained by RTC
  // Uninitialized clock would be 2000-01-00T00:00:00
  stc_timestamp();
  sprintf (msgbuf, "%s+", timestamp);
  Output(msgbuf);

  // Read RTC and set system clock if RTC clock valid
  rtc_initialize();

  if (Time.isValid()) {
    Output("STC: Valid");
  }
  else {
    Output("STC: Not Valid");
  }

  stc_timestamp();
  sprintf (msgbuf, "%s=", timestamp);
  Output(msgbuf);

#if PLATFORM_ID == PLATFORM_ARGON
	pinMode(PWR, INPUT);
	pinMode(CHG, INPUT);
  //==================================================
  // Check if we need to program for WiFi change
  //==================================================
  WiFiPrintCredentials();
  WiFiChangeCheck();
  WiFiPrintCredentials();
#else
  //==================================================
  // Check if we need to program for Sim change
  //==================================================
  SimChangeCheck();
#endif

  //==================================================
  // Wind Speed and Rain Gauge Interrupt Based Sensors
  //==================================================

  // Optipolar Hall Effect Sensor SS451A - Wind Speed
  anemometer_interrupt_count = 0;
  anemometer_interrupt_stime = System.millis();
  attachInterrupt(ANEMOMETER_IRQ_PIN, anemometer_interrupt_handler, FALLING);

  // Optipolar Hall Effect Sensor SS451A - Rain Gauge
  raingauge1_interrupt_count = 0;
  raingauge1_interrupt_stime = System.millis();
  raingauge1_interrupt_ltime = 0;  // used to debounce the tip
  attachInterrupt(RAINGAUGE1_IRQ_PIN, raingauge1_interrupt_handler, FALLING);

  // Check SD Card for files to determine Transmit Interval for OBS 5,10 or 15 minutes
  TXI_Initialize();
  
  // Check SD Card for files to determine if pin A4 has a DIST, 2nd Rain Gauge or Raw file
  A4_Initialize();

  // Check SD Card for files to determine if pin A5 Raw file
  A5_Initialize();

  // Adafruit i2c Sensors
  bmx_initialize();
  htu21d_initialize();
  mcp9808_initialize();
  sht_initialize();
  hih8_initialize();
  si1145_initialize();
  vlx_initialize();
  blx_initialize();
  as5600_initialize();
  pm25aqi_initialize();
  hdc_initialize();
  lps_initialize();

  // Tinovi Mositure Sensors
  tlw_initialize();
  tsm_initialize();
  tmsm_initialize();
  
  // Derived Observations
  wbt_initialize();
  hi_initialize();
  wbgt_initialize();
  
  // Initialize RH_RF95 LoRa Module
  lora_initialize();
  lora_device_initialize();
  lora_msg_check();

  // Connect the device to the Cloud. 
  // This will automatically activate the cellular connection and attempt to connect 
  // to the Particle cloud if the device is not already connected to the cloud.
  // Upon connection to cloud, time is synced, aka Particle.syncTime()

  // Note if we call Particle.connect() and are not truely connected to the Cell network, Code blocks in particle call
  Particle.setDisconnectOptions(CloudDisconnectOptions().graceful(true).timeout(5s));
  Particle.connect();
  
  // Setup Remote Function to DoAction, Expects a parameter to be passed from Particle to control what action
  if (Particle.function("DoAction", Function_DoAction)) {
    Output ("DoAction:OK");
  }
  else {
    Output ("DoAction:ERR");
  }
  Wind_Distance_Air_Initialize(); // Will call HeartBeat()

#if PLATFORM_ID == PLATFORM_BORON
  // Get International Mobile Subscriber Identity
  if ((RESP_OK == Cellular.command(callback_imsi, imsi, 10000, "AT+CIMI\r\n")) && (strcmp(imsi,"") != 0)) {
    sprintf (msgbuf, "IMSI:%s", imsi);
    Output (msgbuf);
  }
  else {
    Output("IMSI:NF");
  }
#endif

  if (Time.isValid()) {
    // We now a a valid clock so we can initialize the EEPROM and make an observation
    EEPROM_Initialize();
    OBS_Do();   
  }
}

/*
 * ======================================================================================================================
 * loop() runs over and over again, as quickly as it can execute.
 * ======================================================================================================================
 */
void loop() {
  // If Serial Console Pin LOW then Display Station Information
  if (0 && countdown && digitalRead(SCE_PIN) == LOW) {
    StationMonitor();
    BackGroundWork();
    countdown--;
  }
  else { // Normal Operation - Main Work

    // This will be invalid if the RTC was bad at poweron and we have not connected to Cell network
    // Upon connection to cell network system Time is set and this becomes valid
    if (Time.isValid()) {  
 
      // Set RTC from Cell network time.
      RTC_UpdateCheck();

      if (!eeprom_valid) {
        // We now a a valid clock so we can initialize the EEPROM
        EEPROM_Initialize();
      }

      // Perform an Observation, save in OBS structure, Write to SD
      if ( (System.millis() - lastOBS) > OBSERVATION_INTERVAL) {  // 1 minute
        I2C_Check_Sensors(); // Make sure Sensors are online
        OBS_Do(); 
      }

      if (SendSystemInformation && Particle.connected()) {
        INFO_Do(); // Function sets SendSystemInformation back to false.
      }

      // Time to Enable Network and Send Observations we have collected
      if ( (LastTransmitTime == 0) || ((System.millis() - LastTransmitTime) > (obs_tx_interval * 60 * 1000)) ) {
        if (Particle.connected()) {
          Output ("Connected");
          LastTransmitTime = System.millis();

          // Incase we are staying connected to the Cell network
          // request time synchronization from the Cell network - Every 4 Hours
          if ((System.millis() - LastTimeUpdate) > (4*3600*1000)) {
            // Note that this function sends a request message to the Cloud and then returns. 
            // The time on the device will not be synchronized until some milliseconds later when 
            // the Cloud responds with the current time between calls to your loop.

            // !!! What if we drop the Cell connection before we get a time update for the Cloud?
            //     If we have 15 observations to send, that 15s + 5s graceful particle disconnect
            Particle.syncTime();
            LastTimeUpdate = System.millis();
          }

          OBS_PublishAll();

          // If we waited too long for acks while publishing and this threw off our wind observations.
          // In that code ws_refresh was set to true for us to reinit wind data.
          if (ws_refresh) {
            Output ("WS Refresh Required");
            Wind_Distance_Air_Initialize();
          }

          // Update OLED and Console
          stc_timestamp();
          Output(timestamp);
          Output_CellBatteryInfo();

          // Shutoff System Status Bits related to initialization after we have logged first observation 
          JPO_ClearBits();
        }
      }

#if PLATFORM_ID == PLATFORM_ARGON
      // See if it's been an hour without a network connection and transmission of data
      // With Argon WiFi we have seen it stuck in Breathing Green - Trying to connect
      if (System.millis() - LastTransmitTime > (3600 * 1000)) {  
        // Been too long with out a network connection, lets reboot
        Output("1HR W/O NW: Rebooting");
        delay(5000);
        System.reset();
      }
#endif
      // Do background work, delays 1 Second
      BackGroundWork();
    }
    else {

      stc_timestamp();
      Output(timestamp);
      Output("ERR: No Clock");
      delay (DELAY_NO_RTC);
    }

    // ========================================================================================
    // Reboot Boot Every 22+ hours - Not using time but a loop counter.
    // ========================================================================================
    if ((cf_reboot_countdown_timer>0) && (--DailyRebootCountDownTimer<=0)) {
      Output ("Daily Reboot");

      if (Particle.connected()) {
        OBS_PublishAll();
      }

      EEPROM_SaveUnreportedRain();
      delay(1000);

      // Lets not rip the rug out from the modem. Do a graceful shutdown.
      Particle.disconnect();
      waitFor(Particle.disconnected, 1000);  // Returns true when disconnected from the Cloud.

 #if PLATFORM_ID == PLATFORM_BORON     
      // Be kind to the cell modem and try to shut it down
      Cellular.disconnect();
      delay(1000);
      Cellular.off();
#endif

      Output("Rebooting");  
      delay(1000);
   
      DeviceReset();

      // We should never get here, but just incase 
      Output("I'm Alive! Why?");  

#if PLATFORM_ID == PLATFORM_BORON
		  Cellular.on();
      delay(1000);
#endif

		  Particle.connect();

      DailyRebootCountDownTimer = cf_reboot_countdown_timer; // Reset count incase reboot fails

      // We need to reinitialize our wind readings before we can move on.
      Wind_Distance_Air_Initialize();
    }   

#if PLATFORM_ID == PLATFORM_BORON
    // ========================================================================================
    // Low Power Check and Power Off
    // ========================================================================================

    // Before we go do an observation and transmit, check our power status.
    // If we are not connected to a charging source and our battery is at a low level
    // then power down the display and board. Wait for power to return.
    // Do this at a high enough battery level to avoid the board from powering
    // itself down out of our control. Also when power returns to be able to charge
    // the battery and transmit with out current drops causing the board to reset or 
    // power down out of our control.

    // Could change the below to...
    // SEE: https://docs.particle.io/reference/device-os/api/system-calls/system-uptime/
    // int powerSource = System.powerSource();
    // if ((powerSource == POWER_SOURCE_BATTERY) && (System.batteryCharge() <= 10.0) {

    if (!pmic.isPowerGood() && (System.batteryCharge() <= 15.0)) {

      Output("Low Power!");

      if (Particle.connected()) {
        OBS_PublishAll(); 
        INFO_Do();
      }

      // While this function will disconnect from the Cloud, it will keep the connection to the network.
      Particle.disconnect();
      waitFor(Particle.disconnected, 1000);  // Returns true when disconnected from the Cloud.
      
      Cellular.disconnect();
      delay(1000);
      Cellular.off();
       
      if (LORA_exists) {
        rf95.sleep(); // Power Down LoRa. Note: it turn on when accessed
      }

      Output("Powering Down");

      OLED_sleepDisplay();
      delay(5000);

      // Disabling the BATFET disconnects the battery from the PMIC. Since there
		  // is no longer external power, this will turn off the device.
		  pmic.disableBATFET();

		  // This line should not be reached. When power is applied again, the device
		  // will cold boot starting with setup().

		  // However, there is a potential for power to be re-applied while we were in
		  // the process of shutting down so if we're still running, enable the BATFET
		  // again and reconnect to the cloud. Wait a bit before doing this so the
		  // device has time to actually power off.
		  delay(2000);

      OLED_wakeDisplay();   // May need to toggle the Display reset pin.
		  delay(2000);
		  Output("Power Re-applied");

      // WakeUp LoRa
      // May need to toggle LoRa Reset

		  pmic.enableBATFET();

		  Cellular.on();

		  Particle.connect();

      // We need to reinitialize our wind readings before we can move on.
      Wind_Distance_Air_Initialize();
    }
#endif
  }
}
