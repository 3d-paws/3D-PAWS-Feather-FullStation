#define COPYRIGHT "Copyright [2026] [University Corporation for Atmospheric Research]"
#define VERSION_INFO "FSLW-260219"  // Full Station LoRaWiFi - Release Date

/*
 *======================================================================================================================
 * Full Station LoRaWAN WiFi Node
 *   Board Type : Adafruit Feather M0 LoRaWAN Board https://www.adafruit.com/product/3178
 *   Board Type : Adafruit Feather M0 WiFi Board https://www.adafruit.com/product/3010
 *   Description: 
 *   Author: Robert Bubon
 *   Date:   2024-01-01 RJB Initial
 *           2024-03-25 RJB Swapped Rain Gauge Pins Now: A3=RG1, A2=RG2
 *           2024-04-09 RJB Limited the EQ freqs used for TTN to just 868.1 868.3 868.5
 *           2024-04-10 RJB Backed out the above just need to define the additional freqs on device at TTN
 *           2024-04-11 RJB When Serial Console Enabled, EEPRM is cleared at Boot
 *                          System status bit cleared for wind direction
 *                          System status bit setup for eeprom
 *                          Stopped clearing the GPS status bit if not found at boot
 *           2024-04-14 RJB Converted to OTAA
 *           2024-04-21 RJB Increased wait time to resend from 5s to 10. This helped with n2s sending.
 *                          Changed how long we stay in n2s sending base on interval of obs (1s, 5s 15s)
 *           2024-04-30 RJB Fixed bug in Station Monitor
 *                          Station Monitor Line 3 now delays 5s between updates
 *           2024-06-10 RJB Modified SF.h - isValidHexString(), hexStringToUint32(), SDC.h - SD_findKey()
 *                          Added HI, WBT, WBGT
 *           2024-06-23 RJB Copyright Added
 *           2024-07-17 RJB Removed #include <ArduinoLowPower.h> not needed
 *           2024-07-18 RJB Pin changes A2 Wind, A3 RG1, A4 RG2, A5, Distance
 *           2024-08-09 RJB Merged changed into LoRaWAN code base to support WiFi boards
 *                          Renamed to FullStaion LoRaWan WiFi or FSLW
 *           2024-08-14 RJB Quality check added to WiFi NTP time 
 *                          Changed OBS_Send to look for http error code 500. For N2S being sent a 500 response
 *                          means I move past that in the N2S file
 *                          If year is not valid we will not send a observation.
 *                          Added define statements for valid start and end years
 *           2024-08-25 RJB hih8_getTempHumid() modified removing Wire.endTransmission() call.
 *                          pm25aqi_initialize() now calls I2C_Device_Exist(PM25AQI_ADDRESS) instead of doing wire call directly.
 *           2024-08-26 RJB Moved the WiFi connection check / reconnect to WiFi_Send()
 *           2024-08-27 RJB Changed Distance sensor to multiply raw value by 5 for 5m, 10 for 10m, OBS will be in mm
 *           2024-10-07 RJB Improved hi_calculate() function.
 *           2024-11-05 RJB Discovered BMP390 first pressure reading is bad. Added read pressure to bmx_initialize()
 *                          Bug fixes for 2nd BMP sensor in bmx_initialize() using first sensor data structure
 *                          Now will only send humidity if bmx sensor supports it.
 *           2025-01-26 RJB Added support for Tinovi moisture sensors (Leaf, Soil, Multi Level Soil)
 *                          Added support for GetDeviceID()
 *                          Added support for observation intervals 1,5,6,10,15,20,30
 *           2025-09-11 RJB In OBS fixed casting bug on rain collection. Added (float)
 *                          (rain > (((float) rgds / 60) * QC_MAX_RG))
 *                          Added tmsms5 that was forgotten
 *           2025-09-17 RJB Renamed WRDB.h to WRDA.h
 *                          Added SD_crt_file file
 *                          Added clear rain totals CRT.TXT
 *           
 *           2026-02-19 RJB Release Major Update 
 *                          Added lora wan options to config file
 *                          Added SDU boot
 *                          SD_ClearRainTotals(); // Clear from EEPROM if CRT.TXT exists
 *                          Added INFO_Do() 
 *                          Library Update and Changes
 *                            Changed SD to SdFat 2.3.0
 *                            Add Adafruit_ZeroDMA, LeafArduinoI2c, i2cArduino, i2cMultiArd
 *                            MCCI LoRaWAN LMIC library 4.1.1 -> 5.0.1
 *                            BMP280 2.6.2 -> 2.6.8
 *                            BMP3XX 2.1.2 -> 2.1.6
 *                            BusIO 1.11.6 -> 1.17.4
 *                            GFX 1.11.1 -> 1.12.4
 *                            HTU21DF 1.1.0 -> 1.1.2
 *                            MCP9808 2.0.0 -> 2.0.2
 *                            PM25_AQI 1.0.6 -> 2.0.0
 *                            SHT31 2.2.0 -> 2.2.2
 *                            SI1145 1.2.0 -> 1.2.2
 *                            SSD1306 2.5.3-> 2.5.16
 *                            Unified_Sensor 1.1.5 -> 1.1.15
 *                            VEML7700 2.1.2 -> 2.1.6
 *                            RTClib 2.0.3 -> 2.1.4                 
 *                          Fixed bug in Wind_SampleDirection(), if AS5600 went offline we would stop doing wind. 
 *                          Moved A4/A5 to OP1/OP2 
 *                          Added OP2 option to read voltaic voltage
 *                          Documentation Added               
 *                          
 *  Future Note - Support Chords and Google Big Query
 *    OBS.h line 212 will need to be modified
 *    EX URL curl -H "Authorization: Bearer YOUR_ACCESS_TOKEN"
 *               "https://bigquery.googleapis.com/bigquery/v2/projects/YOUR_PROJECT_ID/datasets/YOUR_DATASET_ID"
 *                          
 *  Compile for EU Frequencies 
 *    cd Arduino/libraries/MCCI_LoRaWAN_LMIC_library/project_config
 *    cp lmic_project_config.h-eu lmic_project_config.h
 *  
 *  Compile for US Frequencies 
 *    cd Arduino/libraries/MCCI_LoRaWAN_LMIC_library/project_config
 *    cp lmic_project_config.h-us lmic_project_config.h
 *======================================================================================================================
 */

