/*
 * ======================================================================================================================
 *  cf.cpp - Configuration File Functions
 * ======================================================================================================================
 */
#include <SdFat.h>

#include "include/ssbits.h"
#include "include/qc.h"
#include "include/feather.h"
#include "include/support.h"
#include "include/output.h"
#include "include/sdcard.h"
#include "include/lorawan.h"
#include "include/wrda.h"
#include "include/main.h"
#include "include/cf.h"


/*
 * ======================================================================================================================
 *  Define Global Configuration File Variables
 * ======================================================================================================================
 */
int cf_lw_mode=0; // 0=OTAA, 1=ABP
int cf_lw_sf=7;   // Spreading factor (7,8)
int cf_lw_txpw=14; // Transmit Power
// OTAA
char *cf_lw_joineui = NULL;
char *cf_lw_deveui = NULL;
char *cf_lw_appkey = NULL;
// ABP
char *cf_lw_devaddr = NULL;
char *cf_lw_nwkskey = NULL;
char *cf_lw_appskey = NULL;
// WiFi
char *cf_wifi_ssid   = NULL;
char *cf_wifi_pw     = NULL;
char *cf_wifi_wepkey = NULL;
// Web Server
char *cf_webserver     = NULL;
int  cf_webserver_port = 80;
char *cf_urlpath       = NULL;
char *cf_apikey        = NULL;
int  cf_instrument_id  = 0;
// Info Server
char *cf_info_server     = NULL;
int  cf_info_server_port = 443;
char *cf_info_urlpath    = NULL;
char *cf_info_apikey     = NULL;
// Time Server
char *cf_ntpserver = NULL;

int cf_rg1_enable=0;
int cf_op1=0;
int cf_op2=0;
int cf_ds_baseline=0;
int cf_obs_period=0;
int cf_daily_reboot=0;
int cf_elevation=0;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

 /* 
 * =======================================================================================================================
 * Support functions for Config file
 * 
 *  https://arduinogetstarted.com/tutorials/arduino-read-config-from-sd-card
 *  
 *  myInt_1    = SD_findInt(F("myInt_1"));
 *  myFloat_1  = SD_findFloat(F("myFloat_1"));
 *  myString_1 = SD_findString(F("myString_1"));
 *  
 *  CONFIG.TXT content example
 *  myString_1=Hello
 *  myInt_1=2
 *  myFloat_1=0.74
 * =======================================================================================================================
 */

int SD_findKey(const __FlashStringHelper * key, char * value) {
  
  // Disable LoRA / WiFi SPI0 Chip Select
  pinMode(LORA_WIFI_SS, OUTPUT);
  digitalWrite(LORA_WIFI_SS, HIGH);
  
  // At this point we do not need to deal with LoRa SPI collisions. LoRa Not initialized yet.
  File configFile = SD.open(CF_NAME);

  if (!configFile) {
    Serial.print(F("SD Card: error on opening file "));
    Serial.println(CF_NAME);
    return(0);
  }

  char key_string[KEY_MAX_LENGTH];
  char SD_buffer[KEY_MAX_LENGTH + VALUE_MAX_LENGTH + 1]; // 1 is = character
  int key_length = 0;
  int value_length = 0;

  // Flash string to string
  PGM_P keyPoiter;
  keyPoiter = reinterpret_cast<PGM_P>(key);
  byte ch;
  do {
    ch = pgm_read_byte(keyPoiter++);
    if (ch != 0)
      key_string[key_length++] = ch;
  } while (ch != 0);

  // check line by line
  while (configFile.available()) {
    int buffer_length = configFile.readBytesUntil('\n', SD_buffer, LINE_MAX_LENGTH);
    if (SD_buffer[buffer_length - 1] == '\r')
      buffer_length--; // trim the \r

    if (buffer_length > (key_length + 1)) { // 1 is = character
      if (memcmp(SD_buffer, key_string, key_length) == 0) { // equal
        if (SD_buffer[key_length] == '=') {
          value_length = buffer_length - key_length - 1;
          memcpy(value, SD_buffer + key_length + 1, value_length);
          break;
        }
      }
    }
  }

  configFile.close();  // close the file
  return value_length;
}

