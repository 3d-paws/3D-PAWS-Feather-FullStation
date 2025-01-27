#define COPYRIGHT "Copyright [2024] [University Corporation for Atmospheric Research]"
#define VERSION_INFO "FSLW-250127"  // Full Station LoRaWiFi - Release Date

/*
 *======================================================================================================================
 * Full Station LoRaWAN WiFi Node
 *   Board Type : Adafruit Feather M0 (LoRaWAN or WiFi Boards)
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
 *                          
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
 *=======================================================================================================================
 * Includes
 *=======================================================================================================================
 */
// #include <SDU.h>  // Secure Digital Updater 

#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <ctime>  // Provides the tm structure
#include <WiFi101.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
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
#include <Adafruit_EEPROM_I2C.h>
#include <lmic.h>    // MCCI_LoRaWAN_LMIC_library
#include <hal/hal.h>
#include <RTClib.h>  // https://github.com/adafruit/RTClib FYI: this library has DateTime functions in it used by TimeManagement.h
#include <i2cArduino.h>
#include <LeafSens.h>
#include <i2cMultiSm.h>
#include <SparkFun_I2C_GPS_Arduino_Library.h>
#include <TinyGPS++.h>


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
#define REBOOT_PIN        A0  // Connect to WatchDog Trigger
#define HEARTBEAT_PIN     A1  // Connect to WatchDog Heartbeat
#define SCE_PIN           12  // Serial Console Enable Pin
#define LED_PIN           LED_BUILTIN

#define TM_VALID_YEAR_START     2024
#define TM_VALID_YEAR_END       2033

/*
 * ======================================================================================================================
 * System Status Bits used for report health of systems - 0 = OK
 * 
 * OFF =   SSB &= ~SSB_PWRON
 * ON =    SSB |= SSB_PWROFF
 * ======================================================================================================================
 */
#define SSB_PWRON           0x1      // Set at power on, but cleared after first observation
#define SSB_SD              0x2      // Set if SD missing at boot or other SD related issues
#define SSB_RTC             0x4      // Set if RTC missing at boot
#define SSB_OLED            0x8      // Set if OLED missing at boot, but cleared after first observation
#define SSB_N2S             0x10     // Set when Need to Send observations exist
#define SSB_FROM_N2S        0x20     // Set in transmitted N2S observation when finally transmitted
#define SSB_AS5600          0x40     // Set if wind direction sensor AS5600 has issues                                                        
#define SSB_BMX_1           0x80     // Set if Barometric Pressure & Altitude Sensor missing
#define SSB_BMX_2           0x100    // Set if Barometric Pressure & Altitude Sensor missing
#define SSB_HTU21DF         0x200    // Set if Humidity & Temp Sensor missing
#define SSB_SI1145          0x400    // Set if UV index & IR & Visible Sensor missing
#define SSB_MCP_1           0x800    // Set if Precision I2C Temperature Sensor missing
#define SSB_MCP_2           0x1000   // Set if Precision I2C Temperature Sensor missing
#define SSB_LORA            0x2000   // Set if LoRa Radio missing at startup
#define SSB_SHT_1           0x4000   // Set if SHTX1 Sensor missing
#define SSB_SHT_2           0x8000   // Set if SHTX2 Sensor missing
#define SSB_HIH8            0x10000  // Set if HIH8000 Sensor missing
#define SSB_GPS             0x20000  // Set if GPS Sensor missing
#define SSB_PM25AQI         0x40000  // Set if PM25AQI Sensor missing
#define SSB_EEPROM          0x80000  // Set if 24LC32 EEPROM missing
#define SSB_TLW             0x100000 // Set if Tinovi Leaf Wetness I2C Sensor missing
#define SSB_TSM             0x200000 // Set if Tinovi Soil Moisture I2C Sensor missing
#define SSB_TMSM            0x400000 // Set if Tinovi MultiLevel Soil Moisture I2C Sensor missing

/*
 * ======================================================================================================================
 *  Globals
 * ======================================================================================================================Y
 * 
 */
unsigned long  SystemStatusBits = SSB_PWRON;  // Set bit 1 to 1 for initial value power on. Is set to 0 after first obs
bool JustPoweredOn = true;                    // Used to clear SystemStatusBits set during power on device discovery
bool TurnLedOff = false;                      // Set true in rain gauge interrupt


int DSM_countdown = 1800; // Exit Display Station Monitor screen when reaches 0 - protects against burnt out pin or forgotten jumper

#define MAX_MSGBUF_SIZE   1024
char msgbuf[MAX_MSGBUF_SIZE];   // Used to hold messages
char *msgp;                     // Pointer to message text
char Buffer32Bytes[32];         // General storage

#define MAX_OBS_SIZE  1024
char obsbuf[MAX_OBS_SIZE];      // Url that holds observations for HTTP GET
char *obsp;                     // Pointer to obsbuf

int DailyRebootCountDownTimer;

char DeviceID[25];              // A generated ID based on board's 128-bit serial number converted down to 96bits

