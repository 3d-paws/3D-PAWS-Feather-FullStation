/*
 * ======================================================================================================================
 *  info_wifi.cpp - Station Information WiFi Functions
 * ======================================================================================================================
 */
#include <Arduino.h>
#include <SdFat.h>
#include <RTClib.h>

#include "include/ssbits.h"
#include "include/eeprom.h"
#include "include/nvcf.h"
#include "include/mux.h"
#include "include/dsmux.h"
#include "include/sensor_bmp.h"
#include "include/sensor_sht.h"
#include "include/sensor_mcp.h"
#include "include/sensor_htu.h"
#include "include/sensor_pm25aqi.h"
#include "include/sensor_tlw.h"
#include "include/sensor_tsm.h"
#include "include/sensor_rgop.h"
#include "include/sensor_wind.h"
#include "include/sensor_bh1750.h"
#include "include/sensor_ltr390.h"
#include "include/derived.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/lorawan.h"
#include "include/support.h"
#include "include/time.h"
#include "include/wifi.h"
#include "include/gps.h"
#include "include/feather.h"
#include "include/notecard.h"
#include "include/main.h"
#include "include/info_wifi.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures 
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * INFO_Do_Wifi() - Get and Send System Information
 *   Build one complete info message to be save in INFO.TXT
 *   If WiFi send complete info message - this will use fullmsg
 *   fullmsg will be JSON
 * =======================================================================================================================
 */
