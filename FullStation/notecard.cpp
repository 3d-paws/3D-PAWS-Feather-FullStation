/*
 * ======================================================================================================================
 *  notecard.cpp - Notecard Functions
 * ======================================================================================================================
 */

#include "include/support.h"
#include "include/ssbits.h"
#include "include/output.h"
#include "include/time.h"
#include "include/nvcf.h"
#include "include/sensor_bmp.h"
#include "include/sensor_sht.h"
#include "include/sensor_mcp.h"
#include "include/sensor_htu.h"
#include "include/sensor_pm25aqi.h"
#include "include/sensor_tlw.h"
#include "include/derived.h"
#include "include/sensor_rgop.h" 
#include "include/main.h"
#include "include/notecard.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
bool notecard_exists = false;
bool notecard_ntn_enabled = false;
Notecard notecard;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 *=======================================================================================================================
 * notecard_hubsync() - Force the Notecard to synchronize its state outward to Notehub.
 *=======================================================================================================================
 */
bool notecard_hubsync() {
  Output("NC:HUBSYNC");

  J *syncReq = notecard.newRequest("hub.sync");
  if (syncReq == NULL) {
    Output("NC:New(hub.sync) ERR");
    notecard_exists = false;
    return(false);
  }

  // Limit this manual sync to outbound Notefiles only.
  JAddBoolToObject(syncReq, "out", true);

  // sendRequestWithRetry() takes ownership of syncReq.
  // Do not JDelete(syncReq) after this call.
  if (!notecard.sendRequestWithRetry(syncReq, scv.nc_rr)) {
    notecard_exists = false;
    Output("NC:Send(hub.sync) ERR");
    return(false);
  }

  Output("NC:HUBSYNC OK");
  return(true);
}

/*
 *=======================================================================================================================
 * notecard_hubsync_status() - Read and print the Notecard synchronization status.
 *
 * Returns true:
 *   A response was received with no API error and no sync alert.
 *
 * Returns false:
 *   Request allocation failed, no response was received, an API error occurred,
 *   or the most recent sync reported an alert.
 *
 * This does not initiate a sync or wait for synchronization to complete.
 *=======================================================================================================================
 */
bool notecard_hubsync_status() {
  Output("NC:HUBSYNC_STATUS");

  J *statusReq = notecard.newRequest("hub.sync.status");
  if (statusReq == NULL) {
    Output("NC:New(hub.sync.status) ERR");
    return(false);
  }

  // requestAndResponse() takes ownership of statusReq.
  // The caller must delete the returned response.
  J *statusRsp = notecard.requestAndResponse(statusReq);
  if (statusRsp == NULL) {
    notecard_exists = false;
    Output("NC:Send(hub.sync.status) ERR");
    return(false);
  }

  // Print the complete response for troubleshooting.
  char *statusJson = JPrintUnformatted(statusRsp);
  if (statusJson != NULL) {
    Output(statusJson);
    JFree(statusJson);
  } else {
    Output("NC:Print(hub.sync.status) ERR");
  }

  bool apiError = JHasObjectItem(statusRsp, "err");
  bool syncAlert = JGetBool(statusRsp, "alert");

  JDelete(statusRsp);

  if (apiError) {
    Output("NC:HUBSYNC_STATUS API ERR");
    return(false);
  }

  if (syncAlert) {
    Output("NC:HUBSYNC_STATUS SYNC ALERT");
    return(false);
  }

  Output("NC:HUBSYNC_STATUS READ OK");
  return(true);
}

/*
 *=======================================================================================================================
 * notecard_wireless_status() - Read and print the Notecard wireless status.
 *
 * Returns true if a response was received, printed, and contained no API error.
 * Does not modify wireless settings or initiate a connection.
 * A true return value does not mean the Notecard is connected.
 *=======================================================================================================================
 */