/*
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
 *  RTCLibrary              https://github.com/adafruit/RTClib - 1.13.0
 *  SdFat                   https://github.com/greiman/SdFat.git - 1.0.16 by Bill Greiman
 *  RF9X-RK-SPI1            https://github.com/rickkas7/AdafruitDataLoggerRK - 0.2.0 - Modified RadioHead LoRa for SPI1
 *  AES-master              https://github.com/spaniakos/AES - 0.0.1 - Modified to make it compile
 *  CryptoLW-RK             https://github.com/rickkas7/CryptoLW-RK - 0.2.0
 *  HIH8000                 No Library, Local functions hih8_initialize(), hih8_getTempHumid() - rjb
 *  SENS0390                https://wiki.dfrobot.com/Ambient_Light_Sensor_0_200klx_SKU_SEN0390 - DFRobot_B_LUX_V30B - 1.0.1 I2C ADDRESS 0x94
 *  EEPROM                  https://docs.particle.io/reference/device-os/api/eeprom/eeprom/ , https://www.adafruit.com/product/5146
 *  SpatkFun_I2C_GPS        https://github.com/sparkfun/SparkFun_I2C_GPS_Arduino_Library I2C ADDRESS 0x10
 *  LeafSens                https://github.com/tinovi/LeafArduinoI2c I2C ADDRESS 0x61
 *  i2cArduino              https://github.com/tinovi/i2cArduino     I2C ADDRESS 0x63
 *  i2cMultiSm              https://github.com/tinovi/i2cMultiSoilArduino/tree/master/lib ADDRESS 0x65
 */
 