void INFO_Do_WiFi() {
  char msg[1024];   // Holds JSON information 
  const char *comma = "";
  float vbat;

  memset(msg, 0, 1024);

  rtc_timestamp();
  
  vbat = vbat_get();

  sprintf (msg, "{\"MT\":\"INFO\""); // Message Type -> INFO

  sprintf (msg+strlen(msg), ",\"at\":\"%s\",\"devid\":\"%s\",\"board\":\"%s\"", // Adafruit Feather M0 WiFi
    timestamp, DeviceID, board_config_tag[board_config]);

  sprintf (msg+strlen(msg), ",\"ver\":\"%s\",\"lwr\":\"%s\",\"bv\":%.2f,\"hth\":%d,\"elev\":%d,\"rtro\":\"%d:%d\"",
    versioninfo, 
    LW_Region(), // LW Region: EU868 US915 ...
                 // Even though we are running on a WiFi board. Good to know what Region we are compiled for.
    vbat, SystemStatusBits, scv.elevation, scv.rtro_hour, scv.rtro_minute);

  // Log Server Information and Chords Apikey and Id
  sprintf (msg+strlen(msg), ",\"ls\":\"%s\",\"lsp\":%d,\"lsurl\":\"%s\",\"lsapi\":\"%s\",\"lsid\":%d",
    scv.ls_fqdn.c_str(), scv.ls_port, scv.ls_urlpath.c_str(), scv.ls_apikey.c_str(), scv.ls_instr_id);

  sprintf (msg+strlen(msg), ",\"ntp\":\"%s\",\"ssid\":\"%s\"",
    scv.ts_fqdn.c_str(), scv.wifi_ssid.c_str());

  // obs_period (1,5,6,10,15,20,30), 1 minute observation period is the default
  // daily_reboot Number of hours between daily reboots, A value of 0 disables this feature
  sprintf (msg+strlen(msg), ",\"obsi\":\"%dm\",\"t2nt\":\"%ds\",\"drbt\":\"%dm\"",
    scv.obsi, (int)((millis()-time_to_next_obs())/1000), scv.daily_reboot);

  SD_NeedToSend_Status(Buffer32Bytes);
  sprintf (msg+strlen(msg), ",\"n2s\":%s", Buffer32Bytes);

  // Discovered Device List
  comma="";
  sprintf (msg+strlen(msg), ",\"devs\":\"");
  if (RTC_exists) {
    sprintf (msg+strlen(msg), "%srtc", comma);
    comma=",";
  }
  if (SD_exists) {
    sprintf (msg+strlen(msg), "%ssd", comma);
    comma=",";
  }
  if (eeprom_exists) {
    sprintf (msg+strlen(msg), "%seeprom", comma);
    comma=",";    
  }
  if (MUX_exists) {
    sprintf (msg+strlen(msg), "%smux", comma);
    comma=",";    
  }
  if (DSMUX_exists) {
    sprintf (msg+strlen(msg), "%sdsmux", comma);
    comma=",";    
  }
  if (oled_type) {
    sprintf (msg+strlen(msg), ",oled(%s)", OLED32 ? "32" : "64");  
  }
  if (gps_exists) {
    sprintf (msg+strlen(msg), "%sgps(", comma);
    if (gps_valid) {
      sprintf (msg+strlen(msg), "lat:%f,lon:%f,alt:%f,sat:%d", 
        comma, gps_lat, gps_lon, gps_altm, gps_sat);
    }
    sprintf (msg+strlen(msg), ")");
    comma=","; 
  }
  sprintf (msg+strlen(msg), "\""); 

  mux_info(msg, 1024, comma); // MUX SENSORS - tsm
  dsmux_info(msg, 1024, comma); // DSMUX Temperature Sensors

  // Lat and Lon config settings
  if ((scv.lat_deg != 0.0) && (scv.lon_deg != 0.0)) {
    sprintf (msg+strlen(msg), ",\"lat\":%f,\"lon\":%f", scv.lat_deg, scv.lon_deg);   
  }

  // WiFi Info
  IPAddress ip = WiFi.localIP();
  long rssi = WiFi.RSSI();

  sprintf (msg+strlen(msg), ",\"wifi\":{");
  sprintf (msg+strlen(msg), "\"fw\":\"%s\",\"ssid\":\"%s\",\"auth\":\"%s\"",
    WiFi.firmwareVersion(), scv.wifi_ssid.c_str(), WiFiAuthType);
  sprintf (msg+strlen(msg), ",\"ip\":\"%d.%d.%d.%d\",\"rssi\":%ld}",
    ip[0], ip[1], ip[2], ip[3],rssi);

  // SENSORS
  comma="";
  sprintf (msg+strlen(msg), ",\"sensors\":\"");
  bmp_info(msg, 1024, comma);
  sht_info(msg, 1024, comma);
  mcp_info(msg, 1024, comma);
  htu_info(msg, 1024, comma);
  pm25aqi_info(msg, 1024, comma);
  tlw_info(msg, 1024, comma);
  tsm_info(msg, 1024, comma);
  Wind_info(msg, 1024, comma);
  bh1750_info(msg, 1024, comma);
  ltr390_info(msg, 1024, comma);

  if (scv.rg1) {
    sprintf (msg+strlen(msg), "%sRG1(%s)", comma, pinNames[RAINGAUGE1_IRQ_PIN]); 
    comma=",";
  } 
  if (nv_op1_state() == OP1_STATE_RAW) {
    sprintf (msg+strlen(msg), "%sOP1R(%s)", comma, pinNames[OP1_PIN]);
    comma=",";
  } 
  if (nv_op1_state() == OP1_STATE_RAIN) {
    sprintf (msg+strlen(msg), "%sRG2(%s)", comma, pinNames[RAINGAUGE2_IRQ_PIN]);
    comma=",";
  } 
  if (nv_op1_state() == OP1_STATE_DIST_5M) {
    sprintf (msg+strlen(msg), "%s5MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], scv.ds_baseline);
    comma=",";
  } 
  if (nv_op1_state() == OP1_STATE_DIST_10M) {
    sprintf (msg+strlen(msg), "%s10MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], scv.ds_baseline);
    comma=",";
  }
  if (nv_op2_state() == OP2_STATE_RAW) {
    sprintf (msg+strlen(msg), "%sOP2R(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  }
  if (nv_op2_state() == OP2_STATE_VOLTAIC) {
    sprintf (msg+strlen(msg), "%sVBV(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  } 
   // Close off sensors
  sprintf (msg+strlen(msg), "\"");

  // Derived Observatins
  derived_info(msg, 1024, comma);

  // Adding closing }
  sprintf (msg+strlen(msg), "}");

  Serial_writeln(msg); 

  if (WiFi_Send(msg, scv.is_fqdn.c_str(), scv.is_port, scv.is_urlpath.c_str(), METHOD_POST, scv.is_apikey.c_str())) {
    Output("INFO->PUB WiFi OK");
  }
  else {
    Output("INFO->PUB WiFi FAILED");

    // Save to the N2S File
    SD_NeedToSend_Add(msg);
  }

  // Update INFO.TXT file
  SD_UpdateInfoFile(msg);
}