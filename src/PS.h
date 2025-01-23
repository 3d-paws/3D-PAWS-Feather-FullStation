/*
 * ======================================================================================================================
 * PS.h - Particle Support Funtions
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * Output_CellBatteryInfo() - On OLED display station information
 * ======================================================================================================================
 */
void Output_CellBatteryInfo() {
#if PLATFORM_ID == PLATFORM_ARGON
  WiFiSignal sig = WiFi.RSSI();
  float SignalStrength = sig.getStrength();

  sprintf (Buffer32Bytes, "CS:%d.%02d", 
    (int)SignalStrength, (int)(SignalStrength*100)%100);
  Output(Buffer32Bytes);
#else
  CellularSignal sig = Cellular.RSSI();
  float SignalStrength = sig.getStrength();
  
  int BatteryState = System.batteryState();
  float BatteryPoC = 0.0;                 // Battery Percent of Charge

  // Read battery charge information only if battery is connected. 
  if (BatteryState>0 && BatteryState<6) {
    BatteryPoC = System.batteryCharge();
  }
  
  sprintf (Buffer32Bytes, "CS:%d.%02d B:%d,%d.%02d", 
    (int)SignalStrength, (int)(SignalStrength*100)%100,
    BatteryState, (int)BatteryPoC, (int)(BatteryPoC*100)%100);
  Output(Buffer32Bytes);
#endif
}

/*
 * ======================================================================================================================
 * DeviceReset() - Kill power to ourselves and do a cold boot
 * ======================================================================================================================
 */
void DeviceReset() {
  digitalWrite(REBOOT_PIN, HIGH);
  delay(5000);
  // Should not get here if relay / watchdog is connected.
  digitalWrite(REBOOT_PIN, LOW);
  delay(2000); 

  // May never get here if relay board / watchdog not connected.

  // Resets the device, just like hitting the reset button or powering down and back up.
  System.reset();
}

/*
 * ======================================================================================================================
 * Function_DoAction() - Handler for Particle Function DoAction     
 * ======================================================================================================================
 */