/*
 * ======================================================================================================================
 * Pin Definitions
 * 
 * Board Label   Arduino  Info & Usage                   Grove Shield Connector   
 * ======================================================================================================================
 * RST
 * 3V            3v3 Power
 * ARef
 * GND
 * A0            A0       WatchDog Trigger               Grove A0
 * A1            A1       WatchDog Heartbeat             Grove A0
 * A2            A2       Interrupt For Anemometer       Grove A2
 * A3            A3       Interrupt For Rain Gauge 1     Grove A2
 * A4            A4       Interrupt For Rain Gauge 2     Grove A4
 * A5            A5       Distance Sensor                Grove A4
 * SCK           SCK      SPI0 Clock                     Not on Grove               
 * MOS           MOSI     Used by SD Card                Not on Grove
 * MIS           MISO     Used by SDCard                 Not on Grove
 * RX0           D0                                      Grove UART
 * TX1           D1                                      Grove UART 
 * io1           DIO1     Connects to D6 for LoRaWAN     Not on Grove (Particle Pin D9)

 * BAT           VBAT Power
 * En            Control - Connect to ground to disable the 3.3v regulator
 * USB           VBUS Power
 * 13            D13      LED                            Not on Grove 
 * 12            D12      Serial Console Enable          Not on Grove
 * 11            D11                                     Not on Grove
 * 10            D10      Used by SD Card as CS          Grove D4  (Particle Pin D5)
 * 9             D9/A7    Voltage Battery Pin            Grove D4  (Particle Pin D4)
 * 6             D6       Connects to DIO1 for LoRaWAN   Grove D2  (Particle Pin D3)
 * 5             D5                                      Grove D2  (Particle Pin D2)
 * SCL           D3       i2c Clock                      Grove I2C_1
 * SDA           D2       i2c Data                       Grove I2C_1 
 * RST

 * 
 * Not exposed on headers
 * D8 = LoRa NSS aka Chip Select CS
 * D4 = LoRa Reset
 * D3 = LoRa DIO
 * 
 * D8 = WiFI
 * D7 = WiFI
 * D4 = WiFI
 * D2 = WiFI
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * Add SDUBoot Support for UPDATE.BIN on SD card
 * ======================================================================================================================
 */
#include "boot/SDU.h"

 /* 
 *=======================================================================================================================
 * Local Includes
 *=======================================================================================================================
 */
#include "include/ssbits.h"         // System Status Bits
#include "include/qc.h"             // Quality Control Min and Max Sensor Values on Surface of the Earth
#include "include/feather.h"        // Feather Related Board Functions and Definations
#include "include/support.h"        // Support Functions
#include "include/output.h"         // Serial and OLED Output Functions
#include "include/cf.h"             // Configuration File Variables
#include "include/eeprom.h"         // EEPROM Functions
#include "include/sdcard.h"         // SD Card Functions
#include "include/time.h"           // Time Management Functions
#include "include/wifi.h"           // Wifi Functions
#include "include/gps.h"            // GPS Support Functions
#include "include/wrda.h"           // Wind Rain Distance Air Functions
#include "include/mux.h"            // Mux Functions for mux connected sensors
#include "include/lorawan.h"        // LoRaWAN Support Functions
#include "include/sensors.h"        // I2C Based Sensor Functions
#include "include/statmon.h"        // Station Monitor Functions
#include "include/obs.h"            // Observation Functions
#include "include/info.h"           // Info Functions
#include "include/main.h"

/*
 * ======================================================================================================================
 *  Globals
 * ======================================================================================================================
 * 
 */
bool JustPoweredOn = true;               // Used to clear SystemStatusBits set during power on device discovery
bool TurnLedOff = false;                 // Set true in rain gauge interrupt

char versioninfo[sizeof(VERSION_INFO)];  // allocate enough space including null terminator
char msgbuf[MAX_MSGBUF_SIZE];            // Used to hold messages
char *msgp;                              // Pointer to message text
char Buffer32Bytes[32];                  // General storage

unsigned long Time_of_obs = 0;           // unix time of observation
unsigned long Time_of_next_obs = 0;      // time of next observation

// Local
int DailyRebootCountDownTimer;
unsigned long nextinfo=0;                // Time of Next INFO transmit