bool notecard_wireless_status() {
  Output("NC:WIRELESS_STATUS");

  J *wirelessReq = notecard.newRequest("card.wireless");
  if (wirelessReq == NULL) {
    Output("NC:New(card.wireless) ERR");
    return(false);
  }

  // requestAndResponse() takes ownership of wirelessReq.
  J *wirelessRsp = notecard.requestAndResponse(wirelessReq);
  if (wirelessRsp == NULL) {
    notecard_exists = false;
    Output("NC:Send(card.wireless) ERR");
    return(false);
  }

  notecard_exists = true;

  char *wirelessJson = JPrintUnformatted(wirelessRsp);
  if (wirelessJson == NULL) {
    JDelete(wirelessRsp);
    Output("NC:Print(card.wireless) ERR");
    return(false);
  }

  Output(wirelessJson);
  JFree(wirelessJson);

  bool apiError = JHasObjectItem(wirelessRsp, "err");

  JDelete(wirelessRsp);

  if (apiError) {
    Output("NC:WIRELESS_STATUS API ERR");
    return(false);
  }

  Output("NC:WIRELESS_STATUS READ OK");
  return(true);
}

/*
 *=======================================================================================================================
 * notecard_wifi_status() - Read and print the current Notecard Wi-Fi configuration.
 *
 * Returns true if a response was received, printed, and contained no API error.
 * Does not modify Wi-Fi settings or confirm connectivity.
 *=======================================================================================================================
 */
bool notecard_wifi_status() {
  Output("NC:WIFI_STATUS");

  J *wifiReq = notecard.newRequest("card.wifi");
  if (wifiReq == NULL) {
    Output("NC:New(card.wifi) ERR");
    return(false);
  }

  // requestAndResponse() takes ownership of wifiReq.
  J *wifiRsp = notecard.requestAndResponse(wifiReq);
  if (wifiRsp == NULL) {
    notecard_exists = false;
    Output("NC:Send(card.wifi) ERR");
    return(false);
  }

  notecard_exists = true;

  char *wifiJson = JPrintUnformatted(wifiRsp);
  if (wifiJson == NULL) {
    JDelete(wifiRsp);
    Output("NC:Print(card.wifi) ERR");
    return(false);
  }

  Output(wifiJson);
  JFree(wifiJson);

  bool apiError = JHasObjectItem(wifiRsp, "err");

  JDelete(wifiRsp);

  if (apiError) {
    Output("NC:WIFI_STATUS API ERR");
    return(false);
  }

  Output("NC:WIFI_STATUS READ OK");
  return(true);
}

/*
 * ======================================================================================================================
 * nc_add_template_body() - build template body from active sensors
 * 
 * Template value | Compact type            | C/C++ equivalent      | Typical range / use 
 * -------------- | ----------------------- | --------------------- | ---------------------------------------------------
 * 11             | 1-byte signed integer   | int8_t                | −128 to 127 
 * 12             | 2-byte signed integer   | int16_t               | −32,768 to 32,767
 * 14             | 4-byte signed integer   | int32_t               | −2,147,483,648 to 2,147,483,647
 * 18             | 8-byte signed integer   | int64_t               | Very large signed counters/timestamps
 * 21             | 1-byte unsigned integer | uint8_t               | 0 to 255
 * 22             | 2-byte unsigned integer | uint16_t              | 0 to 65,535
 * 24             | 4-byte unsigned integer | uint32_t              | 0 to 4,294,967,295
 * 28             | 8-byte unsigned integer | uint64_t              | Very large non-negative counters
 * 14.1           | 4-byte float            | float / float32_t     | Sensor values with fractions
 * 18.1           | 8-byte float            | double / float64_t    | Higher precision, 8-byte cost
 * true           | Boolean                 | bool                  | true or false
 * "text"         | String                  | const char * / string | Variable length; avoid for satellite-critical telemetry
 * ======================================================================================================================
 */
