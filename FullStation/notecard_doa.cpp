/*
 * ======================================================================================================================
 *  notecard_doa.cpp - Handle Notecard DO Action Functions
 * ======================================================================================================================
 */

#include "include/qc.h"
#include "include/support.h"
#include "include/output.h"
#include "include/time.h"
#include "include/nvcf.h"
#include "include/eeprom.h"
#include "include/feather.h"
#include "include/info_notecard.h"
#include "include/main.h"
#include "include/sdcard.h"
#include "include/sensor_rgop.h"
#include "include/derived.h"
#include "include/notecard.h"
#include "include/notecard_doa.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
unsigned long nc_doa_check_time=300000; // 5 minutes after boot ask note card if there is a DOA, value 5*60*1000 ms

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * notecard_doa_perform() - Do, DoAction Request
 * ======================================================================================================================
 */
void notecard_doa_perform(String s) {
  if (s.equals("REBOOT")) {  // Reboot - We loose untransmitted observations. But they are save to SD.
    Output("DOA:REBOOT");     // Do a SEND before a REBOOT to address the abive issue.
    EEPROM_SaveUnreportedRain();
    delay(1000);

    DeviceReset();
  }

  else if (s.equals("INFO")) {  // Send System Information
    Output("DOA:INFO");
    INFO_Do_NoteCard();
    notecard_hubsync();
  }

  else if (s.equals("SEND")) {  // Send OBS Now
    Output("DOA:SEND");
    notecard_hubsync();  
  }

  else if (s.equals("CRT")) { // Clear Rain Totals
    uint32_t current_time = rtc_unixtime();
    Output("DOA:CRT");
    EEPROM_ClearRainTotals(current_time);
    // Display EEPROM Information 
    // EEPROM_Dump();   
  }

  else if (s.equals("CNV")) { // Clear NV Config Files
    Output("DOA:CNV");
    nv_deleteConfigIfCnvFileExists(); // this does not return if all went well
  }

  else if (s.equals("NOWIND")) { 
    Output("DOA:NOWIND");
    scv.wind=false;
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.equals("DOWIND")) {
    Output("DOA:DOWIND");
    scv.wind=true;
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR"); 
  }

  else if (s.equals("NORAIN")) { 
    Output("DOA:NORAIN");
    scv.rg1=false;
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.equals("DORAIN")) { 
    Output("DOA:DORAIN");
    scv.rg1=true;
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR"); 
  }

  // op1: null,raw,d5m,d10m,rg2      
  // op2: null,raw,voltaic
  else if (s.equals("OP1D5M")) { // Set 5M Distance Sensor State File
    Output("DOA:OP1D5M");
    scv.op1 = "d5m";
    dg_resolution_adjust = 1.25;
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.equals("OP1D10M")) { // Set OP1 State File to 10m Distance 
    Output("DOA:OP1D10M");
    scv.op1 = "d10m";
    dg_resolution_adjust = 2.5;
    pinMode(OP1_PIN, INPUT);
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.equals("OP1RAIN")) { // Set OP1 State File to Rain
    Output("DOA:OP1RAIN");
    scv.op1 = "rg2";
    dg_resolution_adjust = 2.5;
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.equals("OP1RAW")) { // Set OP1 State File to Raw
    Output("DOA:OP1RAW");
    scv.op1 = "raw";
    dg_resolution_adjust = 2.5;
    pinMode(OP1_PIN, INPUT);
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.equals("OP1CLR")) {
    Output("DOA:OP1CLR");
    scv.op1 = "null";
    dg_resolution_adjust = 2.5;
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.equals("OP2RAW")) { // Set OP2 State File to Raw
    Output("DOA:OP2RAW");
    scv.op2 = "raw";
    pinMode(OP2_PIN, INPUT);
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.equals("OP2VBV")) { // Set OP2 State File to Voltaic Battery Voltage
    Output("DOA:OP2VBV");
    scv.op2 = "voltaic";
    pinMode(OP2_PIN, INPUT);
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }
    
  else if (s.equals("OP2CLR")) { // Clear OP2 State Files
    Output("DOA:OP2CLR");
    scv.op2 = "null";
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.startsWith("OBSI:")) { // Pattern start of string aka 0 offset
    Output("DOA:OBSI");
    String rest = s.substring(5); // get part after "OBSI:", 5 = length of 
    int interval = rest.toInt();  // convert to integer

    if ((String(interval) == rest) && ((interval >= 1)) && (interval <= 60)) {
      scv.obsi = interval; // Set running value of elevation
      (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
    }
    else {
      Output("DOA:OBSI, INVALID INTERVAL");            
    }
  }

  else if (s.equals("OPTAQS")) { // Enable Air Quality Station
    Output("DOA:OPTAQS");
    scv.aqs = true;
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.equals("OPTFS")) { // Enable Full Station
    Output("DOA:OPTFS");
    uint32_t current_time = rtc_unixtime();
    EEPROM_ClearRainTotals(current_time);
    scv.aqs = false;
    (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
  }

  else if (s.startsWith("SETELEV:")) { // Pattern start of string aka 0 offset
    Output("DOA:SETELEV");
    String rest = s.substring(8);   // get part after "SETELEV:", 8 = length of 
    int elevation = rest.toInt();  // convert to integer

    if ((String(elevation) == rest) && 
        ((elevation == QC_ERR_ELEV) || ((elevation >= QC_MIN_ELEV)) && (elevation <= QC_MAX_ELEV))) {
      scv.elevation = elevation; // Set running value of elevation
      mslp_initialize(); // Set flags so we don't need to reboot.
      (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
    }
    else {
      Output("DOA:SETELEV, INVALID ELEV#");           
    }
  }

  else if (s.startsWith("SETRTRO:")) { // Pattern start of string aka 0 offset
    Output("DOA:SETRTRO");
    int hour = 0;
    int minute = 0;
    bool valid = false;

    String rest = s.substring(8); // get part after "SETRTRO:", 8 = length of 

    rest.trim(); // Trim whitespace

    int parsed = sscanf(rest.c_str(), "%d:%d", &hour, &minute);

    if (parsed == 2) {
      // H:MM format - validate quarter-hour
      if ((hour >= 0 && hour <= 23) && (minute == 0 || minute == 15 || minute == 30 || minute == 45)) {
        valid=true;
      }
    } 
    else if (parsed == 1) {
      // Just H format (minute = 0)
      hour = atoi(rest.c_str());
      if (hour >= 0 && hour <= 23) {
        valid=true;
      }
    } 

    // Save to SD and set config values
    if (valid) {
      scv.rtro_hour = hour;
      scv.rtro_minute = minute;
      (saveConfig()) ? Output ("NVSave:OK") : Output ("NVSave:ERR");
    } 
    else {
      sprintf(Buffer32Bytes, "DOA:SETRTRO:%d:%02d INVALID", hour, minute);
      Output (Buffer32Bytes);
    }
  }
  else {
    Output("DOA:UKN"); 
  }
}

/*
 * ======================================================================================================================
 * notecard_doa_ready() - Process up to 3 Notes from doaction.qi per call.
 * Each Note is deleted when retrieved, BEFORE its action is performed.
 * Budget roughly 50–200 ms per iteration want to stay under 1 second
 * ======================================================================================================================
 */
void notecard_doa_ready() {
  const unsigned int maxNotesPerCall = 3;

  for (unsigned int i = 0; i < maxNotesPerCall; ++i) {
    J *req = notecard.newRequest("note.get");
    if (req == NULL) {
      Output("DOA Check: Could not create request");
      return;
    }

    JAddStringToObject(req, "file", "doaction.qi");
    JAddBoolToObject(req, "delete", true); // Remove Note when retrieved

    J *rsp = notecard.requestAndResponse(req);
    if (rsp == NULL) {
      Output("DOA Check:No response from NC");
      return;
    }

    const char *err = JGetString(rsp, "err");

    if (err != NULL && err[0] != '\0') {
      // An empty inbound queue is normal, not a connection failure.
      if (strstr(err, "{note-noexist}") == NULL) {
        snprintf(msgbuf, sizeof(msgbuf), "DOA NC Error: %s", err);
        Output(msgbuf);
      }

      notecard.deleteResponse(rsp);
      return;
    }

    J *body = JGetObject(rsp, "body");

    if (body != NULL) {
      const char *value = JGetString(body, "DO");

      if (value != NULL && value[0] != '\0') {
        String action = String(value);
        notecard_doa_perform(action);
      }
      else {
        Output("DOA: Missing, empty, or non-string DO field");
      }
    }
    else {
      Output("DOA: No body in Note");
    }

    notecard.deleteResponse(rsp);
  }

  Output("DOA: 3-Note limit reached; will check again next call");
}

/*
 * ======================================================================================================================
 * notecard_doa_check() - Check for inbound do action note in queue doaction.qi. Delete note after transfer.
 * ======================================================================================================================
 */
void notecard_doa_check() {
  if (notecard_exists && scv.nc_doa && ((int32_t)(millis() - nc_doa_check_time) >= 0)) {
    Output("DOA:Check");
    notecard_doa_ready();
    nc_doa_check_time = millis() + (scv.nc_doa * 60 * 1000);
  }
}