/* 
 *=======================================================================================================================
 * obs_interval_initialize() - observation interval 1,5,6,10,15,20,30
 *=======================================================================================================================
 */
void obs_interval_initialize() {
  if ((cf_obs_period != 1) &&
      (cf_obs_period != 5) && 
      (cf_obs_period != 6) && 
      (cf_obs_period != 10) &&
      (cf_obs_period != 15) &&
      (cf_obs_period != 20) &&
      (cf_obs_period != 30)) {
    sprintf (Buffer32Bytes, "OBS Interval:%dm Now:1m", cf_obs_period);
    Output(Buffer32Bytes);
    cf_obs_period = 1; 
  }
  else {
    sprintf (Buffer32Bytes, "OBS Interval:%dm", cf_obs_period);
    Output(Buffer32Bytes);    
  }
}

/* 
 *=======================================================================================================================
 * time_to_next_obs() - This will return milliseconds to next observation
 *=======================================================================================================================
 */
int time_to_next_obs() {
  if (cf_obs_period == 1) {
    return (millis() + 60000); // Just go 60 seconds from now.
  }
  else {
    return ((cf_obs_period*60000) - (millis() % (cf_obs_period*60000))); // The mod operation gives us seconds passed
  }
}

/*
 * ======================================================================================================================
 * SystemReset() - reboot the feather
 * ======================================================================================================================
 */
void SystemReset() {
  // Resets the device, just like hitting the reset button or powering down and back up.
  __disable_irq();     // Prevent pending interrupts from interfering.
  NVIC_SystemReset();  // Trigger system reset [web:4]
}

/*
 * ======================================================================================================================
 * DeviceReset() - Kill power to ourselves and do a cold boot
 * ======================================================================================================================
 */
void DeviceReset() {
  digitalWrite(REBOOT_PIN, HIGH);
  delay(5000);
  // Should not get here if relay / watchdog is connected.
  digitalWrite(REBOOT_PIN, LOW);
  delay(2000); 

  // !!! May never get here when a relay board / watchdog is connected.

  // Resets the device, just like hitting the reset button or powering down and back up.
  SystemReset(); 
}

/*
 * ======================================================================================================================
 * HeartBeat() - 
 * ======================================================================================================================
 */
void HeartBeat() {
  digitalWrite(HEARTBEAT_PIN, HIGH);
  delay (250);
  digitalWrite(HEARTBEAT_PIN, LOW);
}

/*
 * ======================================================================================================================
 * BackGroundWork() - Take Sensor Reading, Check LoRa for Messages, Delay 1 Second for use as timming delay            
 * ======================================================================================================================
 */
void BackGroundWork() {
  uint64_t OneSecondFromNow = millis() + 1000;

  if (!AQS_Enabled) {
    if ((cf_op1==5)||(cf_op1==10)) {
      DS_TakeReading();
    }
    
    if (AS5600_exists) {
      Wind_TakeReading();
    }
  }

  if (PM25AQI_exists) {
    pm25aqi_TakeReading();
  }

  // Check WiFi and reconnect
  if (WiFi_valid && (WiFi.status() != WL_CONNECTED)) {
    WiFi_connect();
  }

  HeartBeat();  // Burns 250ms

  while(OneSecondFromNow > millis()) {
    if (WiFi_valid) {
      delay(50);
    }
    else {
      // LoRaWAN - Process radio callbacks. If EV_TXCOMPLETE arrives, flag clears.
      os_runloop_once(); // Run as often as we can
    }
  }

  if (!AQS_Enabled) {
    if (TurnLedOff) {   // Turned on by rain gauge interrupt handler
      digitalWrite(LED_PIN, LOW);  
      TurnLedOff = false;
    }
  }
}

/*
 * ======================================================================================================================
 * setup()
 * ======================================================================================================================
 */
