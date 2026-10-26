/*
 * ======================================================================================================================
 *  info.lorawan - Station Information Functions
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
#include "include/info_lorawan.h"
#include "include/info.h"

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
 * INFO_Send_LoRaWAN() - Handle sending the LoRaWAN INFO messages
 * =======================================================================================================================
 */
void INFO_Send_LoRaWAN(char *msg) {

  // Safty check
  size_t len = strnlen(msg, 222);
  if (len == 0 || len > 222) {
    Output("INFO->PUB InvalLen");
  }

  else if (!LW_valid) {
    Output("INFO->PUB NOLW");
  }

  else if (LW_isJoined()) {
    if (LW_Send(msg)) {
      Output("INFO->PUB LW OK");
    }
    else {
      Output("INFO->PUB LW FAILED");

      // Save to the N2S File
      SD_NeedToSend_Add(msg);
    }
  }

  else {
    Output("INFO->PUB NOT JOINED");

    // Save to the N2S file.
    SD_NeedToSend_Add(msg);
  }
}

/*
 * ======================================================================================================================
 * INFO_Do_LoRaWAN() - Get and Send System Information
 *   Build one complete info message to be save in INFO.TXT - fullmsg - JSON format
 *   If LoRaWAN send multiple 222 byte messages so multiple JSON formatted messages
 *   The webhook at TTN will need to identify this is a info message direct accordingly
 *     It will add LoRaWAN Device ID and timestamp then forward to INFO message server
 *     We will only send the devid on the first packet. TTN will need to use a deveui to devid map table for rest of packets
 * =======================================================================================================================
 */