int Function_DoAction(String s) {
  if (strcmp (s,"REBOOT") == 0) {  // Reboot
    Output("DoAction:REBOOT");
    EEPROM_SaveUnreportedRain();
    delay(1000);

    DeviceReset();

    // Never gets here and thus can never send a ack to Particle Portal
    return(0);  
  }

  else if (strcmp (s,"INFO") == 0) {  // Send System Information
    Output("DoAction:INFO");
    SendSystemInformation=true;
    return(0);  
  }

  else if (strcmp (s,"SEND") == 0) {  // Send OBS Now
    Output("DoAction:SEND");
    LastTransmitTime=0;
    return(0);  
  }

  else if (strcmp (s,"CRT") == 0) { // Clear Rain Totals
    time32_t current_time = Time.now();
    Output("DoAction:CRT");
    EEPROM_ClearRainTotals(current_time);
    // Display EEPROM Information 
    // EEPROM_Dump();   
    return(0);
  }

  else if (strcmp (s,"A4DIST") == 0) { // Set A4 State File to Distance
    Output("DoAction:A4DIST");
    if (SD_exists) {
      if (SD.exists(SD_A4_RAIN_FILE)) {
        EEPROM_ClearRain2Totals();
        if (SD.remove (SD_A4_RAIN_FILE)) {
          Output ("A4=DIST, DEL RAIN:OK");
        }
        else {
          Output ("A4=DIST, DEL RAIN:ERR");
          return(-2);
        }
      }

      if (SD.exists(SD_A4_DIST_FILE)) {
        Output ("A4=DIST, ALREADY EXISTS");    
      }
      else {
        // Touch File
        File fp = SD.open(SD_A4_DIST_FILE, FILE_WRITE);
        if (fp) {
          fp.close();
          Output ("A4=DIST, SET");
        }
        else {
          Output ("A4=DIST, OPEN ERR");
          return(-3);
        }
      }

      // Set Distance Sensor Divisor to that of 10m
      dg_adjustment = 2.5;
      if (SD.exists(SD_5M_DIST_FILE)) {
        if (SD.remove (SD_5M_DIST_FILE)) {
          Output ("A4=DIST, DEL 5M:OK, 10M SET");
        }
        else {
          Output ("A4=DIST, DEL 5M:ERR");
          return(-4);
        }
      }
      else {
        Output ("A4=DIST, 10M");
      }
    }
    else {
      Output("A4=DIST, SD NF"); 
      return(-1);      
    }
 
    return(0);
  }

  else if (strcmp (s,"A4RAIN") == 0) { // Set A4 State File to Rain
    Output("DoAction:A4RAIN");
    if (SD_exists) {
      if (SD.exists(SD_A4_DIST_FILE)) {
        if (SD.remove (SD_A4_DIST_FILE)) {
          Output ("A4=RAIN, DEL DIST:OK");
        }
        else {
          Output ("A4=RAIN, DEL DIST:ERR");
          return(-2);
        }
      }
      if (SD.exists(SD_5M_DIST_FILE)) {
        if (SD.remove (SD_5M_DIST_FILE)) {
          Output ("A4=RAIN, DEL 5M:OK");
        }
        else {
          Output ("A4=RAIN, DEL 5M:ERR");
          return(-4);
        }
      }

      if (SD.exists(SD_A4_RAIN_FILE)) {
        Output ("A4=RAIN, ALREADY EXISTS");      
      }
      else {
        EEPROM_ClearRain2Totals(); // Just a good thing to do.
        // Touch File
        File fp = SD.open(SD_A4_RAIN_FILE, FILE_WRITE);
        if (fp) {
          fp.close();
          Output ("A4=RAIN, SET");
        }
        else {
          Output ("A4=RAIN, OPEN ERR");
          return(-3);
        }
      }
    }
    else {
      Output("A4=RAIN, SD NF"); 
      return(-1);      
    }
    return(0);
  }

  else if (strcmp (s,"5MDIST") == 0) { // Set 5M Distance Sensor State File
    Output("DoAction:5MDIST");
    if (SD_exists) {
      if (SD.exists(SD_5M_DIST_FILE)) {
        Output ("5MDIST, ALREADY EXISTS");      
      }
      else {
        // Touch File
        File fp = SD.open(SD_5M_DIST_FILE, FILE_WRITE);
        if (fp) {
          fp.close();
          dg_adjustment = 1.25;
          Output ("5MDIST SET");
        }
        else {
          Output ("5MDIST OPEN ERR");
          return(-5);
        }
      }
    }
    else {
      Output("5MDIST, SD NF"); 
      return(-1);      
    }
    return(0);
  }

  else if (strcmp (s,"A4RAW") == 0) { // Set A4 State File to Raw
    Output("DoAction:A4RAW");
    if (SD_exists) {

      // Remove Rain Configuration
      if (SD.exists(SD_A4_RAIN_FILE)) {
        EEPROM_ClearRain2Totals();
        if (SD.remove (SD_A4_RAIN_FILE)) {
          Output ("A4=DIST, DEL RAIN:OK");
        }
        else {
          Output ("A4=DIST, DEL RAIN:ERR");
          return(-2);
        }
      }

      // Remove Distance Configuration
      if (SD.exists(SD_A4_DIST_FILE)) {
        if (SD.remove (SD_A4_DIST_FILE)) {
          Output ("A4=DIST, DEL DIST:OK");
        }
        else {
          Output ("A4=DIST, DEL DIST:ERR");
          return(-3);
        }
      }

      // Remove Distanve sensor type and reset distance adjustment to default 10m
      dg_adjustment = 2.5;
      if (SD.exists(SD_5M_DIST_FILE)) {
        if (SD.remove (SD_5M_DIST_FILE)) {
          Output ("A4=DIST, DEL 5M:OK");
        }
        else {
          Output ("A4=DIST, DEL 5M:ERR");
          return(-4);
        }
      }

      // Add A4 Raw configuration
      if (SD.exists(SD_A4_RAW_FILE)) {
        Output ("A4=RAW, ALREADY EXISTS");    
      }
      else {
        // Touch File
        File fp = SD.open(SD_A4_RAW_FILE, FILE_WRITE);
        if (fp) {
          fp.close();
          Output ("A4=RAW, SET");
        }
        else {
          Output ("A4=RAW, OPEN ERR");
          return(-5);
        }
      }
    }
    else {
      Output("A4=RAW, SD NF"); 
      return(-1);      
    }
    return(0);
  }

  else if (strcmp (s,"A4CLR") == 0) { // Clear A4 State Files
    int state=0;
    Output("DoAction:A4CLR");
    if (SD_exists) {
      if (SD.exists(SD_A4_DIST_FILE)) {
        if (SD.remove (SD_A4_DIST_FILE)) {
          A4_State = A4_STATE_NULL;
          Output ("A4=CLR, DEL DIST:OK");
        }
        else {
          Output ("A4=CLR, DEL DIST:ERR");
          state=-2;
        }
      }
      else {
        Output ("A4=CLR, DEL RAIN:NF");
      }

      if (SD.exists(SD_A4_RAIN_FILE)) {
        if (SD.remove (SD_A4_RAIN_FILE)) {
          A4_State = A4_STATE_NULL;      // We still need a reboot to get rid of ISR
          Output ("A4=CLR, DEL RAIN:OK");
        }
        else {
          Output ("A4=CLR, DEL RAIN:ERR");
          state+=-3; // returns a -3 if also failed removing DIST file
        }
      }
      else {
        Output ("A4=CLR, DEL RAIN:NF");
      }

      if (SD.exists(SD_A4_RAW_FILE)) {
        if (SD.remove (SD_A4_RAW_FILE)) {
          A4_State = A4_STATE_NULL;
          Output ("A4=CLR, DEL RAW:OK");
        }
        else {
          Output ("A4=CLR, DEL RAW:ERR");
          state+=-4; // returns a -4 if also failed removing RAW file
        }
      }
      else {
        Output ("A4=CLR, DEL RAIN:NF");
      }

      if (SD.exists(SD_5M_DIST_FILE)) {
        if (SD.remove (SD_5M_DIST_FILE)) {
          Output ("A4=CLR, DEL 5M:OK");
          dg_adjustment = 2.5;
        }
        else {
          Output ("A4=CLR, DEL 5M:ERR");
          state+=-5;
        }
      }
    }
    else {
      Output("A4=CLR, SD NF"); 
      state=-1;     
    }
    return(state);
  }

  else if (strcmp (s,"A5RAW") == 0) { // Set A4 State File to Raw
    Output("DoAction:A5RAW");
    // Add A5 Raw configuration
    if (SD_exists) {
      if (SD.exists(SD_A5_RAW_FILE)) {
        Output ("A5=RAW, ALREADY EXISTS");    
      }
      else {
        // Touch File
        File fp = SD.open(SD_A5_RAW_FILE, FILE_WRITE);
        if (fp) {
          fp.close();
          Output ("A5=RAW, SET");
        }
        else {
          Output ("A5=RAW, OPEN ERR");
          return(-2);
        }
      }
    }
    else {
      Output("A5=RAW, SD NF"); 
      return(-1);      
    }
    return(0);
  }

    
  else if (strcmp (s,"A5CLR") == 0) { // Clear A5 State Files
    int state=0;
    Output("DoAction:A5CLR");
    if (SD_exists) {
      if (SD.exists(SD_A5_RAW_FILE)) {
        if (SD.remove (SD_A4_RAW_FILE)) {
          A5_State = A5_STATE_NULL;
          Output ("A5=CLR, DEL RAW:OK");
        }
        else {
          Output ("A5=CLR, DEL RAW:ERR");
          state=-2;
        }
      }
      else {
        Output ("A5=CLR, DEL A5RAW:NF");
      }
    }
    else {
      Output("A5=CLR, SD NF"); 
      state=-1;     
    }
    return(state);
}

  else if (strcmp (s,"TXI5M") == 0) { // SetTransmit Interval to 5 Minutes
    Output("DoAction:TXI5M");
    if (SD_exists) {
      if (SD.exists(SD_TX5M_FILE)) {
        Output ("TXI5M, ALREADY SET"); 
        obs_tx_interval = 5;     
      }
      else {
        // Touch File
        File fp = SD.open(SD_TX5M_FILE, FILE_WRITE);
        if (fp) {
          fp.close();
          obs_tx_interval = 5;
          Output ("TXI5M SET");
        }
        else {
          Output ("TXI5M OPEN ERR");
          return(-2);
        }
      }

      if (SD.exists(SD_TX10M_FILE)) {
        if (SD.remove (SD_TX10M_FILE)) {
          Output ("TXI5M: Removed TX10M File");
        }
      }
    }
    else {
      Output("TXI5M, SD NF"); 
      return(-1);      
    }
    return(0);
  }

  else if (strcmp (s,"TXI10M") == 0) { // SetTransmit Interval to 10 Minutes
    Output("DoAction:TXI10M");
    if (SD_exists) {
      if (SD.exists(SD_TX10M_FILE)) {
        Output ("TXI10M, ALREADY SET"); 
        obs_tx_interval = 10;     
      }
      else {
        // Touch File
        File fp = SD.open(SD_TX10M_FILE, FILE_WRITE);
        if (fp) {
          fp.close();
          obs_tx_interval = 10;
          Output ("TXI10M SET");
        }
        else {
          Output ("TXI10M OPEN ERR");
          return(-2);
        }
      }

      if (SD.exists(SD_TX5M_FILE)) {
        if (SD.remove (SD_TX5M_FILE)) {
          Output ("TXI5M: Removed TX5M File");
        }
      }
    }
    else {
      Output("TXI10M, SD NF"); 
      return(-1);      
    }
    return(0);
  }

  else if (strcmp (s,"TXI15M") == 0) { // SetTransmit Interval to 15 Minutes
    Output("DoAction:TXI15M");
    if (SD_exists) {
      if (!SD.exists(SD_TX5M_FILE) && !SD.exists(SD_TX10M_FILE)) {
        Output ("TXI15M, ALREADY SET"); 
        obs_tx_interval = 15;     
      }
      else {
        // Remove Files
        if (SD.exists(SD_TX5M_FILE)) {
          if (SD.remove (SD_TX5M_FILE)) {
            Output ("TXI15M: Removed TX5M File");
          }
        }
        if (SD.exists(SD_TX10M_FILE)) {
          if (SD.remove (SD_TX10M_FILE)) {
            Output ("TXI15M: Removed TX10M File");
          }
        }      
        obs_tx_interval = 15;
        Output ("TXI15M SET"); 
      }
    }
    else {
      Output("TXI15M, SD NF"); 
      return(-1);      
    }
    return(0);
  }

  else {
    Output("DoAction:UKN"); 
    return(-1);
  }
}

