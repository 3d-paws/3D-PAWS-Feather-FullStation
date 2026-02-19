/*
 * ======================================================================================================================
 * sdcard.cpp - SD Card
 * ======================================================================================================================
 */
#include <SdFat.h>

#include "include/ssbits.h"
#include "include/eeprom.h"
#include "include/time.h"
#include "include/main.h"
#include "include/cf.h"
#include "include/output.h"
#include "include/sensors.h"
#include "include/obs.h"
#include "include/lorawan.h"        // LoRaWAN Support Functions
#include "include/sdcard.h"

SdFat SD;                                   // File system object.
// SD;                                      // File system object defined by the SD.h include file.
File SD_fp;
char SD_obsdir[] = "/OBS";                  // Observations stored in this directory. Created at power on if not exist
bool SD_exists = false;                     // Set to true if SD card found at boot
char SD_n2s_file[] = "N2SOBS.TXT";          // Need To Send Observation file
uint32_t SD_n2s_max_filesz = 512 * 60 * 24; // Keep a little over 1 day. When it fills, it is deleted and we start over.
char SD_crt_file[] = "CRT.TXT";             // if file exists clear rain totals and delete file
char SD_OPTAQS_FILE[] ="OPTAQS.TXT";        // Enable Air Quality Station, Use OP2_PN to contril sensor

char SD_INFO_FILE[] = "INFO.TXT";       // Store INFO information in this file. Every INFO call will overwrite content

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 * =======================================================================================================================
 * SD_initialize()
 * =======================================================================================================================
 */
void SD_initialize() {

#if SD_MASK_INTERRUPS
  // Block LoRA DIO interrupts
  noInterrupts(); 
#endif 
  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_WIFI_SS, OUTPUT);
  digitalWrite(LORA_WIFI_SS, HIGH);
  
  // Make sure SD CS is set correctly
  pinMode(SD_ChipSelect, OUTPUT);
  digitalWrite(SD_ChipSelect, HIGH);

  // SdFat defaults to the board's max SPI rate (48 MHz on Feather M0), but many cards/cards+boards need a slower start.
  // So we will initialize with 12 MHz
  if (!SD.begin(SD_ChipSelect, SD_SCK_MHZ(12))) {
    Output ("SD:INIT ERR");
  }
  else {
    if (!SD.exists(SD_obsdir)) {
      if (SD.mkdir(SD_obsdir)) {
        Output ("SD:MKDIR OBS OK");
        Output ("SD:Online");
        SD_exists = true;
      }
      else {
        Output ("SD:MKDIR OBS ERR");
        Output ("SD:Offline");
        SystemStatusBits |= SSB_SD;  // Turn On Bit     
      } 
    }
    else {
      Output ("SD:Online");
      Output ("SD:OBS DIR Exists");
      SD_exists = true;
    }
  }
#if SD_MASK_INTERRUPS
  // Enable LoRA DIO interrupts
  interrupts();
#endif
}

/* 
 * =======================================================================================================================
 * SD_LogObservation() - Call rtc_timestamp() prior to set now variable
 * =======================================================================================================================
 */
void SD_LogObservation(char *observations) {
  char SD_logfile[24];
  File fp;
    
  if (!SD_exists) {
    Output ("SD:NOT EXIST");
    return;
  }

  if (!RTC_valid) {
    Output ("SD:RTC NOT VALID");
    return;
  }

  sprintf (SD_logfile, "%s/%4d%02d%02d.log", SD_obsdir, now.year(), now.month(), now.day());
  Output (SD_logfile);

#if SD_MASK_INTERRUPS
  // Block LoRA DIO interrupts
  noInterrupts();
#endif
  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_WIFI_SS, OUTPUT);
  digitalWrite(LORA_WIFI_SS, HIGH);
     
  fp = SD.open(SD_logfile, FILE_WRITE); 
  if (fp) {
    fp.println(observations);
    fp.close();
    SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
    Output ("OBS Logged to SD");
  }
  else {
    SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
    Output ("SD:Open(Log)ERR");
    // At thins point we could set SD_exists to false and/or set a status bit to report it
    // sd_initialize();  // Reports SD NOT Found. Library bug with SD
  }
