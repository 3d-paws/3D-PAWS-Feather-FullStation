/*
 * ======================================================================================================================
 * feather.cpp - Feather Related Board Functions and Definations
 * ======================================================================================================================
 */
#include "include/support.h"
#include "include/output.h"
#include "include/sdcard.h"
#include "include/wifi.h"
#include "include/notecard.h"
#include "include/lorawan.h"
#include "include/main.h"
#include "include/feather.h"

char DeviceID[17];              // A generated ID based on board's 128-bit serial number converted down to 64bits

const char *board_config_tag[] = {"UNKN", "LG", "LGNC", "WF", "WFNC", "LW", "LWNC"};
BOARD_CONFIG board_config = bc_unkn;


const char* pinNames[] = {
  "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
  "D8", "D9", "D10", "D11", "D12", "D13",
  "A0", "A1", "A2", "A3", "A4", "A5"
};

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 *=======================================================================================================================
 * vbat_get() -- return battery voltage
 *=======================================================================================================================
 */
float vbat_get() {
  float v = analogRead(VBATPIN);
  v *= 2.0f;    // we divided by 2, so multiply back
  v *= 3.3f;  // Multiply by 3.3V, our reference voltage
  v /= 4096.0f; // convert to voltage
  return (v);
}

/*
 * ======================================================================================================================
 * GetDeviceID() - 
 * 
 * This function reads the 128-bit (16-byte) unique serial number from the 
 * SAMD21 microcontroller's flash memory, compresses it to 12 bytes, and 
 * converts it to a 24-character hexadecimal string.
 * 
 * The compression is done by XORing the first 8 bytes with the last 8 bytes 
 * of the original 16-byte identifier, ensuring all 128 bits contribute to 
 * the final result.
 * 
 * SAM D21/DA1 Family Datas Sheet
 * 10.3.3 Serial Number
 * Each device has a unique 128-bit serial number which is a concatenation of four 32-bit words contained at the
 * following addresses:
 * Word 0: 0x0080A00C
 * Word 1: 0x0080A040
 * Word 2: 0x0080A044
 * Word 3: 0x0080A048
 * The uniqueness of the serial number is guaranteed only when using all 128 bits.
 * ======================================================================================================================
 */
void GetDeviceID() {
  // Pointer to the unique 128-bit serial number in flash
  uint32_t *uniqueID = (uint32_t *)0x0080A00C;
  
  // Temporary buffer to store 16 bytes (128 bits)
  uint8_t fullId[16];
  
  // Extract all 16 bytes from the 128-bit serial number
  for (int i = 0; i < 4; i++) {
    uint32_t val = uniqueID[i];
    fullId[i*4] = (val >> 24) & 0xFF;
    fullId[i*4 + 1] = (val >> 16) & 0xFF;
    fullId[i*4 + 2] = (val >> 8) & 0xFF;
    fullId[i*4 + 3] = val & 0xFF;
  }

  // Compress 16 bytes to 8 bytes using XOR
  uint8_t compressedId[8];
  for (int i = 0; i < 8; i++) {
    compressedId[i] = fullId[i] ^ fullId[i + 8];
  }

  memset (DeviceID, 0, 17);
  for (int i = 0; i < 8; i++) {
    sprintf (DeviceID+strlen(DeviceID), "%02X", compressedId[i]);
  }
  DeviceID[16] = '\0'; // Ensure null-termination
}

/*
 * ======================================================================================================================
 * boardcfg_initialize() -             
 * ======================================================================================================================
 */
void boardcfg_initialize() {
  // Place Check here for NoteCard
  notecard_initialize(); // Sets "notecard_exists"  and initializes if found

  if (SD_ChipSelect==SD_ADALOGGER_CS) {
    // Adafruit Feather M0 Adalogger  - Microcontroller with SD on board
    if (notecard_exists) {
      board_config = bc_lg_nc;
      Output(F("BOARD:Logger w/NC"));
    }
    else {
      board_config = bc_lg;
      Output(F("BOARD:Logger"));
    }
  }
  else {
    // See what network solution we have
    WiFi.setPins(8,7,4,2);
    if (WiFi_Feather()) {
      if (notecard_exists) {
        board_config = bc_wf_nc;
        Output(F("BOARD:WiFi w/NC"));
      }
      else {
        board_config = bc_wf;
        Output(F("BOARD:WiFi"));
        WiFi_initialize();
        (WiFi_valid) ? Output(F("WiFi Valid")) : Output(F("WiFi NotValid"));
      }
    }
    else {
      // Defaulting to LoRaWAN Board type
      if (notecard_exists) {
        board_config = bc_lw_nc;
        Output(F("BOARD:LoRaWAN w/NC"));
      }
      else {
        // if LoRaWAN fails to initialize behave like a Feather Logger.
        board_config = bc_lw;
        Output(F("BOARD:LoRaWAN"));
        delay (10000);
        LW_initialize();
        if (!LW_valid) {
          Output(F("LW Disabled"));
          board_config = bc_lg;
        }
        else {
          board_config = bc_lw;
        }
        nextinfo = millis() + 60000; // Give LoRaWAN 1 minute time to Join if OTAA
      }
    }
  }
}