void nc_add_template_body(J *body) {

  JAddNumberToObject(body, "ts", 24);           // 4 byte unsigned
  JAddNumberToObject(body, "bv", 14.1);         // 4 byte float
  JAddNumberToObject(body, "hth", 14.1);        // 4 byte float

  if (!scv.aqs) {
    // Rain Gauge 1 - Each tip is 0.2mm of rain
    if (scv.rg1) {
      JAddNumberToObject(body, "rg1", 14.1);
      JAddNumberToObject(body, "rgt1", 14.1);
      JAddNumberToObject(body, "rgp1", 14.1);
    }

    // Rain Gauge 2 - Each tip is 0.2mm of rain
    if (nv_op1_state() == OP1_STATE_RAIN) {
      JAddNumberToObject(body, "rg2", 14.1);
      JAddNumberToObject(body, "rgt2", 14.1);
      JAddNumberToObject(body, "rgp2", 14.1);
    }
 
    // OP1 Raw
    if (nv_op1_state() == OP1_STATE_RAW) {   
      JAddNumberToObject(body, "op1r", 14.1);
    }

    // OP1 Distance
    if ((nv_op1_state() == OP1_STATE_DIST_5M) || (nv_op1_state() == OP1_STATE_DIST_10M)) {
      JAddNumberToObject(body, "ds", 14.1);
      JAddNumberToObject(body, "dsr", 14.1);
    }

    // OP2 Raw
    if (nv_op2_state() == OP2_STATE_RAW) {
      JAddNumberToObject(body, "op2r", 14.1);
    }

    // OP2 Voltaic Battery Voltage
    if (nv_op2_state() == OP2_STATE_VOLTAIC) {
      JAddNumberToObject(body, "vbv", 14.1);
      JAddNumberToObject(body, "vpc", 14.1);
    }

    // Wind
    if (scv.wind) {
      JAddNumberToObject(body, "ws", 14.1); // Wind Speed
      JAddNumberToObject(body, "wd", 12);   // Wind Direction -2 Byte signed
      JAddNumberToObject(body, "wg", 14.1); // Wind Gust
      JAddNumberToObject(body, "wd", 12);   // Wind Gust Direction (Global)
    }
  }

  // Air Quality
  if (PM25AQI_exists) {
    JAddNumberToObject(body, "pm1e10", 14.1);
    JAddNumberToObject(body, "pm1e25", 14.1);
    JAddNumberToObject(body, "pm1e100", 14.1);
  }

  // BMP1
  if (BMP_1_exists) {
    JAddNumberToObject(body, "bt1", 14.1);  // BMP1 Temperature
    JAddNumberToObject(body, "bp1", 14.1);  // BMP1 Preasure
  }
  // BMP2
  if (BMP_2_exists) {
    JAddNumberToObject(body, "bt2", 14.1);  // BMP2 Temperature
    JAddNumberToObject(body, "bp2", 14.1);  // BMP2 Preasure

  }

  // SHT1
  if (SHT_1_exists) {
    JAddNumberToObject(body, "st1", 14.1);  // SHT1 Temperature
    JAddNumberToObject(body, "sh1", 14.1);  // SHT1 Humidity
  }
  // SHT2
  if (SHT_2_exists) {
    JAddNumberToObject(body, "st2", 14.1);  // SHT2 Temperature
    JAddNumberToObject(body, "sh2", 14.1);  // SHT2 Humidity
  }

  if (MCP_1_exists) {
    JAddNumberToObject(body, "mt1", 14.1);
  }
  if (MCP_2_exists) {
    JAddNumberToObject(body, "mt2", 14.1);
  }
  if (MCP_3_exists) {
    JAddNumberToObject(body, "mt3", 14.1);
  }
  if (MCP_4_exists) {
    JAddNumberToObject(body, "mt4", 14.1);
  }  
  if (HTU_exists) {
    JAddNumberToObject(body, "ht1", 14.1);
    JAddNumberToObject(body, "hh1", 14.1);
  }
  if (TLW_exists) {
    JAddNumberToObject(body, "tlww", 14.1);
    JAddNumberToObject(body, "tlwt", 14.1);
  }

  // Heat Index Temperature
  if (HI_exists) {
    JAddNumberToObject(body, "hi", 14.1);   
  }
  
  // Wet Bulb Temperature
  if (WBT_exists) {
    JAddNumberToObject(body, "wbt", 14.1);   
  }

  // Wet Bulb Globe Temperature
  if (WBGT_exists) {
    JAddNumberToObject(body, "wbgt", 14.1);
  }

  if (MSLP_exists) {
    JAddNumberToObject(body, "mslp", 14.1);
  }



  // Tinovi Soil Moisture
  // mux_obs_do(sidx);

  // Dallas Sensors Temperature on mux
  // dsmux_obs_do(sidx);
}