#if SD_MASK_INTERRUPS
  // Enable LoRA DIO interrupts
  interrupts();
#endif  
}

/* 
 * =======================================================================================================================
 * SD_N2S_Delete()
 * =======================================================================================================================
 */
bool SD_N2S_Delete() {
  bool result;

#if SD_MASK_INTERRUPS
  // Block LoRA DIO / WiFi interrupts
  noInterrupts();
#endif
  // Disable LoRA / WiFi SPI0 Chip Select
  pinMode(LORA_WIFI_SS, OUTPUT);
  digitalWrite(LORA_WIFI_SS, HIGH);
  
  if (SD_exists && SD.exists(SD_n2s_file)) {
    if (SD.remove (SD_n2s_file)) {
      SystemStatusBits &= ~SSB_N2S; // Turn Off Bit
      Output ("N2S->DEL:OK");
      result = true;
    }
    else {
      Output ("N2S->DEL:ERR");
      SystemStatusBits |= SSB_SD; // Turn On Bit
      result = false;
    }
  }
  else {
    Output ("N2S->DEL:NF");
    result = true;
  }

  eeprom.n2sfp = 0;
  EEPROM_Update();
#if SD_MASK_INTERRUPS
  // Enable LoRA DIO / WiFi interrupts
  interrupts();  
#endif  
  return (result);
}

/* 
 * =======================================================================================================================
 * SD_NeedToSend_Add()
 * =======================================================================================================================
 */
void SD_NeedToSend_Add(char *observation) {
  File fp;

  if (!SD_exists) {
    return;
  }

#if SD_MASK_INTERRUPS
  // Block LoRA DIO / WiFi interrupts
  noInterrupts();
#endif
  // Disable LoRA / WiFi SPI0 Chip Select
  pinMode(LORA_WIFI_SS, OUTPUT);
  digitalWrite(LORA_WIFI_SS, HIGH);
  
  fp = SD.open(SD_n2s_file, FILE_WRITE); // Open the file for reading and writing, starting at the end of the file.
                                         // It will be created if it doesn't already exist.
  if (fp) {  
    if (fp.size() > SD_n2s_max_filesz) {
      fp.close();
      Output ("N2S:Full");
      if (SD_N2S_Delete()) {
        // Only call ourself again if we truely deleted the file. Otherwise infinate loop.
        SD_NeedToSend_Add(observation); // Now go and log the data
      }
    }
    else {
      fp.println(observation); //Print data, followed by a carriage return and newline, to the File
      fp.close();
      SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
      SystemStatusBits |= SSB_N2S; // Turn on Bit that says there are entries in the N2S File
      Output ("N2S:OBS Added");
    }
  }
  else {
    SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
    Output ("N2S:Open Error");
    // At thins point we could set SD_exists to false and/or set a status bit to report it
    // sd_initialize();  // Reports SD NOT Found. Library bug with SD
  }
#if SD_MASK_INTERRUPS
  // Enable LoRA DIO / WiFi interrupts
  interrupts();
#endif
}

/* 
 * =======================================================================================================================
 * SD_NeedToSend_Status() - Send back the state 
 * =======================================================================================================================
 */
void SD_NeedToSend_Status(char *status) {

  if (SD_exists) {
#if SD_MASK_INTERRUPS
    // Block LoRA DIO / WiFi interrupts
    noInterrupts();
#endif 
    // Disable LoRA / WiFi SPI0 Chip Select
    pinMode(LORA_WIFI_SS, OUTPUT);
    digitalWrite(LORA_WIFI_SS, HIGH);

    if (SD.exists(SD_n2s_file)) {
      File fp = SD.open(SD_n2s_file, FILE_WRITE);
      if (fp) {
        sprintf (status, "%d", fp.size());
        fp.close();
      }
      else {
        sprintf (status, "-1");
      }
    }
    else {
      sprintf (status, "\"NF\"");
    }
#if SD_MASK_INTERRUPS
    // Enable LoRA DIO / WiFi interrupts
    interrupts(); 
#endif
  }
  else {
    sprintf (status, "\"!SD\"");
  }
}

/* 
 *=======================================================================================================================
 * SD_ClearRainTotals() -- If CRT.TXT exists on SD card clear rain totals - Checked at Boot
 *=======================================================================================================================
 */