int HELPER_ascii2Int(char *ascii, int length) {
  int sign = 1;
  int number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

float HELPER_ascii2Float(char *ascii, int length) {
  int sign = 1;
  int decimalPlace = 0;
  float number  = 0;
  float decimal = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c == '.')
        decimalPlace = 1;
      else if (c >= '0' && c <= '9') {
        if (!decimalPlace)
          number = number * 10 + (c - '0');
        else {
          decimal += ((float)(c - '0') / pow(10.0, decimalPlace));
          decimalPlace++;
        }
      }
    }
  }

  return (number + decimal) * sign;
}

String HELPER_ascii2String(char *ascii, int length) {
  String str;
  str.reserve(length);
  str = "";

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str += String(c);
  }
  return str;
}

char* HELPER_ascii2CharStr(char *ascii, int length) {
  char *str;
  int i = 0;
  str = (char *) malloc (length+1);
  str[0] = 0;
  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str[i] = c;
    str[i+1] = 0;
  }
  return str;
}

bool SD_available(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return value_length > 0;
}

int SD_findInt(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Int(value_string, value_length);
}

float SD_findFloat(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Float(value_string, value_length);
}

String SD_findString(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2String(value_string, value_length);
}

char* SD_findCharStr(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2CharStr(value_string, value_length);
}


/* 
 * =======================================================================================================================
 * SD_ReadConfigFile()
 * =======================================================================================================================
 */
