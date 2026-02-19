/*
 * ======================================================================================================================
 *  info.cpp - Station Information Functions
 * ======================================================================================================================
 */
#include <Arduino.h>
#include <SdFat.h>
#include <RTClib.h>

#include "include/ssbits.h"
#include "include/mux.h"
#include "include/eeprom.h"
#include "include/cf.h"
#include "include/sensors.h"
#include "include/wrda.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/lorawan.h"
#include "include/support.h"
#include "include/time.h"
#include "include/wifi.h"
#include "include/gps.h"
#include "include/feather.h"
#include "include/obs.h"
#include "include/main.h"
#include "include/info.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures 
 * =======================================================================================================================
 */
bool info_server_valid=false;       // If config file has valid Info Server configuration (WiFi) then this is true

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

 /*
 * ======================================================================================================================
 * INFO_Initialize() - Test if we have set the Info Server in the config file
 * =======================================================================================================================
 */
void INFO_Initialize() {
  if (cf_info_server  && cf_info_server[0]  != '\0' &&
      cf_info_urlpath && cf_info_urlpath[0] != '\0' &&
      cf_info_apikey  && cf_info_apikey[0]  != '\0' &&
      cf_info_server_port != 0) {

    info_server_valid = true;
    Output (F("INFO_Server:OK"));
  }
  else {
    info_server_valid = false;
    Output (F("INFO_Server:NOT SET"));
  }
}

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

  // AFM0WiFi = Adafruit Feather M0 WiFi
  sprintf (msg+strlen(msg), ",\"at\":\"%s\",\"devid\":\"%s\",\"board\":\"AFM0WiFi\"", // Adafruit Feather M0 WiFi
    timestamp, DeviceID);

  sprintf (msg+strlen(msg), ",\"ver\":\"%s\",\"bv\":%d.%02d,\"hth\":%d,\"elev\":%d",
    versioninfo, (int)vbat, (int)(vbat*100)%100, SystemStatusBits, cf_elevation);

  // Log Server Information and Chords Apikey and Id
  sprintf (msg+strlen(msg), ",\"ls\":\"%s\",\"lsp\":%d,\"lsurl\":\"%s\",\"lsapi\":\"%s\",\"lsid\":%d",
    cf_webserver, cf_webserver_port, cf_urlpath, cf_apikey, cf_instrument_id);

  sprintf (msg+strlen(msg), ",\"ntp\":\"%s\",\"ssid\":\"%s\"",
    cf_ntpserver, cf_wifi_ssid);

  // obs_period (1,5,6,10,15,20,30), 1 minute observation period is the default
  // daily_reboot Number of hours between daily reboots, A value of 0 disables this feature
  sprintf (msg+strlen(msg), ",\"obsi\":\"%dm\",\"t2nt\":\"%ds\",\"drbt\":\"%dm\"",
    cf_obs_period, (int)((millis()-time_to_next_obs())/1000), cf_daily_reboot);

  SD_NeedToSend_Status(Buffer32Bytes);
  sprintf (msg+strlen(msg), ",\"n2s\":%s", Buffer32Bytes);

  // Discovered Device List
  comma="";
  sprintf (msg+strlen(msg), ",\"devs\":\"");
  if (eeprom_exists) {
    sprintf (msg+strlen(msg), "%seeprom", comma);
    comma=",";    
  }
  if (MUX_exists) {
    sprintf (msg+strlen(msg), "%smux", comma);
    comma=",";    
  }
  if (SD_exists) {
    sprintf (msg+strlen(msg), "%ssd", comma);
    comma=",";
  }
  if (gps_exists) {
    sprintf (msg+strlen(msg), "%sgps", comma);
    comma=",";   
  }
  sprintf (msg+strlen(msg), "\""); 

  // WiFi Info
  IPAddress ip = WiFi.localIP();
  long rssi = WiFi.RSSI();

  sprintf (msg+strlen(msg), ",\"wifi\":{");
  sprintf (msg+strlen(msg), "\"fw\":\"%s\",\"ssid\":\"%s\",\"auth\":\"%s\"",
    WiFi.firmwareVersion(), cf_wifi_ssid, WiFiAuthType);
  sprintf (msg+strlen(msg), ",\"ip\":\"%d.%d.%d.%d\",\"rssi\":%ld}",
    ip[0], ip[1], ip[2], ip[3],rssi);

  // GPS Info
  if (gps_exists && gps_valid) {
    sprintf (msg+strlen(msg), ",\"lat\":%f,\"lon\":%f,\"alt\":%f,\"sat\":%d", 
      gps_lat, gps_lon, gps_altm, gps_sat);
  }

  // MUX SENSORS  
  if (MUX_exists) {
    comma="";
    sprintf (msg+strlen(msg), ",\"mux\":\"");
    for (int c=0; c<MUX_CHANNELS; c++) {
      if (mux[c].inuse) {
        for (int s = 0; s < MAX_CHANNEL_SENSORS; s++) {
          if (mux[c].sensor[s].type == m_tsm) {
            sprintf (msg+strlen(msg), "%sTSM%d(%d.%d)", comma, mux[c].sensor[s].id, c, s);
            comma=",";
          }
        }
      }
    }
    // Close off mux sensors
    sprintf (msg+strlen(msg), "\"");
  }

  // SENSORS
  comma="";
  sprintf (msg+strlen(msg), ",\"sensors\":\"");

  if (BMX_1_exists) {
    sprintf (msg+strlen(msg), "%sBMX1(%s)", comma, bmxtype[BMX_1_type]);
    comma=",";
  }
  if (BMX_2_exists) {
    sprintf (msg+strlen(msg), "%sBMX2(%s)", comma, bmxtype[BMX_2_type]);
    comma=",";
  }
  if (MCP_1_exists) {
    sprintf (msg+strlen(msg), "%sMCP1", comma);
    comma=",";
  }
  if (MCP_2_exists) {
    sprintf (msg+strlen(msg), "%sMCP2", comma);
    comma=",";
  }
  if (MCP_3_exists) {
    sprintf (msg+strlen(msg), "%sMCP3/gt1", comma);
    comma=",";
  }
  if (MCP_4_exists) {
    sprintf (msg+strlen(msg), "%sMCP4/gt2", comma);
    comma=",";
  }
  if (SHT_1_exists) {
    sprintf (msg+strlen(msg), "%sSHT1", comma);
    comma=",";
  }
  if (SHT_2_exists) {
    sprintf (msg+strlen(msg), "%sSHT2", comma);
    comma=",";
  }
  if (HDC_1_exists) {
    sprintf (msg+strlen(msg), "%sHDC1", comma);
    comma=",";
  }
  if (HDC_2_exists) {
    sprintf (msg+strlen(msg), "%sHDC2", comma);
    comma=",";
  }
  if (LPS_1_exists) {
    sprintf (msg+strlen(msg), "%sLPS1", comma);
    comma=",";
  }
  if (LPS_2_exists) {
    sprintf (msg+strlen(msg), "%sLPS2", comma);
    comma=",";
  }
  if (HIH8_exists) {
    sprintf (msg+strlen(msg), "%sHIH8", comma);
    comma=",";
  }
  if (SI1145_exists) {
    sprintf (msg+strlen(msg), "%sSI", comma);
    comma=",";
  }
  if (VEML7700_exists) {
    sprintf (msg+strlen(msg), "%sVEML", comma);
    comma=",";
  }
  if (BLX_exists) {
    sprintf (msg+strlen(msg), "%sBLX", comma);
    comma=",";
  }
  if (AS5600_exists) {
    sprintf (msg+strlen(msg), "%sAS5600", comma);
    comma=",";
    sprintf (msg+strlen(msg), "%sWS(%s)", comma, pinNames[ANEMOMETER_IRQ_PIN]);
  }
  if (TLW_exists) {
    sprintf (msg+strlen(msg), "%sTLW", comma);
    comma=",";
  } 
  if (TSM_exists) {
    sprintf (msg+strlen(msg), "%sTSM", comma);
    comma=",";
  }
  if (TMSM_exists) {
    sprintf (msg+strlen(msg), "%sTMSM", comma);
    comma=",";
  } 
  if (HI_exists) {
    sprintf (msg+strlen(msg), "%sHI", comma);
    comma=",";
  }
  if (WBT_exists) {
    sprintf (msg+strlen(msg), "%sWBT", comma);
    comma=",";
  }
  if (WBGT_exists) {
    if (MCP_3_exists) {
      sprintf (msg+strlen(msg), "%sWBGT W/GLOBE", comma);
    }
    else {
      sprintf (msg+strlen(msg), "%sWBGT WO/GLOBE", comma);
    }
    comma=",";
  }
  if (PM25AQI_exists) {
    sprintf (msg+strlen(msg), "%sPM25AQ", comma);
  }
  if (cf_rg1_enable) {
    sprintf (msg+strlen(msg), "%sRG1(%s)", comma, pinNames[RAINGAUGE1_IRQ_PIN]); 
    comma=",";
  } 
  if (cf_op1 == 1) {
    sprintf (msg+strlen(msg), "%sOP1R(%s)", comma, pinNames[OP1_PIN]);
    comma=",";
  } 
  if (cf_op1 == 2) {
    sprintf (msg+strlen(msg), "%sRG2(%s)", comma, pinNames[RAINGAUGE2_IRQ_PIN]);
    comma=",";
  } 
  if (cf_op1 == 5) {
    sprintf (msg+strlen(msg), "%s5MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], cf_ds_baseline);
    comma=",";
  } 
  if (cf_op1 == 10) {
    sprintf (msg+strlen(msg), "%s10MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], cf_ds_baseline);
    comma=",";
  }
  if (cf_op2 == 1) {
    sprintf (msg+strlen(msg), "%sOP2R(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  }
  if (cf_op2 == 2) {
    sprintf (msg+strlen(msg), "%sVBV(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  } 
   // Close off sensors
  sprintf (msg+strlen(msg), "\"");

  // Adding closing }
  sprintf (msg+strlen(msg), "}");

  Serial_writeln(msg); 

  if (WiFi_Send(msg, cf_info_server, cf_info_server_port, cf_info_urlpath, METHOD_POST, cf_info_apikey)) {
    Output("INFO->PUB WiFi OK");
  }
  else {
    Output("INFO->PUB WiFi FAILED");

    // Save to the N2S File
    SD_NeedToSend_Add(msg);
  }

  // Update INFO.TXT file
  Serial_writeln(msg);
  SD_UpdateInfoFile(msg);
}

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

  // AFM0LoRa = Adafruit Feather M0 LoRa
  sprintf (msg, "\"at\":\"%s\",\"devid\":\"%s\",\"board\":\"AFM0LoRa\",\"ver\":\"%s\",\"bv\":%d.%02d,\"hth\":%d,\"elev\":%d",
    timestamp, DeviceID, versioninfo, 
    (int)vbat, (int)(vbat*100)%100,
    SystemStatusBits, cf_elevation);
  
  // obs_period (1,5,6,10,15,20,30), 1 minute observation period is the default
  // daily_reboot Number of hours between daily reboots, A value of 0 disables this feature
  sprintf (msg+strlen(msg), ",\"obsi\":\"%dm\",\"t2nt\":\"%ds\",\"drbt\":\"%dm\"",
    cf_obs_period, (int)((millis()-time_to_next_obs())/1000), cf_daily_reboot);

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


  // LoRa Configs and Discovered Device List

  // lwr  - LW Region: EU868 US915 ...
  // lwm  - LW Mode: OTAA, ABP
  // lwsf - LW Spreading Factor 7 or 8
  // lstp - LW Transmit Power 14 to 20
  sprintf (msg, "\"LW\":\"%s,%s,%d,%d\"",
    LW_Region(), 
    (cf_lw_mode) ? "ABP" : "OTAA",
    cf_lw_sf,
    cf_lw_txpw);

  // Discovered Device List
  comma="";
  sprintf (msg+strlen(msg), ",\"devs\":\"");
  if (eeprom_exists) {
    sprintf (msg+strlen(msg), "%seeprom", comma);
    comma=",";    
  }
  if (MUX_exists) {
    sprintf (msg+strlen(msg), "%smux", comma);
    comma=",";    
  }
  if (SD_exists) {
    sprintf (msg+strlen(msg), "%ssd", comma);
    comma=",";
  }
  if (gps_exists) {
    sprintf (msg+strlen(msg), "%sgps(%s)", comma, (gps_valid) ? "1" : "0");
    comma=","; 
  } 
  sprintf (msg+strlen(msg), "\""); 

  if (gps_exists && gps_valid) {
    sprintf (msg+strlen(msg), "%s\"lat\":%f,\"lon\":%f,\"alt\":%f,\"sat\":%d", 
      comma, gps_lat, gps_lon, gps_altm, gps_sat);
    comma=",";
  }

  // MUX SENSORS  
  if (MUX_exists) {
    comma="";
    sprintf (msg+strlen(msg), ",\"mux\":\"");
    for (int c=0; c<MUX_CHANNELS; c++) {
      if (mux[c].inuse) {
        for (int s = 0; s < MAX_CHANNEL_SENSORS; s++) {
          if (mux[c].sensor[s].type == m_tsm) {
            sprintf (msg+strlen(msg), "%sTSM%d(%d.%d)", comma, mux[c].sensor[s].id, c, s);
            comma=",";
          }
        }
      }
    }
    // Close off mux sensors
    sprintf (msg+strlen(msg), "\"");
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
  // ,BMX1(6)                                          =  13 bytes
  // ,BMX2(6)                                          =  13 bytes
  // ,MCP1                                             =   4 bytes
  // ,MCP2                                             =   4 bytes
  // ,MCP3/gt1                                         =   9 bytes
  // ,MCP4/gt2                                         =   9 bytes
  // ,SHT1                                             =   5 bytes
  // ,SHT2                                             =   5 bytes
  // ,HDC1                                             =   5 bytes
  // ,HDC2                                             =   5 bytes
  // ,LPS1                                             =   5 bytes
  // ,LPS2                                             =   5 bytes
  // ,HIH8                                             =   5 bytes
  // ,SI                                               =   3 bytes
  // ,VEML                                             =   5 bytes
  // ,BLX                                              =   4 bytes
  // ,AS5600,WS(A2)                                    =  14 bytes
  // ,TLW                                              =   4 bytes
  // ,TSM                                              =   4 bytes
  // ,TMSM                                             =   5 bytes
  // ,HI                                               =   3 bytes
  // ,WBT                                              =   4 bytes
  // ,WBGT WO/GLOBE                                    =  14 bytes
  // ,PM25AQ                                           =   7 bytes
  // ,RG1(A3)                                          =   7 bytes
  // ,RG2(A4)                                          =   7 bytes
  // ,DIST 10M(A5),8849                                =  17 bytes
  //                                               sum = 209 bytes
  //

  // SENSORS
  comma="";
  sprintf (msg, "\"sensors\":\"");

  if (BMX_1_exists) {
    sprintf (msg+strlen(msg), "%sBMX1(%s)", comma, bmxtype[BMX_1_type]);
    comma=",";
  }
  if (BMX_2_exists) {
    sprintf (msg+strlen(msg), "%sBMX2(%s)", comma, bmxtype[BMX_2_type]);
    comma=",";
  }
  if (MCP_1_exists) {
    sprintf (msg+strlen(msg), "%sMCP1", comma);
    comma=",";
  }
  if (MCP_2_exists) {
    sprintf (msg+strlen(msg), "%sMCP2", comma);
    comma=",";
  }
  if (MCP_3_exists) {
    sprintf (msg+strlen(msg), "%sMCP3/gt1", comma);
    comma=",";
  }
  if (MCP_4_exists) {
    sprintf (msg+strlen(msg), "%sMCP4/gt2", comma);
    comma=",";
  }
  if (SHT_1_exists) {
    sprintf (msg+strlen(msg), "%sSHT1", comma);
    comma=",";
  }
  if (SHT_2_exists) {
    sprintf (msg+strlen(msg), "%sSHT2", comma);
    comma=",";
  }
  if (HDC_1_exists) {
    sprintf (msg+strlen(msg), "%sHDC1", comma);
    comma=",";
  }
  if (HDC_2_exists) {
    sprintf (msg+strlen(msg), "%sHDC2", comma);
    comma=",";
  }
  if (LPS_1_exists) {
    sprintf (msg+strlen(msg), "%sLPS1", comma);
    comma=",";
  }
  if (LPS_2_exists) {
    sprintf (msg+strlen(msg), "%sLPS2", comma);
    comma=",";
  }
  if (HIH8_exists) {
    sprintf (msg+strlen(msg), "%sHIH8", comma);
    comma=",";
  }
  if (SI1145_exists) {
    sprintf (msg+strlen(msg), "%sSI", comma);
    comma=",";
  }
  if (VEML7700_exists) {
    sprintf (msg+strlen(msg), "%sVEML", comma);
    comma=",";
  }
  if (BLX_exists) {
    sprintf (msg+strlen(msg), "%sBLX", comma);
    comma=",";
  }
  if (AS5600_exists) {
    sprintf (msg+strlen(msg), "%sAS5600", comma);
    comma=",";
    sprintf (msg+strlen(msg), "%sWS(%s)", comma, pinNames[ANEMOMETER_IRQ_PIN]);
  }
  if (TLW_exists) {
    sprintf (msg+strlen(msg), "%sTLW", comma);
    comma=",";
  } 
  if (TSM_exists) {
    sprintf (msg+strlen(msg), "%sTSM", comma);
    comma=",";
  }
  if (TMSM_exists) {
    sprintf (msg+strlen(msg), "%sTMSM", comma);
    comma=",";
  } 
  if (HI_exists) {
    sprintf (msg+strlen(msg), "%sHI", comma);
    comma=",";
  }
  if (WBT_exists) {
    sprintf (msg+strlen(msg), "%sWBT", comma);
    comma=",";
  }
  if (WBGT_exists) {
    if (MCP_3_exists) {
      sprintf (msg+strlen(msg), "%sWBGT W/GLOBE", comma);
    }
    else {
      sprintf (msg+strlen(msg), "%sWBGT WO/GLOBE", comma);
    }
    comma=",";
  }
  if (PM25AQI_exists) {
    sprintf (msg+strlen(msg), "%sPM25AQ", comma);
  }
  if (cf_rg1_enable) {
    sprintf (msg+strlen(msg), "%sRG1(%s)", comma, pinNames[RAINGAUGE1_IRQ_PIN]); 
    comma=",";
  }
  if (cf_op1 == 1) {
    sprintf (msg+strlen(msg), "%sOP1R(%s)", comma, pinNames[OP1_PIN]);
    comma=",";
  } 
  if (cf_op1 == 2) {
    sprintf (msg+strlen(msg), "%sRG2(%s)", comma, pinNames[RAINGAUGE2_IRQ_PIN]);
    comma=",";
  } 
  if (cf_op1 == 5) {
    sprintf (msg+strlen(msg), "%s5MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], cf_ds_baseline);
    comma=",";
  } 
  if (cf_op1 == 10) {
    sprintf (msg+strlen(msg), "%s10MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], cf_ds_baseline);
    comma=",";
  }
  if (cf_op2 == 1) {
    sprintf (msg+strlen(msg), "%sOP2R(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  }
  if (cf_op2 == 2) {
    sprintf (msg+strlen(msg), "%sVBV(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  } 
  // Close off sensors
  sprintf (msg+strlen(msg), "\"");

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

/*
 * ======================================================================================================================
 * INFO_Do() - Get and Send System Information
 * =======================================================================================================================
 */
void INFO_Do() {
  if (WiFi_valid) {
    if (info_server_valid) {
      INFO_Do_WiFi();
    }
    else {
      Output ("INFO:INVLD INFO SVR");
    }
  }
  else {
    // Hold off LoRaWAN TX/RX job (uplink, join, or downlink window) is already in progress
    if (LMIC.opmode & OP_TXRXPEND) {
      Output ("INFO:LW BUSY");
      return;
    }
    Output ("INFO:LW READY");
    INFO_Do_LoRaWAN();
  }
  nextinfo = millis() + INFO_TIME_INTERVAL;
}