/*
 *=======================================================================================================================
 * notecard_set_t_obs() - Set template for obs messages
 *=======================================================================================================================
 */
bool notecard_set_t_obs() {
  Output("NC:SET_T_O");
  // Create OBS template
  J *TemplateReq_Info = notecard.newRequest("note.template");
  if (TemplateReq_Info == NULL) {
    Output("NC:New(note.t.obs) ERR");
    return(false);
  }
  JAddStringToObject(TemplateReq_Info, "file", scv.nc_qo_obs.c_str());
  JAddStringToObject(TemplateReq_Info, "format", "compact");
  JAddNumberToObject(TemplateReq_Info, "port", scv.nc_qo_port_obs);

  J *body = JAddObjectToObject(TemplateReq_Info, "body");
  JAddStringToObject(body, "info", "text");

  if (notecard.sendRequestWithRetry(TemplateReq_Info, scv.nc_rr)) {  // Retry for up to 5 seconds
    Output("NC:Send(note.t.obs) OK");
  }
  else {
    notecard_exists = false;
    Output("NC:Send(note.t.obs) ERR");
    return (false);
  }
  return (true);
}

/*
 *=======================================================================================================================
 * notecard_clr_t_obs() - Set template for obs messages
 *=======================================================================================================================
 */
bool notecard_clr_t_obs() {
  Output("NC:CLR_T_O");
  // Clear OBS template
  J *TemplateReq_Info = notecard.newRequest("note.template");
  if (TemplateReq_Info == NULL) {
    Output("NC:New(note.t.obs) ERR");
    return(false);
  }
  JAddStringToObject(TemplateReq_Info, "file", scv.nc_qo_obs.c_str());

  if (notecard.sendRequestWithRetry(TemplateReq_Info, scv.nc_rr)) {  // Retry for up to 5 seconds
    Output("NC:Send(note.t.obs) OK");
  }
  else {
    notecard_exists = false;
    Output("NC:Send(note.t.obs) ERR");
    return (false);
  }
  return (true);
}

/*
 *=======================================================================================================================
 * notecard_set_t_info() - Set template for info messages
 *=======================================================================================================================
 */
bool notecard_set_t_info() {
  Output("NC:SET_T_I");
  // Create INFO template
  J *TemplateReq_Info = notecard.newRequest("note.template");
  if (TemplateReq_Info == NULL) {
    Output("NC:New(note.t.info) ERR");
    return(false);
  }
  JAddStringToObject(TemplateReq_Info, "file", scv.nc_qo_info.c_str());
  JAddStringToObject(TemplateReq_Info, "format", "compact");
  JAddNumberToObject(TemplateReq_Info, "port", scv.nc_qo_port_info);

  J *body = JAddObjectToObject(TemplateReq_Info, "body");
  JAddStringToObject(body, "info", "1"); // 1= is saying we have a payload (pl) of type 1
                                         // A note will include a tag "pl":"Base64 text string so it cannot break JSON syntax"
                                         // A 150 byte source message will be ~200 byte Base64 message
  if (notecard.sendRequestWithRetry(TemplateReq_Info, scv.nc_rr)) {  // Retry for up to 5 seconds
    Output("NC:Send(note.t.info) OK");
  }
  else {
    notecard_exists = false;
    Output("NC:Send(note.t.info) ERR");
    return (false);
  }
  return (true);
}

/*
 *=======================================================================================================================
 * notecard_clr_t_info() - Set template for info messages
 *=======================================================================================================================
 */
bool notecard_clr_t_info() {
  Output("NC:CLR_T_I");
  // Create INFO template
  J *TemplateReq_Info = notecard.newRequest("note.template");
  if (TemplateReq_Info == NULL) {
    Output("NC:New(note.t.info) ERR");
    return(false);
  }
  JAddStringToObject(TemplateReq_Info, "file", scv.nc_qo_info.c_str());

  if (notecard.sendRequestWithRetry(TemplateReq_Info, scv.nc_rr)) {  // Retry for up to 5 seconds
    Output("NC:Send(note.t.info) OK");
  }
  else {
    notecard_exists = false;
    Output("NC:Send(note.t.info) ERR");
    return (false);
  }
  return (true);
}

