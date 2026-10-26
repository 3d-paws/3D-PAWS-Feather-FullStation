/*
 * ======================================================================================================================
 *  info.cpp - Station Information Functions
 * ======================================================================================================================
 */
#include "include/wifi.h"
#include "include/lorawan.h"
#include "include/notecard.h"
#include "include/info_wifi.h"
#include "include/info_notecard.h"
#include "include/info_lorawan.h"
#include "include/nvcf.h"
#include "include/output.h"
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
 * INFO_Initialize() - Test if we have set the Info Server in the config file - used later for WiFi
 * =======================================================================================================================
 */
void INFO_Initialize() {
  if ((scv.is_fqdn.length() > 0) && (scv.is_urlpath.length() > 0) && 
      (scv.is_apikey.length() > 0) && (scv.is_port != 0)) {

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
 * INFO_Do() - Get and Send System Information
 * =======================================================================================================================
 */
void INFO_Do() {

  if (notecard_exists) {
    INFO_Do_NoteCard();
  }
  else if (WiFi_valid) {
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