#if PLATFORM_ID == PLATFORM_ARGON
/*
 * ======================================================================================================================
 * WiFiChangeCheck() - Check for WIFI.TXT file and set Wireless SSID, Password            
 * ======================================================================================================================
 */
void WiFiChangeCheck() {
  File fp;
  int i=0;
  char *p, *auth, *ssid, *pw;
  char ch, buf[128];
  bool changed = false;

  if (SD_exists) {
    // Test for file WIFI.TXT
    if (SD.exists(SD_wifi_file)) {
      fp = SD.open(SD_wifi_file, FILE_READ); // Open the file for reading, starting at the beginning of the file.

      if (fp) {
        // Deal with too small or too big of file
        if (fp.size()<=7 || fp.size()>127) {
          fp.close();
          Output ("WIFI:Invalid SZ");
        }
        else {
          Output ("WIFI:Open");
          // Read one line from file
          while (fp.available() && (i < 127 )) {
            ch = fp.read();

            // sprintf (msgbuf, "%02X : %c", ch, ch);
            // Output (msgbuf);

            if ((ch == 0x0A) || (ch == 0x0D) ) {  // newline or linefeed
              break;
            }
            else {
              buf[i++] = ch;
            }
          }
          fp.close();

          // At this point we have encountered EOF, CR, or LF
          // Now we need to terminate array with a null to make it a string
          buf[i] = (char) NULL;

          // Parse string for the following
          //   WIFI ssid password
          p = &buf[0];
          auth = strtok_r(p, ",", &p);

          if (auth == NULL) {
            Output("WIFI:ID=Null Err");
          }
          else if ( (strcmp (auth, "WEP") != 0)  &&
                    (strcmp (auth, "WPA") != 0)  &&
                    (strcmp (auth, "WPA2") != 0) &&
                    (strcmp (auth, "UNSEC") != 0)) {
            sprintf (msgbuf, "WIFI:ATYPE[%s] Err", auth);          
            Output(msgbuf);
          }
          else {
            ssid = strtok_r(p, ",", &p);
            pw  = strtok_r(p, ",", &p);
            
            if (pw == NULL) {
              pw = (char *) "";  // Handle the case when nothing is after the ","
            }

            if (ssid == NULL) {
              Output("WIFI:SSID=Null Err");
            }

            // UNSEC is allow to have no password just a ssid
            else if ((strcmp (auth, "UNSEC") != 0)  && (pw == NULL)) {
              Output("WIFI:PW=Null Err");
            }
            else {
              sprintf (msgbuf, "WIFI:ATYPE[%s]", auth);          
              Output(msgbuf);
              sprintf (msgbuf, "WIFI:SSID[%s]", ssid);
              Output(msgbuf);
              sprintf (msgbuf, "WIFI:PW[%s]", pw);
              Output(msgbuf);

              // Connects to a network secured with WPA2 credentials.
              // https://docs.particle.io/reference/device-os/api/wifi/securitytype-enum/
              if (strcmp (auth, "UNSEC") == 0) {
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                if (strcmp (pw, "") == 0) {
                  Output("WIFI:Credentials Set UNSEC NO PW");
                  WiFi.setCredentials(ssid);
                }
                else {
                  Output("WIFI:Credentials Set UNSEC");
                  WiFi.setCredentials(ssid, pw);                 
                }
              }
              else if (strcmp (auth, "WEP") == 0) {
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WEP");
                WiFi.setCredentials(ssid, pw, WEP);
              }
              else if (strcmp (auth, "WPA") == 0) {
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WPA");
                WiFi.setCredentials(ssid, pw, WPA);
              }
              else if (strcmp (auth, "WPA2") == 0) {
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WPA2");
                WiFi.setCredentials(ssid, pw, WPA2);
              }
              else if (strcmp (auth, "WPA_ENTERPRISE") == 0) {
                // WPA Enterprise is only supported on the Photon and P1.
                // It is not supported on the Argon, P2, and Photon 2.
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WPAE");
                WiFi.setCredentials(ssid, pw, WPA_ENTERPRISE);
              }
              else if (strcmp (auth, "WPA2_ENTERPRISE") == 0) {
                // WPA Enterprise is only supported on the Photon and P1.
                // It is not supported on the Argon, P2, and Photon 2.
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WPAE2");
                WiFi.setCredentials(ssid, pw, WPA2_ENTERPRISE);
              }
              else { 
                Output("WIFI:Credentials NOT Set");
                Output("WIFI:USING NVAUTH");
              }
            }
          }
        }
      }
      else {
        sprintf (msgbuf, "WIFI:Open[%s] Err", SD_wifi_file);          
        Output(msgbuf);
        Output ("WIFI:USING NVAUTH");
      }
    } 
    else {
      Output ("WIFI:NOFILE USING NVAUTH");
    }
  } // SD enabled
  else {
    Output ("WIFI:NOSD USING NVAUTH");
  }
}