void SD_ClearRainTotals() {
  if (RTC_valid && SD_exists) {
    if (SD.exists(SD_crt_file)) {
      if (SD.remove (SD_crt_file)) {
        Output (F("CRT:OK-CLR"));
        uint32_t current_time = rtc_unixtime();
        EEPROM_ClearRainTotals(current_time);
        EEPROM_Dump();
      }
      else {
        Output (F("CRT:ERR-RM"));
      }
    }
    else {
      Output (F("CRT:OK-NF"));
    }
  }
  else {
    Output (F("CRT:ERR-CLK"));
  }
}

/* 
 * =======================================================================================================================
 * SD_UpdateInfoFile() - Update INFO.TXT file 
 * =======================================================================================================================
 */
void SD_UpdateInfoFile(char *info) {
  // Update INFO.TXT file
  if (SD_exists) {
#if SD_MASK_INTERRUPS
    // Block LoRA DIO / WiFi interrupts
    noInterrupts(); 
#endif 
    // Disable LoRA / WiFi SPI0 Chip Select
    pinMode(LORA_WIFI_SS, OUTPUT);
    digitalWrite(LORA_WIFI_SS, HIGH);

    File fp = SD.open(SD_INFO_FILE, FILE_WRITE | O_TRUNC); 
    if (fp) {
      fp.println(info);
      fp.close();
      SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
      Output ("INFO->SD OK");
      // Output ("INFO Logged to SD");
    }
    else {
      SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
      Output ("SD:Open(Info)ERR");
    }
#if SD_MASK_INTERRUPS
    // Enable LoRA DIO / WiFi interrupts
    interrupts();
#endif 
  }
  else {
    Output ("INFO->!SD");
  }
}

/* 
 *=======================================================================================================================
 * SD_N2S_Publish()
 *=======================================================================================================================
 */