void setup() {
  pinMode (LED_PIN, OUTPUT);
  Output_Initialize();
  delay(2000); // prevents usb driver crash on startup, do not omit this

  analogReadResolution(12);  //Set all analog pins to 12bit resolution reads to match the SAMD21's ADC channels

  Serial_writeln(COPYRIGHT);
  strcpy(versioninfo, VERSION_INFO);
  Output (versioninfo);

  GetDeviceID();
  sprintf (msgbuf, "DevID:%s", DeviceID);
  Output (msgbuf);

  Output (F("REBOOTPN SET"));
  pinMode (REBOOT_PIN, OUTPUT); // By default all pins are LOW when board is first powered on. Setting OUTPUT keeps pin LOW.
  
  Output (F("HEARTBEAT SET"));
  pinMode (HEARTBEAT_PIN, OUTPUT);
  HeartBeat();

  // Initialize SD card if we have one.
  Output(F("SD:INIT"));
  SD_initialize();
  if (!SD_exists) {
    Output(F("!!!HALTED!!!"));
    while (true) {
      delay(1000);
    }
  }
  
  SD_ReadConfigFile();

  obs_interval_initialize();

  INFO_Initialize();

  // Set Daily Reboot Timer
  DailyRebootCountDownTimer = cf_daily_reboot * 3600;
  
  rtc_initialize();

  gps_initialize();  // if found gps_aquire() will run;

  if (RTC_valid) {
    // We now do a valid clock before we can initialize the EEPROM and make an observation
    EEPROM_initialize();
  }

  // Check SD Card for file to determine if we are a Air Quality Station
  OPT_AQS_Initialize(); // Sets AQS_Enabled to true

  if (!AQS_Enabled) {
    // Optipolar Hall Effect Sensor SS451A - Rain1 Gauge
    if (cf_rg1_enable) {
      raingauge1_interrupt_count = 0;
      raingauge1_interrupt_stime = millis();
      raingauge1_interrupt_ltime = 0;  // used to debounce the tip
      attachInterrupt(RAINGAUGE1_IRQ_PIN, raingauge1_interrupt_handler, FALLING);
      Output (F("RG1:ENABLED"));
    }
    else {
      Output (F("RG1:NOT ENABLED"));
    }

    // Optipolar Hall Effect Sensor SS451A - Rain2 Gauge
    if (cf_op1 == 2) {
      raingauge2_interrupt_count = 0;
      raingauge2_interrupt_stime = millis();
      raingauge2_interrupt_ltime = 0;  // used to debounce the tip
      attachInterrupt(RAINGAUGE2_IRQ_PIN, raingauge2_interrupt_handler, FALLING);
      Output (F("RG2:ENABLED"));
    }
    else {
      Output (F("RG2:NOT ENABLED"));
    }

    as5600_initialize();
    if (AS5600_exists) {
      Output (F("WS:Enabled"));
      // Optipolar Hall Effect Sensor SS451A - Wind Speed
      anemometer_interrupt_count = 0;
      anemometer_interrupt_stime = millis();
      attachInterrupt(ANEMOMETER_IRQ_PIN, anemometer_interrupt_handler, FALLING);
    }
  }

  // Scan for i2c Devices and Sensors
  mux_initialize();
  if (!MUX_exists) {
    tsm_initialize(); // Check main bus
  }
  bmx_initialize();
  htu21d_initialize();
  mcp9808_initialize();
  sht_initialize();
  hih8_initialize();
  si1145_initialize();
#ifdef NOWAY
  lux_initialize();   // Can not use when GPS Module is connected same i2c address of 0x10
#endif
  pm25aqi_initialize();

  // Tinovi Mositure Sensors
  tlw_initialize();
  tsm_initialize();
  tmsm_initialize();

  // Derived Observations
  wbt_initialize();
  hi_initialize();
  wbgt_initialize();

  // See what network solution we have
  WiFi.setPins(8,7,4,2);
  if (WiFi_Feather()) {
    Output(F("BOARD:WiFi"));
    WiFi_initialize();
    (WiFi_valid) ? Output(F("WiFi Valid")) : Output(F("WiFi NotValid"));
  }
  else {
    // LoRaWAN Init
    Output(F("BOARD:LoRaWAN"));
    delay (10000);
    LW_initialize();
    if (!LW_valid) {
      Output(F("LW Disabled"));
    }
    nextinfo = millis() + 60000; // Give LoRaWAN 1 minute time to Join if OTAA
  }

  Output (F("Start Main Loop"));
  Time_of_next_obs = millis() + 60000; // Give Network some time to connect

  if (RTC_valid) {
    Wind_Distance_Air_Initialize(); // Will call HeartBeat() - Full station call.
  }
}