/*
 * ======================================================================================================================
 * WiFiPrintCredentials() - Read NVRAM and print WiFi Creditials     
 * 
 * Particle's Device OS does not provide a direct method to determine which of the stored Wi-Fi credentials is currently 
 * being used for the connection. However, you can infer this indirectly by attempting to connect to each stored Wi-Fi 
 * network and checking if the connection is successful.        
 * ======================================================================================================================
 */
void WiFiPrintCredentials() {
  byte mac[6];

  WiFi.macAddress(mac);


  sprintf (msgbuf, "WIFI MAC[%02x:%02x:%02x:%02x:%02x:%02x]", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Output(msgbuf);

  WiFiAccessPoint ap[5];
  WiFi.getCredentials(ap, 5);
  int found = 0;

  for (int i = 0; i < 5; i++) {
    if (strlen(ap[i].ssid) != 0) {
      found++;
      if (found == 1) {
        Output("WIFI Creds:");
      }
      sprintf (msgbuf, "IDX[%d]", i);
      Output(msgbuf);
      sprintf (msgbuf, " SSID[%s]", ap[i].ssid);
      Output(msgbuf);
      switch (ap[i].security) {
        case WLAN_SEC_UNSEC:
          Output(" Type:Unsecured");
          break;
        case WLAN_SEC_WEP:
          Output(" Type:WEP");
          break;
        case WLAN_SEC_WPA:
          Output(" Type:WPA");
          break;
        case WLAN_SEC_WPA2:
          Output(" Type:WPA2");
          break;
        case WLAN_SEC_WPA_ENTERPRISE:
          Output(" Type:WPA Ent");
          break;
        case WLAN_SEC_WPA2_ENTERPRISE:
          Output(" Type:WPA2 Ent");
          break;
        default:
          Output(" Type:Unknown");
          break;
      }
      sprintf (msgbuf, " SigStr[%d]", ap[i].rssi);
      Output(msgbuf);
      sprintf (msgbuf, " Channel[%d]", ap[i].channel);
      Output(msgbuf);
      sprintf (msgbuf, " BMAC[%02x:%02x:%02x:%02x:%02x:%02x]", 
        ap[i].bssid[0], ap[i].bssid[1], ap[i].bssid[2], ap[i].bssid[3], ap[i].bssid[4], ap[i].bssid[5]);
      Output(msgbuf);
    }
  }
  if (!found) {
    Output("WIFI Creds:NF");
  }
}
#else
/*
 * ======================================================================================================================
 * SimChangeCheck() - Check for SIM.TXT file and set sim based on contents, after rename file to SIMOLD.TXT            
 * ======================================================================================================================
 */
void SimChangeCheck() {
  File fp;
  int i=0;
  char *p, *id, *apn, *un, *pw;
  char ch, buf[128];
  bool changed = false;

  SimType simType = Cellular.getActiveSim();

  if (simType == 1) {
    Output ("SIM:Internal");
  } else if (simType == 2) {
    Output ("SIM:External");
  }
  else {
    sprintf (msgbuf, "SIM:Unknown[%d]", simType);
    Output (msgbuf);
  }

  if (SerialConsoleEnabled && SD_exists) {
    // Test for file SIM.TXT
    if (SD.exists(SD_sim_file)) {
      fp = SD.open(SD_sim_file, FILE_READ); // Open the file for reading, starting at the beginning of the file.

      if (fp) {
        // Deal with too small or too big of file
        if (fp.size()<=7 || fp.size()>127) {
          fp.close();
          Output ("SIMF:Invalid SZ");
          if (SD.remove (SD_sim_file)) {
            Output ("SIMF->Del:OK");
          }
          else {
            Output ("SIMF->Del:Err");
          }
        }
        else {
          Output ("SIMF:Open");
          while (fp.available() && (i < 127 )) {
            ch = fp.read();

            if ((ch == 0x0A) || (ch == 0x0D) ) {  // newline or linefeed
              break;
            }
            else {
              buf[i++] = ch;
            }
          }

          // At this point we have encountered EOF, CR, or LF
          // Now we need to terminate array with a null to make it a string
          buf[i] = (char) NULL;

          // Parse string for the following
          //   INTERNAL
          //   AUP epc.tmobile.com username passwd
          //   UP username password
          //   APN epc.tmobile.com
          p = &buf[0];
          id = strtok_r(p, " ", &p);

          if (id != NULL) {
            sprintf (msgbuf, "SIMF:ID[%s]", id);
            Output(msgbuf);
          }

          if (strcmp (id,"INTERNAL") == 0) {
            Cellular.setActiveSim(INTERNAL_SIM);
            Cellular.clearCredentials();
            changed = true;
          }
          else if (strcmp (id,"APN") == 0) {
            apn = strtok_r(p, " ", &p);

            if (apn == NULL) {
              Output("SIMF:APN=Null Err");
            }
            else {
              Cellular.setActiveSim(EXTERNAL_SIM);
              Output("SIM:Set External-APN");

              // Connects to a cellular network by APN only
              Cellular.setCredentials(apn);
              Output("SIM:Set Credentials");
              sprintf (msgbuf, " APN[%s]", apn);
              Output(msgbuf);
              changed = true;
            }
          }
          else if (strcmp (id," UP") == 0) {
            un  = strtok_r(p, " ", &p);
            pw  = strtok_r(p, " ", &p);

            if (un == NULL) {
              Output("SIMF:Username=Null Err");
            }
            else if (pw == NULL) {
              Output("SIMF:Passwd=Null Err");
            }
            else {
              Cellular.setActiveSim(EXTERNAL_SIM);
              Output("SIM:Set External-UP");

              // Connects to a cellular network with USERNAME and PASSWORD only
              Cellular.setCredentials(un,pw);
              Output("SIM:Set Credentials");
              sprintf (msgbuf, " UN[%s]", un);
              Output(msgbuf);
              sprintf (msgbuf, " PW[%s]", pw);
              Output(msgbuf);
              changed = true;
            }
          }
          else if (strcmp (id,"AUP") == 0) {
            apn = strtok_r(p, " ", &p);
            un  = strtok_r(p, " ", &p);
            pw  = strtok_r(p, " ", &p);

            if (apn == NULL) {
              Output("SIMF:APN=Null Err");
            }
            else if (un == NULL) {
              Output("SIMF:Username=Null Err");
            }
            else if (pw == NULL) {
              Output("SIMF:Passwd=Null Err");
            }
            else {
              Cellular.setActiveSim(EXTERNAL_SIM);
              Output("SIM:Set External-AUP");

              // Connects to a cellular network with a specified APN, USERNAME and PASSWORD
              Cellular.setCredentials(apn,un,pw);
              Output("SIM:Set Credentials");
              sprintf (msgbuf, " APN[%s]", apn);
              Output(msgbuf);
              sprintf (msgbuf, "  UN[%s]", un);
              Output(msgbuf);
              sprintf (msgbuf, "  PW[%s]", pw);
              Output(msgbuf);
              changed = true;
            }
          }
          else {  // Pasrse Error
            sprintf (msgbuf, "SIMF:ID[%s] Err", id);
            Output(msgbuf);
          }
        }

        // No matter what happened with the above, rename file so we don't process again at boot
        // if SIMOLD.TXT exists then remove it before we rename SIM.TXT
        if (SD.exists(SD_simold_file)) {
          if (SD.remove (SD_simold_file)) {
            Output ("SIMF:DEL SIMOLD");
          }
        }

        if (!fp.rename(SD_simold_file)) {
          Output ("SIMF:RENAME ERROR");
        }
        else {
          Output ("SIMF:RENAME OK");
        }
        fp.close();

        // Notify user to reboot blink led forever
        if (changed) {
          Output ("==============");
          Output ("!!! REBOOT !!!");
          Output ("==============");

        }
        else {
          Output ("=====================");
          Output ("!!! SET SIM ERROR !!!");
          Output ("=====================");
        }
        
        while(true) { // wait for Host to open serial port
          Blink(1, 750);
        }
      }
      else {
        Output ("SIMF:OPEN ERROR");
      }
    } 
    else {
      Output ("SIM:NO UPDATE FILE");
    }
  } // Console and SD enabled
}
#endif

#if PLATFORM_ID == PLATFORM_BORON
/*
 * ======================================================================================================================
 * callback_imsi() - Callback for International Mobile Subscriber Identity 
 * 
 * COMMAND: AT+CIMI   Note: Each line returned is a call to this callback function
 * SEE https://docs.particle.io/reference/device-os/api/cellular/command/
 * ======================================================================================================================
 */
int callback_imsi(int type, const char* buf, int len, char* cimi) {
  // sprintf (msgbuf, "AT+CIMI:%X [%s]", type, buf); Output (msgbuf);     

  if ((type == TYPE_UNKNOWN) && cimi) {
    if (sscanf(buf, "\r\n%[^\r]\r\n", cimi) == 1)
      /*nothing*/;
  }

  if (type == TYPE_OK) {
    return (RESP_OK);
  }
  return (WAIT);
}
#endif

/* 
 *=======================================================================================================================
 * TXI_Initialize() - Transmit Interval 5, 10 or 15 minutes
 *=======================================================================================================================
 */
void TXI_Initialize() {
  Output ("TXI:INIT");
  if (SD_exists) {
    if (SD.exists(SD_TX5M_FILE)) {
      Output ("TXI5M Found");
      obs_tx_interval = 5;
      if (SD.exists(SD_TX10M_FILE)) {
        if (SD.remove (SD_TX10M_FILE)) {
          Output ("TXI:RM 10M");
        }
      }
    }
    else if (SD.exists(SD_TX10M_FILE)) {
      Output ("TXI10M Found");
      obs_tx_interval = 10;
      if (SD.exists(SD_TX5M_FILE)) {
        if (SD.remove (SD_TX5M_FILE)) {
          Output ("TXI:RM 5M");
        }
      }
    }
  }
  sprintf (msgbuf, "TXI=%dM", (int) obs_tx_interval);
  Output(msgbuf);  
}