const char* pinNames[] = {
  "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
  "D8", "D9", "D10", "D11", "D12", "D13",
  "A0", "A1", "A2", "A3", "A4", "A5"
};

/*
 * ======================================================================================================================
 *  Local Code Includes - Do not change the order of the below 
 * ======================================================================================================================
 */
#include "QC.h"                   // Quality Control Min and Max Sensor Values on Surface of the Earth
#include "SF.h"                   // Support Functions
#include "Output.h"               // Output support for OLED and Serial Console
#include "CF.h"                   // Configuration File Variables
#include "TM.h"                   // Time Management
#include "WiFi.h"                 // WiFi
#include "GPS.h"                  // GPS Support
#include "WRDB.h"                 // Wind Rain Distance Battery
#include "EP.h"                   // EEPROM
#include "LW.h"                   // LoRaWAN
#include "SDC.h"                  // SD Card
#include "Sensors.h"              // I2C Based Sensors
#include "OBS.h"                  // Do Observation Processing
#include "SM.h"                   // Station Monitor
//#include "INFO.h"                 // Station Information

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
 * DeviceReset() - Kill power to ourselves and do a cold boot
 * ======================================================================================================================
 */
void DeviceReset() {
  digitalWrite(REBOOT_PIN, HIGH);
  delay(5000);
  // Should not get here if relay / watchdog is connected.
  digitalWrite(REBOOT_PIN, LOW);
  delay(2000); 

  // May never get here if relay board / watchdog not connected.

  // Resets the device, just like hitting the reset button or powering down and back up.
  // System.reset();
}

/*
 * ======================================================================================================================
 * BackGroundWork() - Take Sensor Reading, Check LoRa for Messages, Delay 1 Second for use as timming delay            
 * ======================================================================================================================
 */
void BackGroundWork() {
  uint64_t OneSecondFromNow = millis() + 1000;

  if (cf_ds_enable) {
    DS_TakeReading();
  }
    
  if (AS5600_exists) {
    Wind_TakeReading();
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
      os_runloop_once(); // Run as often as we can
    }
  }

  if (TurnLedOff) {   // Turned on by rain gauge interrupt handler
    digitalWrite(LED_PIN, LOW);  
    TurnLedOff = false;
  }
}
/* 
 *=======================================================================================================================
 * Wind_Distance_Air_Initialize()
 *=======================================================================================================================
 */
