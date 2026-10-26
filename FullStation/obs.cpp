/*
 * ======================================================================================================================
 *  obs.cpp - Observation Reporting Functions
 * ======================================================================================================================
 */
#include "include/ssbits.h"
#include "include/feather.h"
#include "include/eeprom.h"
#include "include/wifi.h"
#include "include/sensor_rgop.h"
#include "include/sensor_wind.h"
#include "include/nvcf.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/mux.h"
#include "include/dsmux.h"
#include "include/lorawan.h"
#include "include/support.h"
#include "include/time.h"
#include "include/sensor_bmp.h"
#include "include/sensor_sht.h"
#include "include/sensor_mcp.h"
#include "include/sensor_htu.h"
#include "include/sensor_pm25aqi.h"
#include "include/sensor_tlw.h"
#include "include/sensor_tsm.h"
#include "include/sensor_bh1750.h"
#include "include/sensor_ltr390.h"
#include "include/derived.h"
#include "include/main.h"
#include "include/notecard.h"
#include "include/obs.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
OBSERVATION_STR obs;
char obsbuf[MAX_OBS_SIZE];      // Url that holds observations for HTTP GET
char *obsp;                     // Pointer to obsbuf


/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * OBS_Clear() - Set OBS to not in use
 * ======================================================================================================================
 */
void OBS_Clear() {
  obs.inuse =false;
  for (int s=0; s<MAX_SENSORS; s++) {
    obs.sensor[s].inuse = false;
  }
}

/*
 * ======================================================================================================================
 * OBS_N2S_Add_WiFi() - Save OBS to N2S file
 * ======================================================================================================================
 */
void OBS_N2S_Add_WiFi() {
  if (obs.inuse) {     // Sanity check
    char ts[32];
   
    memset(obsbuf, 0, sizeof(obsbuf));

    // Modify System Status and Set From Need to Send file bit
    obs.hth |= SSB_FROM_N2S; // Turn On Bit

    tm *dt = gmtime(&obs.ts);
    sprintf (obsbuf, "%s?key=%s&instrument_id=%d&",
      scv.ls_urlpath.c_str(), scv.ls_apikey.c_str(), scv.ls_instr_id);
    sprintf (obsbuf+strlen(obsbuf), "devid=%s&at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
      DeviceID,
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
    sprintf (obsbuf+strlen(obsbuf), "&bv=%.2f",obs.bv); 
    sprintf (obsbuf+strlen(obsbuf), "&hth=%d", obs.hth);
   
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }
    Serial_writeln (obsbuf);
    SD_NeedToSend_Add(obsbuf); // Save to N2F File
    Output("OBS-> N2S");
  }
  else {
    Output("OBS->N2S OBS:Empty");
  }
}

/*
 * ======================================================================================================================
 * OBS_LOG_Add() - Create observation in obsbuf and save to SD card.
 * 
 * {"at":"2022-02-13T17:26:07","css":18,"hth":0,"bcs":2,"bpc":63.2695,.....,"mt2":20.5625}
 * ======================================================================================================================
 */
void OBS_LOG_Add() {
  Output("OBS_LOG_ADD()");
    
  if (obs.inuse) {     // Sanity check

    memset(obsbuf, 0, sizeof(obsbuf));

    // Save the Observation in JSON format
    
    sprintf (obsbuf, "{");

    tm *dt = gmtime(&obs.ts); 
    
    sprintf (obsbuf+strlen(obsbuf), "\"at\":\"%d-%02d-%02dT%02d:%02d:%02d\"",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
      
    sprintf (obsbuf+strlen(obsbuf), ",\"bv\":%.2f", obs.bv); 
    sprintf (obsbuf+strlen(obsbuf), ",\"hth\":%d", obs.hth);
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }
    sprintf (obsbuf+strlen(obsbuf), "}");
    
    Output("OBS->SD");
    Serial_writeln (obsbuf);
    SD_LogObservation(obsbuf); 
  }
  else {
    Output("OBS->SD OBS:Empty");
  }
}

/*
 * ======================================================================================================================
 * OBS_Take() - Take Observations - Should be called once a minute - fill data structure
 * ======================================================================================================================
 */
void OBS_Take() {
  int sidx = 0;
  
  Output("OBS_TAKE()");
  
  // Safty Check for Vaild Time
  if (!RTC_valid) {
    Output ("OBS_Take: TM Invalid");
    return;
  }
  
  OBS_Clear(); // Just do it again as a safty check

  Wind_GustUpdate(); // Update Gust and Gust Direction readings

  obs.inuse = true;
  // obs.ts = rtc_unixtime();
  obs.ts = (time_t) Time_of_obs;

  obs.bv = vbat_get();

  if (!scv.aqs) {
    // Rain Gauge 1 & 2
    raingauge_obs_do(sidx);

    // op1 and op2 observations - minus raingauge2
    op_obs_do(sidx);

    Wind_obs_do(sidx);
  }

  // Do AQ before others since we have a 30s wakeup time when we are in AQS mode
  pm25aqi_obs_do(sidx);

  // Report Any Bosch Sensors
  bmp_obs_do(sidx);

  // Report Any Sensirion Sensors
  sht_obs_do(sidx);

  // Report MicroChip mcp9808 Sensors
  mcp_obs_do(sidx);

  // Report HTU21DF Sensor
  htu_obs_do(sidx);

  // Report Tinovi Leaf Wetness Sensor
  tlw_obs_do(sidx);

  // Tinovi Soil Moisture
  mux_obs_do(sidx); // on the mux or single on main bus.

  // DFRobot SEN0562 I2C Ambient Light Sensor
  bh1750_obs_do(sidx);

  // Adafruit LTR390 UV Light Sensor
  ltr390_obs_do(sidx);

  // Derived Sensors
  derived_obs_do(sidx);

  // Dallas Sensors Temperature on mux
  dsmux_obs_do(sidx);

  // Set this after we read all sensors. So we capture if their state changes
  obs.hth = SystemStatusBits;

  Output("OBS_TAKE(DONE)");
}