/*
 * ======================================================================================================================
 * loop() - 
 * ======================================================================================================================
 */
void loop() {
  static int  countdown = 1800; // How log do we stay in calibration display mode
  // Output (F("LOOP"));

  if (!RTC_valid) {
    // Must get time from user
    static bool first = true;
    
    delay (1000);
      
    if (first) {
      // Enable Serial if not already
      if (digitalRead(SCE_PIN) != LOW) {
        Serial.begin(9600);
        delay(2000);
        SerialConsoleEnabled = true;
      }  

      // Show invalid time and prompt for UTC Time
      sprintf (msgbuf, "%d:%02d:%02d:%02d:%02d:%02d", 
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());
      Output(msgbuf);
      Output(F("SET RTC w/GMT, ENTER:"));
      Output("YYYY:MM:DD:HH:MM:SS");
      first = false;
    }

    // Now two things can happen. User enters valid time or we get time from GPS


    rtc_readserial(); // check for serial input, validate for rtc, set rtc, report result

    if (!RTC_valid) {
      gps_aquire();  // This can set RTC_valid to true when valid GPS time obtained 
    }

    if (RTC_valid) {
      Output(F("!!!!!!!!!!!!!!!!!"));
      Output(F("!!! Rebooting !!!"));
      Output(F("!!!!!!!!!!!!!!!!!")); 
      delay(2000);
      SystemReset(); // We don't use the watchdog to reboot. So soft reset to keep GPS and RTC powered.
    }
  }
    //Calibration mode
  else if (countdown && digitalRead(SCE_PIN) == LOW) { 
    // Every minute, Do observation (don't save to SD) and transmit
    StationMonitor();
    
    // check for input sting, validate for rtc, set rtc, report result
    if (Serial.available() > 0) {
      rtc_readserial(); // check for serial input, validate for rtc, set rtc, report result
    }
    
    countdown--;
    BackGroundWork();
  }

  // Normal Operation
  else {
    // Delayed initialization. We need a valid clock before we can validate the EEPROM
    if (eeprom_exists && !eeprom_valid) {
      EEPROM_Validate();
      EEPROM_Dump();
      SD_ClearRainTotals(); // Clear if CRT.TXT exists
    }

    if (!WiFi_valid) {
      // Make sure we are Joined of OTAA
      if (cf_lw_mode == LORA_OTAA) {
        if (LW_NeedToJoin()) {
          // No netid and No join in progress so start a join
          LMIC_startJoining();  // Triggers join process
        }
      }
    }

    // Send INFO
    if (millis() >= nextinfo) {      // Upon power on this will be true with nextinfo being zero
      INFO_Do();   // function will set nextinfo time for next call
    }
    
    // Perform an Observation, Write to SD, Send OBS
    if (millis() >= Time_of_next_obs) {
      Output (F("Do OBS"));
      
      now = rtc.now();
      Time_of_obs = now.unixtime();
      if ((now.year() >= TM_VALID_YEAR_START) && (now.year() <= TM_VALID_YEAR_END)) {
        OBS_Do();
      }
      else {
        Output (F("OBS_Do() NotRun-Bad TM"));
      }

      Time_of_next_obs = time_to_next_obs();   
      JPO_ClearBits(); // Clear status bits from boot after we log our first observations
    }
  }

  // Reboot Boot Countdown, only if cf_daily_reboot is set
  if ((cf_daily_reboot>0) && (--DailyRebootCountDownTimer<=0)) {
    Output (F("Daily Reboot/OBS"));
    
    Time_of_obs = rtc_unixtime();
    OBS_Do();
    
    Output(F("Rebooting"));  
    delay(1000);
   
    DeviceReset();

    // We should never get here, but just incase 
    Output(F("I'm Alive! Why?"));
    DailyRebootCountDownTimer = cf_daily_reboot * 3600;
  }
  
  BackGroundWork();
}