/*
 *=======================================================================================================================
 * notecard_scv_validate() - Validate NoteCard Config File Variables
 *=======================================================================================================================
 */
bool notecard_scv_validate() {
  bool ok = true;

  Output("NC:SCV VALIDATE");

  if (scv.nc_product == "") {
    Output("NC:CFG product not set");
    ok=false;
  }
  if ((scv.nc_mode != "cm") && (scv.nc_mode != "pm") && (scv.nc_mode != "mm")) {
    sprintf (msgbuf, "NC:CFG mode not [cm,pm,mm] but [%.10s]", scv.nc_mode.c_str());
    Output(msgbuf);
    ok=false;
  }
  if ((scv.nc_method != "c")  && (scv.nc_method != "w")  && // (scv.nc_method != "n") &&  // not supporting n-only
      (scv.nc_method != "cn") && (scv.nc_method != "wc") && (scv.nc_method != "wn") && (scv.nc_method != "wcn")) {
    sprintf (msgbuf, "NC:CFG method [c,w,wc...] but [%.10s]", scv.nc_method.c_str());
    Output(msgbuf);
    ok=false;
  }
  if (((scv.nc_method == "w") || (scv.nc_method == "wc") || (scv.nc_method == "wn") || (scv.nc_method == "wcn")) &&
       (scv.nc_wifi_ssid == "") ) {
    Output("NC:CFG wifi_ssid not set");
    ok=false;
  }

  if ((scv.nc_esim_method != "") && (scv.nc_esim_method != "p") && (scv.nc_esim_method != "s") && 
      (scv.nc_esim_method != "dps") && (scv.nc_esim_method != "dsp")) {
    sprintf (msgbuf, "NC:CFG esim_method invalid [%.10s]", scv.nc_esim_method.c_str());
    Output(msgbuf);
    ok=false;
  }

  if (scv.nc_esim_method == "") {     // Force embedded Blues SIM if not set in config
    scv.nc_esim_method = "p";
  }

  if ((scv.nc_esim_method != "p") && (scv.nc_esim_apn == "")) { // aka we have a 2nd sim and apn is null and required
    Output("NC:CFG esim_apn not set");
    ok=false;  
  }
  if ((scv.nc_method != "cn") && (scv.nc_wifi_ssid == "")) { // aka we have wifi but ssid is null
    Output("NC:CFG wifi_ssid not set");
    ok=false;
  }
  if ((scv.nc_outbound < 0) || (scv.nc_outbound > 43200)) {
    sprintf (msgbuf, "NC:CFG outbound not [0-43200] [%d]", scv.nc_outbound);
    Output(msgbuf);   
  }
  if ((scv.nc_inbound < 0) || (scv.nc_inbound > 43200)) {
    sprintf (msgbuf, "NC:CFG inbound not [0-43200] [%d]", scv.nc_inbound);
    Output(msgbuf); 
    ok=false;  
  }
  return(ok);
}

/*
 *=======================================================================================================================
 * notcard_method() - return notecard's method config based on system config variable svc.nc_method
 *                    Also set if ntn is enabled
 *=======================================================================================================================
 */
const char *notcard_method() {
    if (scv.nc_method == "c") {
        return "cell";
    }

    if (scv.nc_method == "w") {
        return "wifi";
    }

    /* Not supporting ntn only
    if (scv.nc_method == "n") {
        notecard_ntn_enabled = true;
        return "ntn";
    }
    */

    if (scv.nc_method == "cn") {
        notecard_ntn_enabled = true;
        return "cell-ntn";
    }

    if (scv.nc_method == "wc") {
        return "wifi-cell";
    }

    if (scv.nc_method == "wn") {
        notecard_ntn_enabled = true;
        return "wifi-ntn";
    }

    if (scv.nc_method == "wcn") {
        notecard_ntn_enabled = true;
        return "wifi-cell-ntn";
    }

    // This includes unsupported "cw" and "cwn".
    return nullptr;
}

/*
 *=======================================================================================================================
 * notecard_initialize() - Set "notecard_exists" and initialize if found
 *=======================================================================================================================
 */