void INFO_Do_LoRaWAN() {
  char header[32];
  char loramsg[256];
  char fullmsg[1024];   // Holds JSON observations to write to INFO.TXT
  char msg[256];
  const char *comma = "";
  float vbat;

  memset(header,  0, sizeof(header));  // Will be send with each LoRa Info message
  memset(fullmsg, 0, sizeof(fullmsg)); // For the log file JSON format
  memset(msg,     0, sizeof(msg));

  rtc_timestamp();
  
  vbat = vbat_get();

  sprintf (fullmsg, "{\"MT\":\"INFO\""); // Message Type -> INFO
  sprintf (header, "{\"MT\":\"INFO\"");  // 12 bytes

  // Header                                            =  12 bytes
  // ,"at":"","devid":"","board":"AFM0LoRa","ver":"","bv":,"hth":,"elev": =  68 bytes
  // at = 2026-01-21T19:03:57                          =  19 bytes
  // devid = 330eff6367815b7d93bfbcec                  =  24 bytes
  // ver = FSLW-260118                                 =  11 bytes
  // bv = 4.27                                         =   4 bytes
  // hth = 646721                                      =   6 bytes
  // elev = 8849                                       =   4 bytes
  // ,"obsi":"%dm","t2nt":"s","drbt":"m"               =  31 bytes
  // obsi = 30                                         =   2 bytes
  // tn2t = 1800                                       =   4 bytes
  // drbt = 22                                         =   2 bytes
  // ,"n2s:" +6                                        =  13 bytes
  // }                                                 =   1 byte
  //                                               sum = 201 bytes

  // Example below is 193 characters
  // {"MT":"INFO","at":"2026-10-26T17:30:04","devid":"548FA41EF43EE791","board":"LW","ver":"FFS-261126","bv":4.27,"hth":1,"elev":0,"rtro":"0:0","obsi":"1m","t2nt":"4294907s","drbt":"22m","n2s":"NF"}

  sprintf (msg, "\"at\":\"%s\",\"devid\":\"%s\",\"board\":\"%s\",\"ver\":\"%s\",\"bv\":%.2f,\"hth\":%d,\"elev\":%d,\"rtro\":\"%d:%d\"",
    timestamp, DeviceID,board_config_tag[board_config], versioninfo, 
    vbat, SystemStatusBits, scv.elevation, scv.rtro_hour, scv.rtro_minute);
  
  // obs_period (1,5,6,10,15,20,30), 1 minute observation period is the default
  // daily_reboot Number of hours between daily reboots, A value of 0 disables this feature
  sprintf (msg+strlen(msg), ",\"obsi\":\"%dm\",\"t2nt\":\"%ds\",\"drbt\":\"%dm\"",
    scv.obsi, (int)((millis()-time_to_next_obs())/1000), scv.daily_reboot);

  // Update INFO.TXT file
  SD_NeedToSend_Status(Buffer32Bytes);
  sprintf (msg+strlen(msg), ",\"n2s\":%s", Buffer32Bytes);

  // Tally it all up
  sprintf (fullmsg+strlen(fullmsg), ",%s", msg);
  sprintf (loramsg, "%s,%s}", header, msg);

  Serial_writeln(loramsg);

  LW_WaitOnTXRXPEND(INFO_LW_WAIT_BUF_CLR); // Make sure we can buffer a new message
  INFO_Send_LoRaWAN(loramsg);
  LW_WaitOnTXRXPEND(INFO_LW_WAIT_BUF_CLR); // Make sure we can buffer a new message

  //LW_WaitBetween(INFO_LW_WAIT_BTWN_TM);

  // Start Next LoRa Message

  // Header                                            =  12 bytes
  // ,"LW":",,," =  11+5+4+1+2                         =  23 bytes
  // ,"devs":"eeprom,mux,sd,gps(#)"                    =  30 bytes
  // ,"lat":,"lon":,"alt":,"sat":"  = 29+10+11+11+2    =  63 bytes
  // ,"mux":"sensor list"                              =   9
  // }                                                 =   1 byte
  //                                               sum = 138 bytes
  //  We can have 8 of the below on the mux for a total of 218 bytes
  //   ,TSM1(1.1) 6 + 2 + 1 + 1                        =  10  ()

  // Example below is 157 characters
  // {"MT":"INFO","LW":"EU868,OTAA,7,14","devs":"rtc,sd,eeprom,gps(lat:40.042145,lon:40.042165,alt:-105.064532,sat:-858993459)","lat":40.042385,"lon":-105.041656}

  // LoRa Configs and Discovered Device List

  // lwr  - LW Region: EU868 US915 ...
  // lwm  - LW Mode: OTAA, ABP, ERR
  // lwsf - LW Spreading Factor 7 or 8
  // lstp - LW Transmit Power 14 to 20
  sprintf (msg, "\"LW\":\"%s,",LW_Region());
  if ((scv.lw_mode == "OTAA") || (scv.lw_mode == "ABP")) {
    sprintf (msg+strlen(msg), "%s,", scv.lw_mode.c_str());
  }
  else {
    sprintf (msg+strlen(msg), "ERR,");
  }
  sprintf (msg+strlen(msg), "%d,%d\"", scv.lw_sf, scv.lw_txpwr);

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
    sprintf (msg+strlen(msg), "%s\"lat\":%f,\"lon\":%f", comma, scv.lat_deg, scv.lon_deg);   
  }

  // Tally it all up
  sprintf (fullmsg+strlen(fullmsg), ",%s", msg);
  sprintf (loramsg, "%s,%s}", header, msg);

  Serial_writeln(loramsg);

  LW_WaitOnTXRXPEND(INFO_LW_WAIT_BUF_CLR); // Make sure we can buffer a new message
  INFO_Send_LoRaWAN(loramsg);
  LW_WaitOnTXRXPEND(INFO_LW_WAIT_BUF_CLR); // Make sure we can buffer a new message

  // LW_WaitBetween(INFO_LW_WAIT_BTWN_TM);

  // Start Next LoRa Message

  // Header                                            =  12 bytes
  // "sensors":" ... "                                 =  12 bytes
  // ,BMP1(6)                                          =  13 bytes
  // ,BMP2(6)                                          =  13 bytes
  // ,MCP1                                             =   4 bytes
  // ,MCP2                                             =   4 bytes
  // ,MCP3/gt1                                         =   9 bytes
  // ,MCP4/gt2                                         =   9 bytes
  // ,SHT1                                             =   5 bytes
  // ,SHT2                                             =   5 bytes
  // ,AS5600,WS(A2)                                    =  14 bytes
  // ,TLW                                              =   4 bytes
  // ,TSM                                              =   4 bytes
  // ,HI                                               =   3 bytes
  // ,WBT                                              =   4 bytes
  // ,WBGT WO/G                                        =  10 bytes
  // ,PM25AQ                                           =   7 bytes
  // ,RG1(A3)                                          =   7 bytes
  // ,RG2(A4)                                          =   7 bytes
  // ,DIST 10M(A5),8849                                =  17 bytes
  //                                               sum =     bytes
  
  // Example below is 120 characters
  // {"MT":"INFO","sensors":"BMP581,SHT45,SHT31(118DF311),MCP1,!WIND,RG1(A3),5MDIST(A4,0)","derived":"HI,WBT,WBGT WO/G,MSLP"}

  // SENSORS
  comma="";
  sprintf (msg, "\"sensors\":\"");
  bmp_info(msg, 1024, comma);
  sht_info(msg, 1024, comma);
  mcp_info(msg, 1024, comma);
  htu_info(msg, 1024, comma);
  pm25aqi_info(msg, 1024, comma);
  tlw_info(msg, 1024, comma);
  tlw_info(msg, 1024, comma);
  tsm_info(msg, 1024, comma);
  bh1750_info(msg, 1024, comma);
  ltr390_info(msg, 1024, comma);

  if (scv.wind) {
    sprintf (msg+strlen(msg), "%sWIND", comma);
    comma=",";
    sprintf (msg+strlen(msg), "%sWS(%s)", comma, pinNames[ANEMOMETER_IRQ_PIN]);

    if (AS5600_exists) {
      sprintf (msg+strlen(msg), "%sAS5600", comma);
    }
    else {
      sprintf (msg+strlen(msg), "%s!AS5600", comma);
    }
  }
  else {
    sprintf (msg+strlen(msg), "%s!WIND", comma);
    comma=",";
  }

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

  // Tally it all up
  sprintf (fullmsg+strlen(fullmsg), ",%s}", msg);
  sprintf (loramsg, "%s,%s}", header, msg);

  Serial_writeln(loramsg);

  LW_WaitOnTXRXPEND(INFO_LW_WAIT_BUF_CLR); // Make sure we can buffer a new message
  INFO_Send_LoRaWAN(loramsg);
  LW_WaitOnTXRXPEND(INFO_LW_WAIT_BUF_CLR); // Make sure we can buffer a new message

  // LW_WaitBetween(INFO_LW_WAIT_BTWN_TM); //
  // LW_WaitOnTXRXPEND(INFO_LW_WAIT_BUF_CLR); // Lets wait for the buffer to clear
  
  // Update INFO.TXT file
  Serial_writeln(fullmsg);
  SD_UpdateInfoFile(fullmsg);
}