/*
 * ======================================================================================================================
 * OBS_Build_WiFi() - Create observation in obsbuf from structure obs for sending
 * 
 * Example at=2022-05-17T17%3A40%3A04&hth=8770 .....
 * ======================================================================================================================
 */
bool OBS_Build_WiFi() {
  Output("OBSBLDWIFI()"); 
  if (obs.inuse) {     // Sanity check  
    memset(obsbuf, 0, sizeof(obsbuf));
    tm *dt = gmtime(&obs.ts);

    sprintf (obsbuf, "%s?key=%s&instrument_id=%d&devid=%s",
      scv.ls_urlpath.c_str(), scv.ls_apikey.c_str(), scv.ls_instr_id, DeviceID);

    sprintf (obsbuf+strlen(obsbuf), "&at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
    sprintf (obsbuf+strlen(obsbuf), "&bv=%.2f", obs.bv); 
    sprintf (obsbuf+strlen(obsbuf), "&hth=%d", 
      obs.hth);  

    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }

    Output("OBSBLDWIFI:OK");
    Serial_writeln (obsbuf);
    return (true);
  }
  else {
    Output("OBSBLDWIFI:NOTINUSE");
    return (false);
  }
}

/*
 * ======================================================================================================================
 * OBS_Send_WiFi() - From obs structure build and send observation via wifi
 * ======================================================================================================================
 */
void OBS_Send_WiFi() {
  if (OBS_Build_WiFi()) {  // Build Observation to Send in obsbuf
    // WiFi_Send() returns 0=not sent, -500=ErrorCode Not Sent, 1=Sent
    if (WiFi_Send(obsbuf, scv.ls_fqdn.c_str(), scv.ls_port, scv.ls_urlpath.c_str(), METHOD_GET, scv.ls_apikey.c_str()) != 1) {
      Output("FS->PUB FAILED");
      OBS_N2S_Add_WiFi();
      OBS_Clear();
    }
    else {
      Output("FS->PUB OK");
      OBS_Clear();
      // Check if we have any N2S only if we have not added to the file while trying to send OBS
      SD_N2S_Publish(); 
    }
  }
}

/*
 * ======================================================================================================================
 * OBS_Send_LoRaWAN() - From obs structure build and send 1 or more LoRaWAN packets as needed
 *                      The receiving side will need to use LoRaWAN deveui to identify device. 
 *                      We are not sending the devid in the observation. 
 * ======================================================================================================================
 */