void notecard_initialize() {
  Output("NC:INIT");

  if (!I2C_Device_Exist(NOTECARD_I2C_ADDRESS)) {
    Output("NC:NF");
    notecard_exists = false;
    return;
  }

  if (!notecard_scv_validate()) {
    notecard_exists = false;
    return;
  }

  notecard.begin();  // I2C, default address 0x17

  if (SerialConsoleEnabled) {
    notecard.setDebugOutputStream(Serial);  // Echo Notecard request/response traffic to console
  }

  // =======================================================================================================================
  // hub.set - 
  // Is the Notecard request used to configure how the Notecard identifies itself and communicates with Notehub. 
  // It stores persistent settings in the Notecard, including its Notehub ProductUID, synchronization mode, scheduled 
  // inbound/outbound sync intervals, device serial number, firmware version, and selected advanced power/communication behavior.
  // =======================================================================================================================
  J *hubReq = notecard.newRequest("hub.set");
  if (hubReq == NULL) {
    Output("NC:New(hub.set) ERR");
    notecard_exists = false;
    return;
  }

  // Direct Notecard library diagnostic output to the host debug serial port.
  // This is a local host-library setting; it sends no request to, and changes
  // no persistent configuration on, the Notecard.
  JAddStringToObject(hubReq, "product", scv.nc_product.c_str());

  if (scv.nc_mode == "cm") {
    // "continuous" maintains an active network and Notehub connection. 
    // For Cellular Notecards that means keeping the cellular modem powered; 
    // for Wi-Fi devices it maintains association with the AP.
    // Use it for mains-powered devices that need quick remote control, inbound Signals, or low-latency command reception. 
    // It is normally a poor fit for an energy-constrained station because the modem remains active. 
    JAddStringToObject(hubReq, "mode", "continuous");
    JAddNumberToObject(hubReq, "outbound", scv.nc_outbound);
    JAddNumberToObject(hubReq, "inbound",  scv.nc_inbound);
  }
  else if (scv.nc_mode == "mm") {
    // "minimum" blocks automatic connection and synchronization. 
    // The Notecard does not connect simply because it has queued Notes or an inbound timer; 
    // your host MCU must deliberately send hub.sync
    // It is useful when your firmware has its own rigorous power, time-of-day, or sampling policy and you want no autonomous modem wakeups. 
    // However, OTA host and Notecard DFU is unavailable in minimum mode—even if you issue manual syncs—so it is less recoverable for unattended deployments.
    JAddStringToObject(hubReq, "mode", "minimum");
  }
  else {
    // "periodic" is the default after a factory reset. 
    // The Notecard powers its transport radio only when it needs to communicate with Notehub, then powers it down afterward. 
    // This is usually what you want for solar/battery environmental instrumentation.
    JAddStringToObject(hubReq, "mode", "periodic");
    JAddNumberToObject(hubReq, "outbound", scv.nc_outbound);
    JAddNumberToObject(hubReq, "inbound",  scv.nc_inbound);
  }

  // Note that `notecard.sendRequestWithRetry()` always frees the request data
  // structure, and it returns "true" if success or "false" if there is any
  // failure. It is important to use `sendRequestWithRetry()` on the first
  // message from the MCU to the Notecard, because there will always be a
  // hardware race condition on cold boot and the Notecard must be ready to
  // receive and process the message.
  if (notecard.sendRequestWithRetry(hubReq, scv.nc_rr)) {  // Retry for up to 5 seconds
    Output("NC:Send(hub.set) OK");
  }
  else {
    notecard_exists = false;
    Output("NC:Send(hub.set) ERR");
    return;
  }

  // =======================================================================================================================
  // card.wireless - Cellular SIM policy
  // =======================================================================================================================
  if (scv.nc_esim_method.length() > 0) {
    J *wirelessReq = notecard.newRequest("card.wireless");
    if (wirelessReq == NULL) {
      Output("NC:New(card.wireless) ERR");
      notecard_exists = false;
      return;
    }

    if (scv.nc_esim_method == "p") {
        JAddStringToObject(wirelessReq, "method", "primary");    // embedded Blues SIM
    }
    else if (scv.nc_esim_method == "s") {
        JAddStringToObject(wirelessReq, "method", "secondary");  // your external SIM
    }
    else if (scv.nc_esim_method == "dps") {
        JAddStringToObject(wirelessReq, "method", "dual-primary-secondary");
    }
    else if (scv.nc_esim_method == "dsp") {
        JAddStringToObject(wirelessReq, "method", "dual-secondary-primary");
    }

    // set the external sim apn
    if (scv.nc_esim_method != "p") {
      JAddStringToObject(wirelessReq, "apn", scv.nc_esim_apn.c_str());
    }

    // Use sendRequestWithRetry() on this first request - the Notecard may still be booting.
    if (notecard.sendRequestWithRetry(wirelessReq, scv.nc_rr)) {  // Retry for up to 5 seconds
        Output("NC:Send(card.wireless) OK");
    }
    else {
      notecard_exists = false;
      Output("NC:Send(card.wireless) ERR");
      return;
    }
  }

  // =======================================================================================================================
  // card.wifi
  // =======================================================================================================================
  if ((scv.nc_method == "w")  || (scv.nc_method == "cw") || (scv.nc_method == "cwn") ||
      (scv.nc_method == "wc") || (scv.nc_method == "wn") || (scv.nc_method == "wcn")) {

    J *wifiReq = notecard.newRequest("card.wifi");
    if (wifiReq == NULL) {
      Output("NC:New(card.wifi) ERR");
      notecard_exists = false;
      return;
    }

    JAddStringToObject(wifiReq, "ssid", scv.nc_wifi_ssid.c_str());
    JAddStringToObject(wifiReq, "password", scv.nc_wifi_pw.c_str());

    if (notecard.sendRequestWithRetry(wifiReq, scv.nc_rr)) {  // Retry for up to 5 seconds
      Output("NC:Send(card.wifi) OK");
    }
    else {
      notecard_exists = false;
      Output("NC:Send(card.wifi) ERR");
      return;
    }

    notecard_wifi_status();
  }

  // =======================================================================================================================
  // card.transport
  // =======================================================================================================================
  J *transportReq = notecard.newRequest("card.transport");
  if (transportReq == NULL) {
    Output("NC:New(card.transport) ERR");
    notecard_exists = false;
    return;
  }

  JAddStringToObject(transportReq, "method", notcard_method());

  if (notecard.sendRequestWithRetry(transportReq, scv.nc_rr)) {  // Retry for up to 5 seconds
    Output("NC:Send(card.transport) OK");
  }
  else {
    notecard_exists = false;
    Output("NC:Send(card.transport) ERR");
    return;
  }

  // =======================================================================================================================
  // NTN Enabled - setup info and obs templates
  // =======================================================================================================================
  if (notecard_ntn_enabled) {
    notecard_set_t_info();
    notecard_set_t_obs();
  }
  else {
    // Clear any prior templates, we are not using them
    notecard_clr_t_info();
    notecard_clr_t_obs();
  }
  notecard_hubsync();
  notecard_hubsync_status();

  notecard_exists = true;
}