void Wind_Distance_Air_Initialize() {
  Output ("WDA:Init()");

  // Clear windspeed counter
  if (AS5600_exists) {
    anemometer_interrupt_count = 0;
    anemometer_interrupt_stime = millis();
  
    // Init default values.
    wind.gust = 0.0;
    wind.gust_direction = -1;
    wind.bucket_idx = 0;
  }

  // Take N 1s samples of wind speed and direction and fill arrays with values.
  if (AS5600_exists | PM25AQI_exists |cf_ds_enable) {
    for (int i=0; i< WIND_READINGS; i++) {
      BackGroundWork();
    
      if (SerialConsoleEnabled) Serial.print(".");  // Provide Serial Console some feedback as we loop and wait til next observation
      OLED_spin();
    }
    if (SerialConsoleEnabled) Serial.println();  // Send a newline out to cleanup after all the periods we have been logging
  }

  // Now we have N readings we can output readings
  
  if (AS5600_exists) {
    Wind_TakeReading();
    float ws = Wind_SpeedAverage();
    sprintf (Buffer32Bytes, "WS:%d.%02d WD:%d", (int)ws, (int)(ws*100)%100, Wind_DirectionVector());
    Output (Buffer32Bytes);
  }
  
  if (PM25AQI_exists) {
    sprintf (Buffer32Bytes, "pm1s10:%d", pm25aqi_obs.max_s10);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1s25:%d", pm25aqi_obs.max_s25);
    Output (Buffer32Bytes); 
    
    sprintf (Buffer32Bytes, "pm1s100:%d", pm25aqi_obs.max_s100);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1e10:%d", pm25aqi_obs.max_e10);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1e25:%d", pm25aqi_obs.max_e25);
    Output (Buffer32Bytes);
    
    sprintf (Buffer32Bytes, "pm1e100:%d", pm25aqi_obs.max_e100);
    Output (Buffer32Bytes);
  }

  if (cf_ds_enable) {
    sprintf (Buffer32Bytes, "DS:%d", DS_Median());
    Output (Buffer32Bytes);
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

  Serial_write(COPYRIGHT);
  Output (VERSION_INFO);

  GetDeviceID();
  sprintf (msgbuf, "DevID:%s", DeviceID);
  Output (msgbuf);

  Output ("REBOOTPN SET");
  pinMode (REBOOT_PIN, OUTPUT); // By default all pins are LOW when board is first powered on. Setting OUTPUT keeps pin LOW.
  
  Output ("HEARTBEAT SET");
  pinMode (HEARTBEAT_PIN, OUTPUT);
  HeartBeat();

  // Initialize SD card if we have one.
  Output("SD:INIT");
  SD_initialize();
  if (!SD_exists) {
    Output("!!!HALTED!!!");
    while (true) {
      delay(1000);
    }
  }
  
  SD_ReadConfigFile();

  // Set Daily Reboot Timer
  DailyRebootCountDownTimer = cf_daily_reboot * 3600;
  
  rtc_initialize();

  gps_initialize(true);  // true = print NotFound message

  EEPROM_initialize();

  obs_interval_initialize();

  // Optipolar Hall Effect Sensor SS451A - Rain1 Gauge
  if (cf_rg1_enable) {
    raingauge1_interrupt_count = 0;
    raingauge1_interrupt_stime = millis();
    raingauge1_interrupt_ltime = 0;  // used to debounce the tip
    attachInterrupt(RAINGAUGE1_IRQ_PIN, raingauge1_interrupt_handler, FALLING);
    Output ("RG1:ENABLED");
  }
  else {
    Output ("RG1:NOT ENABLED");
  }

  // Optipolar Hall Effect Sensor SS451A - Rain2 Gauge
  if (cf_rg2_enable) {
    raingauge2_interrupt_count = 0;
    raingauge2_interrupt_stime = millis();
    raingauge2_interrupt_ltime = 0;  // used to debounce the tip
    attachInterrupt(RAINGAUGE2_IRQ_PIN, raingauge2_interrupt_handler, FALLING);
    Output ("RG2:ENABLED");
  }
  else {
    Output ("RG2:NOT ENABLED");
  }
  
  DS_Initialize(); //Distance Sensor

  // I2C Sensor Init
  as5600_initialize();
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

  WiFi.setPins(8,7,4,2);
  if (WiFi_Feather()) {
    Output("Feather:WiFi");
    WiFi_initialize();
  }
  else {
    // LoRaWAN Init
    Output("Feather:LoRaWAN");
    delay (10000);
    LW_initialize();
    if (!LW_valid) {
      Output("LW Disabled");
    }
  }

  if (AS5600_exists) {
    Output ("WS:Enabled");
    // Optipolar Hall Effect Sensor SS451A - Wind Speed
    anemometer_interrupt_count = 0;
    anemometer_interrupt_stime = millis();
    attachInterrupt(ANEMOMETER_IRQ_PIN, anemometer_interrupt_handler, FALLING);
  }
  
  Output ("Start Main Loop");
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
  // Output ("LOOP");

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
      Output("SET RTC w/GMT, ENTER:");
      Output("YYYY:MM:DD:HH:MM:SS");
      first = false;
    }

    // Now two things can happen. User enters valid time or we get time from GPS
    rtc_readserial(); // check for serial input, validate for rtc, set rtc, report result
    gps_initialize(false);  // This can set RTC_valid to true when valid GPS time obtained 
                            // false = do not print NotFound message

    if (RTC_valid) {
      Output("!!!!!!!!!!!!!!!!!!!");
      Output("!!! Press Reset !!!");
      Output("!!!!!!!!!!!!!!!!!!!"); 
      while (true) {
        delay (1000);
      }     
    }
  }
    //Calibration mode
  else if (countdown && digitalRead(SCE_PIN) == LOW) { 
    // Every minute, Do observation (don't save to SD) and transmit
    //I2C_Check_Sensors();
    
    StationMonitor();
    
    // check for input sting, validate for rtc, set rtc, report result
    if (Serial.available() > 0) {
      rtc_readserial(); // check for serial input, validate for rtc, set rtc, report result
    }
    
    countdown--;
    delay (1000);
  }

  // Normal Operation
  else {
    // Send GPS info at boot before 1st OBS
    if (gps_need2pub && gps_valid) {
      gps_publish();
      Time_of_next_obs += 30000; //delay observation by 30s, to provide time to get response from lora modem
    }
    
    // Perform an Observation, Write to SD, Send OBS
    if (millis() >= Time_of_next_obs) {
      Output ("Do OBS");
      
      now = rtc.now();
      Time_of_obs = now.unixtime();
      if ((now.year() >= TM_VALID_YEAR_START) && (now.year() <= TM_VALID_YEAR_END)) {
        OBS_Do();
      }
      else {
        Output ("OBS_Do() NotRun-Bad TM");
      }

      Time_of_next_obs = time_to_next_obs();   
      JPO_ClearBits(); // Clear status bits from boot after we log our first observations
    }
  }

  // Reboot Boot Countdown, only if cf_daily_reboot is set
  if ((cf_daily_reboot>0) && (--DailyRebootCountDownTimer<=0)) {
    Output ("Daily Reboot/OBS");
    
    Time_of_obs = rtc_unixtime();
    OBS_Do();
    
    Output("Rebooting");  
    delay(1000);
   
    DeviceReset();

    // We should never get here, but just incase 
    Output("I'm Alive! Why?");
    DailyRebootCountDownTimer = cf_daily_reboot * 3600;
  }
  
  BackGroundWork();
}