void OBS_Send_LoRaWAN() {
  char header[32];
  char hthbits[32];
  char loramsg[256];
  char sensor[16];
  bool checkN2S=true;
  int sensor_payload;  // LORA_PAYLOAD Buffer Size

  Output("OBS_Send_LW)");
  if (obs.inuse) {                           // Sanity check set by OBS_Take()
    tm *dt = gmtime(&obs.ts);                // OBS_Take() has already obtained the time

    memset(header, 0, sizeof(header));       // Header is included in every lora message
    memset(loramsg, 0, sizeof(loramsg));
    memset(hthbits, 0, sizeof(hthbits));
    memset(obsbuf, 0, sizeof(obsbuf));       // This will be the lora payload we send

    // Consider adding health to header so we can set the from N2S bit.
    
    // Header is included in every lora message
    sprintf (header, "at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",         // 23 bytes
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
    sprintf (hthbits, "&hth=%d", obs.hth);                           // Call it 9 bytes

    sensor_payload = LORA_PAYLOAD - (strlen (header) + strlen (hthbits));

    sprintf (loramsg, "&bv=%.2f", obs.bv); 
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        // sprintf (Buffer32Bytes, "PROCESSOBS=%d", s);
        // Output(Buffer32Bytes); 
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (sensor, "&%s=%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (sensor, "&%s=%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (sensor, "&%s=%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
        
        // Will this Sensor observation fit into loramsg - This is how we do not overflow the 256 byte loramsg structure
        if ( (strlen(sensor) + strlen(loramsg)) <= sensor_payload) {
          // Add the sensor to the sensors
          sprintf (loramsg+strlen(loramsg), "%s", sensor);
        }
        else {       
          Output("OBS_Send_LW:SENDING");
          sprintf (obsbuf, "%s%s%s", header, hthbits, loramsg); // Put the parts together and send
          Output (obsbuf);
          if (LW_Send(obsbuf)) {
            Output("OBS_Send_LW:SENT");
          }
          else {
            Output("OBS_Send_LW:PUB FAILED");
            sprintf (hthbits, "&hth=%d", (obs.hth | SSB_FROM_N2S)); // Turn On Bit

            sprintf (obsbuf, "%s%s%s", header, hthbits, loramsg);
            Output (obsbuf);
            SD_NeedToSend_Add(obsbuf);
            checkN2S = false;
          }

          // Success or fail we continue on trying to send the rest of the observations

          delay(500); // Its Recommended before sending another message

          // Clear buffers and add the one that would not fit
          memset(loramsg, 0, sizeof(loramsg));
          memset(obsbuf, 0, sizeof(obsbuf));
          sprintf (loramsg+strlen(loramsg), "%s", sensor);
        }
      }
      else {
        // sprintf (Buffer32Bytes, "NOT INUSE=%d", s);
        // Output(Buffer32Bytes);          
      }
    } // for

    // There will always be something to send at this point
    Output("OBS_Send_LW:SENDING-LAST");
    sprintf (obsbuf, "%s%s%s", header, hthbits, loramsg); // Put the parts together and send
    Output (obsbuf);

    if (LW_Send(obsbuf)) {
      Output("OBS_Send_LW:SENT");
    }
    else {
      Output("OBS_Send_LW:PUB FAILED");
      sprintf (hthbits, "&hth=%d", (obs.hth | SSB_FROM_N2S)); // Turn On Bit

      sprintf (obsbuf, "%s%s%s", header, hthbits, loramsg);
      Output (obsbuf);
      SD_NeedToSend_Add(obsbuf);
      checkN2S = false;
    }
    
    OBS_Clear(); 

    if (checkN2S) {
      // Check if we have any N2S and if we have not added to the file while trying to send OBS, check
      SD_N2S_Publish();     
    }
  }
  else {
    Output("OBS_Send_LW:NOTINUSE");
  }
}

/*
 * ======================================================================================================================
 * OBS_Build_JSON() - Create observation in obsbuf for sending to Chords 
 *                    Example {"key":1234,"at":"2022-05-17T17:40:04","hth":0}
 * ======================================================================================================================
 */
bool OBS_Build_JSON() {   
  if (obs.inuse) {     // Sanity check  
    memset(obsbuf, 0, sizeof(obsbuf));
    tm *dt = gmtime(&obs.ts);

    // Save the Observation in JSON format
    
    sprintf (obsbuf, "{");
    sprintf (obsbuf+strlen(obsbuf),"\"devid\":\"%s\"", DeviceID);
    sprintf (obsbuf+strlen(obsbuf), ",\"at\":\"%d-%02d-%02dT%02d%:%02d%:%02d\"",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);

    sprintf (obsbuf+strlen(obsbuf), ",\"bv\":%.2f", obs.bv);
    sprintf (obsbuf+strlen(obsbuf), ",\"hth\":%d", obs.hth);
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output (F("WhyAmIHere?"));
            break;
        }
      }
    }
    sprintf (obsbuf+strlen(obsbuf), "}");

    Output(F("OBS->JSON"));
    Serial_writeln (obsbuf);
    return (true);
  }
  else {
    Output(F("OBS->JSON OBS:Empty"));
    return (false);
  }
}

/*
 * ======================================================================================================================
 * OBS_Send_NoteCard() - From obs structure build and send observation via notecard
 * 
 * Notecard buffers notes in its own flash whenever the network is unavailable and delivers them once connectivity 
 * returns, so no need for N2S support
 * ======================================================================================================================
 */
void OBS_Send_NoteCard() {
  Output("OBS->NC");
  if ((scv.nc_method == "c") || (scv.nc_method == "w")  || (scv.nc_method == "cw") || (scv.nc_method == "wc")) {
    // Queue Terrestrial Transmission, no template needed
    if (OBS_Build_JSON()) {
      // obsbuf has the JSON of observation to send.

      // NoteCard Debug
      // notecard_hubsync_status();  
      // notecard_wireless_status();
      // notecard_wifi_status();

      notecard_qtt_obs(obsbuf);
    }
  }
  else {
    // Queue Non Terrestrial Transmission, must use a template because we have a satellite or lorawan notecard option
    // notecard_qnt_obs(obsbuf);
  }

  OBS_Clear(); 
}

/*
 * ======================================================================================================================
 * OBS_Do() - Do Observation Processing, 222 bytes max for LoRaWAN
 * ======================================================================================================================
 */
void OBS_Do() {
  Output("OBS_DO()");

  OBS_Take();           // Take an observation, fill obs data structure
  OBS_LOG_Add();        // Save Observation Data to Log file.

  if (notecard_exists) {
    OBS_Send_NoteCard();
  }
  else if (LW_valid) {
    OBS_Send_LoRaWAN();
  }
  else if (WiFi_valid) {
    OBS_Send_WiFi();
  }
  else {
    Output("No Valid Network"); // We are just logging the observations
  }
}