/*
 *=======================================================================================================================
 * notecard_sync_rtc() - Set the DS3231 from the Notecard's own network-synced clock (card.time), so the
 * RTC never has to be set by hand over serial and can't drift a day off like it did before.
 *
 *   card.time only returns a valid "time" once the Notecard has completed at least one sync with
 *   Notehub, which can take a few seconds after hub.set returns - so this retries briefly rather
 *   than giving up on the first miss. If it never succeeds this boot (e.g. no connectivity yet),
 *   RTC_valid/the clock are simply left as rtc_initialize() already found them, and this is tried
 *   again fresh on the next boot.
 *=======================================================================================================================
 */
bool notecard_rtc_sync() {
  if (!notecard_exists) {
    return (false);
  }

  Output("NC:RTC_SYNC");

  // 2026-01-01 00:00:00 UTC
  constexpr double MIN_EPOCH = 1767225600UL;

  // Ten calendar years: 3652 days, including leap days in 2028 and 2032.
  // Exclusive upper bound: 2036-01-01 00:00:00 UTC
  constexpr double MAX_EPOCH = MIN_EPOCH + (3652UL * 86400UL);

  constexpr int MAX_ATTEMPTS = 5;

  for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
    J *req = notecard.newRequest("card.time");
    if (req == NULL) {
      Output("NC:RTC_SYNC REQ FAIL");
      return (false);
    }

    J *rsp = notecard.requestAndResponse(req);

    if (rsp != NULL) {
      if (!notecard.responseError(rsp) && JHasObjectItem(rsp, "time")) {

        const double value = JGetNumber(rsp, "time");

        notecard.deleteResponse(rsp);
    
        // Validate before converting to an unsigned integer.
        // This also rejects NaN and infinity.
        if (value >= MIN_EPOCH && value < MAX_EPOCH) {
          uint32_t epoch = static_cast<uint32_t>(value);

          rtc.adjust(DateTime(epoch));  // DateTime(uint32_t) takes a Unix epoch directly

          RTC_valid = true;
          SystemStatusBits &= ~SSB_RTC;  // Turn Off Bit

          rtc_timestamp();
          sprintf(msgbuf, "NC:RTC SET %s", timestamp);
          Output(msgbuf);

          return (true);
        }
        else {
          // Extracted value failed the date-range check.
          Output("NC:RTC_SYNC BAD TIME");        
        }
      }
      else {
        // Response contains an error or is missing the time field.
        notecard.deleteResponse(rsp);
        Output("NC:RTC_SYNC BAD RESPONSE");
      }
    }
    else {
      Output("NC:RTC_SYNC NO RESPONSE");
    }

    if (attempt + 1 < MAX_ATTEMPTS) {
      Output("NC:RTC_SYNC WAIT");
      if (!RTC_valid) {
        delay(2000);
      }
      else {
        // Delay 2 seconds by doing our background work 
        BackGroundWork();
        BackGroundWork();
      }
    }
  }

  Output("NC:RTC_SYNC NF");
  return (false);
}