void SD_ReadConfigFile() {
  // LoRa
  cf_lw_mode      = SD_findInt(F("lw_mode"));
  sprintf(msgbuf, "CF:%s=[%d]", F("CF:lw_mode"), cf_lw_mode);   Output (msgbuf);

  // Spreading factor
  cf_lw_sf        = SD_findInt(F("lw_sf"));
  if ((cf_lw_sf<7) || (cf_lw_sf>8)) cf_lw_sf = 7;
  sprintf(msgbuf, "CF:%s=[%d]", F("CF:lw_sf"),   cf_lw_sf);   Output (msgbuf);

  // Transmit Power
  cf_lw_txpw = SD_findInt(F("lw_txpw"));
  if ((cf_lw_txpw<14) || (cf_lw_txpw>20)) cf_lw_txpw = 14;
  sprintf(msgbuf, "CF:%s=[%d]", F("CF:lw_txpw"),   cf_lw_txpw);   Output (msgbuf);

  // LoRa OTAA
  cf_lw_joineui    = SD_findCharStr(F("lw_joineui"));
  sprintf(msgbuf, "%s=[%s]", F("CF:lw_joineui"), cf_lw_joineui); Output (msgbuf);

  cf_lw_deveui    = SD_findCharStr(F("lw_deveui"));
  sprintf(msgbuf, "%s=[%s]", F("CF:lw_deveui"), cf_lw_deveui); Output (msgbuf);

  cf_lw_appkey    = SD_findCharStr(F("lw_appkey"));
  sprintf(msgbuf, "%s=[%s]", F("CF:lw_appkey"), cf_lw_appkey); Output (msgbuf);

  // LoRa ABP
  cf_lw_devaddr   = SD_findCharStr(F("lw_devaddr"));
  sprintf(msgbuf, "%s=[%s]", F("CF:lw_devaddr"), cf_lw_devaddr); Output (msgbuf);

  cf_lw_nwkskey   = SD_findCharStr(F("lw_nwkskey"));
  sprintf(msgbuf, "%s=[%s]", F("CF:lw_nwkskey"), cf_lw_nwkskey); Output (msgbuf);

  cf_lw_appskey   = SD_findCharStr(F("lw_appskey"));
  sprintf(msgbuf, "%s=[%s]", F("CF:lw_appskey"), cf_lw_appskey); Output (msgbuf);
  
  // WiFi
  cf_wifi_ssid    = SD_findCharStr(F("wifi_ssid"));
  sprintf(msgbuf, "%s=[%s]", F("CF:wifi_ssid"), cf_wifi_ssid); Output (msgbuf);

  cf_wifi_pw      = SD_findCharStr(F("wifi_pw"));
  sprintf(msgbuf, "%s=[%s]", F("CF:wifi_pw"), cf_wifi_pw); Output (msgbuf);

  cf_wifi_wepkey  = SD_findCharStr(F("wifi_wepkey"));
  sprintf(msgbuf, "%s=[%s]", F("CF:wifi_wepkey"), cf_wifi_wepkey); Output (msgbuf);

  // Web Server
  cf_webserver      = SD_findCharStr(F("webserver"));
  sprintf(msgbuf, "%s=[%s]", F("CF:webserver"), cf_webserver); Output (msgbuf);
  
  cf_webserver_port = SD_findInt(F("webserver_port"));
  sprintf(msgbuf, "%s=[%d]", F("CF:webserver_port"), cf_webserver_port); Output (msgbuf);

  cf_urlpath        = SD_findCharStr(F("urlpath"));
  sprintf(msgbuf, "%s=[%s]", F("CF:urlpath"), cf_urlpath); Output (msgbuf);
  
  cf_apikey         = SD_findCharStr(F("apikey"));
  sprintf(msgbuf, "%s=[%s]", F("CF:apikey"), cf_apikey); Output (msgbuf);
  
  cf_instrument_id  = SD_findInt(F("instrument_id"));
  sprintf(msgbuf, "%s=[%d]", F("CF:instrument_id"), cf_instrument_id); Output (msgbuf);

  // Info Server
  cf_info_server  = SD_findCharStr(F("info_server"));
  sprintf(msgbuf, "%s=[%s]", F("CF:info_server"), cf_info_server); Output (msgbuf);

  cf_info_server_port = SD_findInt(F("info_server_port"));
  sprintf(msgbuf, "%s=[%d]", F("CF:info_server_port"), cf_info_server_port); Output (msgbuf);

  cf_info_urlpath = SD_findCharStr(F("info_urlpath"));
  sprintf(msgbuf, "%s=[%s]", F("CF:info_urlpath"), cf_info_urlpath); Output (msgbuf);

  cf_info_apikey  = SD_findCharStr(F("info_apikey"));
  sprintf(msgbuf, "%s=[%s]", F("CF:info_apikey"), cf_info_apikey); Output (msgbuf);

  //Time Server
  cf_ntpserver    = SD_findCharStr(F("ntpserver"));
  sprintf(msgbuf, "%s=[%s]", F("CF:ntpserver"), cf_ntpserver); Output (msgbuf);

  // Option Pin 1 A4
  cf_rg1_enable   = SD_findInt(F("rg1_enable"));
  sprintf(msgbuf, "%s=[%d]", F("CF:rg1_enable"), cf_rg1_enable); Output (msgbuf);

  cf_op1   = SD_findInt(F("op1"));
  sprintf(msgbuf, "%s=[%d]", F("CF:op1"), cf_op1); Output (msgbuf);
  if ((cf_op1 != 0) && (cf_op1 != 1) && (cf_op1 != 2) && (cf_op1 != 5) && (cf_op1 != 10)) {
    cf_op1 = 0;
    Output (" OP1 Invalid");
  }
  else if (cf_op1 == 5) {
    dg_resolution_adjust = 5;
    Output (" DIST5M Set");
  }
  else if (cf_op1 == 10) {
    dg_resolution_adjust = 10;
    Output (" DIST10M Set");
  }
  
  // Option Pin 2 A5
  cf_op2    = SD_findInt(F("op2"));
  sprintf(msgbuf, "%s=[%d]", F("CF:op2"), cf_op2); Output (msgbuf);
  if ((cf_op2 != 0) && (cf_op2 != 1) && (cf_op2 != 2)) {
    cf_op2 = 0;
    Output (" OP2 Invalid");
  }

  cf_ds_baseline = SD_findInt(F("ds_baseline"));
  sprintf(msgbuf, "%s=[%d]", F("CF:ds_baseline"), cf_ds_baseline); Output (msgbuf);

  cf_obs_period   = SD_findInt(F("obs_period"));
  if (cf_obs_period == 0) {
    cf_obs_period = 1;
  }
  sprintf(msgbuf, "%s:obs_period=[%d]", F("CF:obs_period"), cf_obs_period); Output (msgbuf);
  
  // Misc
  cf_daily_reboot = SD_findInt(F("daily_reboot"));
  sprintf(msgbuf, "%s=[%d]", F("CF:daily_reboot"), cf_daily_reboot); Output (msgbuf);

  cf_elevation = SD_findInt(F("elevation"));
  sprintf(msgbuf, "%s=[%d]", F("CF:elevation"), cf_elevation); Output (msgbuf);
}