void SD_N2S_Publish() {
  File fp;
  char ch;
  int i;
  int sent=0;

  memset(obsbuf, 0, sizeof(obsbuf));

  Output ("N2S Publish");

  if (SD_exists) {
#if SD_MASK_INTERRUPS
    // Block LoRA DIO / Wifi interrupts
    noInterrupts();
#endif 
    // Disable LoRA / Wifi SPI0 Chip Select
    pinMode(LORA_WIFI_SS, OUTPUT);
    digitalWrite(LORA_WIFI_SS, HIGH);
  
    if (SD.exists(SD_n2s_file)) {
      Output ("N2S:Exists");

      fp = SD.open(SD_n2s_file, FILE_READ); // Open the file for reading, starting at the beginning of the file.

      if (fp) {
        // Delete Empty File or too small of file to be valid
        if (fp.size()<=20) {
          fp.close();

          Output ("N2S:Empty");
          SD_N2S_Delete();
        }
        else {
          // If no eeprom, then we can still use the variable and start from the start of the file and hope.
          // if we loose power before we empty the file, we will have to start all over and retransmit.
          if (eeprom.n2sfp) {
            if (fp.size()<=eeprom.n2sfp) {
              // Something wrong. Can not have a file position that is larger than the file
              eeprom.n2sfp = 0; 
            }
            else {
              fp.seek(eeprom.n2sfp);  // Seek to where we left off last time. 
            }
          } 

          // Loop through each line / obs and transmit
        
          // set timer on when we need to stop sending n2s obs

          uint64_t TimeFromNow;

          if (LW_valid && (cf_obs_period < 10)) {
            // Since we have to delay between LoRaWAN transmissions we are going to forget about
            // stopping before the next observation period and just send N2S for 5 minutes
            TimeFromNow = millis() + (60 * 5 * 1000); // 5 minutes of sending
          }
          else {
            TimeFromNow = time_to_next_obs() - 45000; // stop sending before next observations period
          }

          i = 0;
          while (fp.available() && (i < MAX_MSGBUF_SIZE )) {
            ch = fp.read();

            if (ch == 0x0A) {  // newline
#if SD_MASK_INTERRUPS
              // Enable LoRA DIO / WiFi interrupts
              interrupts();
#endif

              if (LW_valid) {
                LW_WaitOnTXRXPEND(60); // Probably not needed sind we have a 30s Wait Gap before the LoRa Send
                                       // It returns right away if TXRXPEND has been cleared
              }

              int send_result = SendMsg(obsbuf);
#if SD_MASK_INTERRUPS
              // Block LoRA DIO / Wifi interrupts
              noInterrupts();
#endif
              // Disable LoRA / Wifi SPI0 Chip Select
              pinMode(LORA_WIFI_SS, OUTPUT);
              digitalWrite(LORA_WIFI_SS, HIGH);

              if (send_result == 1) { 
                sprintf (Buffer32Bytes, "N2S[%d]->PUB:OK", sent++);
                Output (Buffer32Bytes);
                Serial_writeln (obsbuf);

                // setup for next line in file
                i = 0;

                // file position is at the start of the next observation or at eof
                // eeprom might not exist, but we can still use the variable to store our offset in.
                eeprom.n2sfp = fp.position();

                BackGroundWork(); // os_runloop_once() will be called but will only do housekeeping and 
                                  // no SPI and no radio_irq_handler() calls

                sprintf (Buffer32Bytes, "N2S[%d] Continue", sent);
                Output (Buffer32Bytes); 

                if(millis() > TimeFromNow) {
                  // need to break out so new obs can be made
                  Output ("N2S->TIME2EXIT");
                  break;                
                }
              }
            
              else if (send_result == -500) { // HTTP/1.1 500 Internal Server Error (Applies to WiFi Only)
                // Suspect we have a bad N2S observation that webserver does not like, move past it.
                sprintf (Buffer32Bytes, "N2S[%d]->ERR:500", sent++);
                Output (Buffer32Bytes);
                Serial_writeln (obsbuf);

                // setup for next line in file
                i = 0;

                // file position is at the start of the next observation or at eof
                // eeprom might not exist, but we can still use the variable to store our offset in.
                eeprom.n2sfp = fp.position();
              
                sprintf (Buffer32Bytes, "N2S[%d] Continue", sent);
                Output (Buffer32Bytes); 

                if(millis() > TimeFromNow) {
                  // need to break out so new obs can be made
                  Output ("N2S->TIME2EXIT");
                  break;                
                }             
              }
              else {
                sprintf (Buffer32Bytes, "N2S[%d]->PUB:ERR", sent);
                Output (Buffer32Bytes);
                // On transmit failure, stop processing file.
                break;
              }
            
            // At this point file pointer's position is at the first character of the next line or at eof
            
            } // Newline
            else if (ch == 0x0D) { // CR, LF follows and will trigger the line to be processed       
             obsbuf[i] = 0; // null terminate then wait for newline to be read to process OBS
            }
            else {
              obsbuf[i++] = ch;
            }

            // Check for Buffer OverRun
            if (i >= MAX_MSGBUF_SIZE) {
              sprintf (Buffer32Bytes, "N2S[%d]->BOR:ERR", sent);
              Output (Buffer32Bytes);
              fp.close();
#if SD_MASK_INTERRUPS
              // Enable LoRA DIO / WiFi interrupts
              interrupts();
#endif
              SD_N2S_Delete(); // Bad data in the file so delete the file           
              return;
            }
          } // end while 

          if (fp.available() <= 20) {
            // If at EOF or some invalid amount left then delete the file
            fp.close();
#if SD_MASK_INTERRUPS
            // Enable LoRA DIO / WiFi interrupts
            interrupts();
#endif
            SD_N2S_Delete();
          }
          else {
            // At this point we sent 0 or more observations but there was a problem.
            // eeprom.n2sfp was maintained in the above read loop. So we will close the
            // file and next time this function is called we will seek to eeprom.n2sfp
            // and start processing from there forward. 
            fp.close();
#if SD_MASK_INTERRUPS          
            // Enable LoRA DIO / WiFi interrupts
            interrupts();
#endif
            EEPROM_Update(); // Update file postion in the eeprom.
          }
        }
      }
      else {
        Output ("N2S->OPEN:ERR");
      }
    }
#if SD_MASK_INTERRUPS
    // Enable LoRA DIO / WiFi interrupts
    interrupts();
#endif
  }
  else {
    Output ("N2S->!SD");
  }
  Output ("N2S Done");
}