/*
 *=======================================================================================================================
 * notecard_qtt_obs() - Queue Terrestrial Transmission observation on the Notecard for delivery to Notehub.io
 *
 * We already built a valid JSON observation string, so we simply hand that same string to the Notecard as the note body.
 * Notecard buffers notes in its own flash whenever the network is unavailable and delivers them once connectivity 
 * returns, so no need 2 send
 *=======================================================================================================================
 */
void notecard_qtt_obs(char *obs) {
  if (!notecard_exists) {
    return;
  }

  J *req = notecard.newRequest("note.add");
  if (req == NULL) {
    Output("NC:MEM ERR");
    return;
  }

  JAddStringToObject(req, "file", scv.nc_qo_obs.c_str());

  J *body = JParse(obs);
  if (body != NULL) {
    JAddItemToObject(req, "body", body);
  }

  if ((scv.nc_mode == "mm") || (scv.nc_outbound == 0)) { 
    // Minimum Mode - we are responsible for calling hub.sync, aka synchronize with Notehub now
    // No periodic schedule set to connect and transmit, so we must call hub.sync

    // Setting sync to true in request tells the Notecard to initiate synchronization 
    // immediately, rather than waiting for the normal outbound schedule
    JAddBoolToObject(req, "sync", true);
  }

  // sendRequest() always frees req, even on failure.
  if (!notecard.sendRequest(req)) {
    Output("NC:SEND ERR");
  }
}

/*
 *=======================================================================================================================
 * notecard_qtt_info() - Queue Terrestrial Transmission info on the Notecard for delivery to Notehub.io
 *
 * We already built a valid JSON observation string, so we simply hand that same string to the Notecard as the note body.
 * Notecard buffers notes in its own flash whenever the network is unavailable and delivers them once connectivity 
 * returns, so no need 2 send
 *=======================================================================================================================
 */
void notecard_qtt_info(char *info) {
  if (!notecard_exists) {
    return;
  }

  J *req = notecard.newRequest("note.add");
  if (req == NULL) {
    Output("NC:MEM ERR");
    return;
  }

  JAddStringToObject(req, "file", scv.nc_qo_info.c_str());

  J *body = JParse(info);
  if (body != NULL) {
    JAddItemToObject(req, "body", body);
  }

  // Lets force the transmission of info to nothub, ignoring mode and queuing policies
  JAddBoolToObject(req, "sync", true);

  // sendRequest() always frees req, even on failure.
  if (!notecard.sendRequest(req)) {
    Output("NC:SEND ERR");
  }
}


