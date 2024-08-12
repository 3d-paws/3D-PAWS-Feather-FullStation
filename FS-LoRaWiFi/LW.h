/*
 * ======================================================================================================================
 *  LW.h - LoRaWAN Functions
 *  
 *  Setting Country Frequencies
 *  
 *  MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h
 *  
 * // project-specific definitions
 * //#define CFG_eu868 1
 * #define CFG_us915 1
 * //#define CFG_au915 1
 * //#define CFG_as923 1
 * // #define LMIC_COUNTRY_CODE LMIC_COUNTRY_CODE_JP    // for as923-JP; also define CFG_as923
 * //#define CFG_kr920 1
 * //#define CFG_in866 1
 * #define CFG_sx1276_radio 1
 * //#define LMIC_USE_INTERRUPTS
 * 
 *  Compile for EU Frequencies 
 *    cd MCCI_LoRaWAN_LMIC_library/project_config
 *    cp lmic_project_config.h-eu lmic_project_config.h
 *  
 *  Compile for US Frequencies 
 *    cd MCCI_LoRaWAN_LMIC_library/project_config
 *    cp lmic_project_config.h-us lmic_project_config.h
 *    
 * ======================================================================================================================
 * Modified Library - SEE https://github.com/mcci-catena/arduino-lmic?tab=readme-ov-file
 * ======================================================================================================================
 * LoRa radio is being polled, no need to mask interrupts. That would cause the loss of rain and wind speed interrupts. 
 * Comment out interupts in the code file MCCI_LoRaWAN_LMIC_library/src/hal/hal.cpp
 * 
 * void hal_disableIRQs () {
 *    //noInterrupts();    "./src/hal/hal.cpp" line 340  COMMENT OUT
 *   irqlevel++;
 * }
 *
 * void hal_enableIRQs () {
 *   if(--irqlevel == 0) {
 *   //   interrupts();    "./src/hal/hal.cpp" line 346  COMMENT OUT
 * ======================================================================================================================
 */

void BackGroundWork(); // Prototype this function to aviod compile function unknown issue.

#define LORA_OTAA  0
#define LORA_ABP   1

#define LORA_SS  8      // We need to set this pin high to disable LoRa, prior to accessing the SD card.

bool LW_valid = false;
 
// Pin mapping for Adafruit Feather M0 LoRa
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,              // LBT cal for the Adafruit Feather M0 LoRa, in dB
    .spi_freq = 8000000,
};


// LoRaWAN parameters (TTN) ABP
uint32_t DEV_ADDR;
uint8_t  NWK_SKEY[16];
uint8_t  APP_SKEY[16];


// LoRaWAN parameters (TTN) OTA
uint8_t  APP_EUI[8];   // This EUI must be in little-endian format, so least-significant-byte first. Aka flip the order
uint8_t  DEV_EUI[8];   // This EUI must be in little-endian format, so least-significant-byte first. Aka flip the order
uint8_t  APP_KEY[16];  // This key should be in big endian format. The key  can be copied from the TTN as-is.

// Below Functions Required by LoRa Library
void os_getArtEui (u1_t* buf) { 
  if (cf_lw_mode == LORA_OTAA) {
    memcpy_P(buf, APP_EUI, 8);
  }
}

void os_getDevEui (u1_t* buf) { 
  if (cf_lw_mode == LORA_OTAA) {
    memcpy_P(buf, DEV_EUI, 8);
  }
}

void os_getDevKey (u1_t* buf) {
  if (cf_lw_mode == LORA_OTAA) {
    memcpy_P(buf, APP_KEY, 16);
  }
}


/*
 * ======================================================================================================================
 * onEvent() - 
 * ======================================================================================================================
 */
void onEvent (ev_t ev) {
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Output("LW:EV_SCAN_TIMEOUT");
            break;
        case EV_BEACON_FOUND:
            Output("LW:EV_BEACON_FOUND");
            break;
        case EV_BEACON_MISSED:
            Output("LW:EV_BEACON_MISSED");
            break;
        case EV_BEACON_TRACKED:
            Output("LW:EV_BEACON_TRACKED");
            break;
        case EV_JOINING:
            Output("LW:EV_JOINING");
            break;
        case EV_JOINED:
            Output("LW:EV_JOINED");
            if (cf_lw_mode == LORA_OTAA) {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
          
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              sprintf(msgbuf, "NET_ID %d", netid); Output (msgbuf);
              sprintf(msgbuf, "DEV_ADDR %X", devaddr); Output (msgbuf);
              Output ("NWK_KEY"); for (int i=0; i<16; i++) { sprintf(msgbuf+(i*2), "%02X", nwkKey[i]); } Output (msgbuf);
              Output ("APP_KEY"); for (int i=0; i<16; i++) { sprintf(msgbuf+(i*2), "%02X", artKey[i]); } Output (msgbuf);
              // Disable link check validation (automatically enabled during join), 
              // but because slow data rates change max TX size, we don't use it.
              LMIC_setLinkCheckMode(0);
            }
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Output("LW:EV_JOIN_FAILED");
            break;
        case EV_REJOIN_FAILED:
            Output("LW:EV_REJOIN_FAILED");
            break;
        case EV_TXCOMPLETE:
            sprintf (msgbuf, "LW:EV_TXCMPLT,%d recv", LMIC.dataLen);
            Output (msgbuf);
            if (LMIC.txrxFlags & TXRX_ACK) {
              Output("LW:Received ack");
            }
            /*
            if (LMIC.dataLen == 4) {
              uint32_t receivedTime = LMIC.frame[LMIC.dataBeg] |
                                      (LMIC.frame[LMIC.dataBeg + 1] << 8) |
                                      (LMIC.frame[LMIC.dataBeg + 2] << 16) |
                                      (LMIC.frame[LMIC.dataBeg + 3] << 24);
              sprintf (msgbuf, "LW:Received Time: %d", receivedTime);
              Output (msgbuf);
            }
            */
            break;
        case EV_LOST_TSYNC:
            Output("LW:EV_LOST_TSYNC");
            break;
        case EV_RESET:
            Output("LW:EV_RESET");
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Output("LW:EV_RXCOMPLETE");
            break;
        case EV_LINK_DEAD:
            Output("LW:EV_LINK_DEAD");
            break;
        case EV_LINK_ALIVE:
            Output("LW:EV_LINK_ALIVE");
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Output("LW:EV_TXSTART");
            break;
        case EV_TXCANCELED:
            Output("LW:EV_TXCANCELED");
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Output("LW:EV_JOIN_TXCOMPLETE: No JoinAccept");
            break;
        default:
            sprintf (msgbuf, "LW:EV_UNKNOWN: %d", (unsigned) ev);
            Output (msgbuf);
            break;
    }
}

/*
 * ======================================================================================================================
 * LW_Send()
 * ======================================================================================================================
 */
bool LW_Send(char *obs) {
  if (LMIC.opmode & OP_TXRXPEND) {
    
    uint64_t TimeFromNow = millis() + 10000;
    Output("LW:RetryWait");
    while(TimeFromNow > millis()) {
      if (LMIC.opmode & OP_TXRXPEND) {
        BackGroundWork();
      }
      else {
        break;
      }
    }
    
    Output("LW:Retry");    
    if (LMIC.opmode & OP_TXRXPEND) {
      Output("LW:Busy, OBS NOT Sent");
      return (false);
    }
    else {
      Output("LW:OBS Queuing");
      LMIC_setTxData2(1, (uint8_t*)obs, strlen(obs), 0);
      Output("LW:OBS Queued");
      return(true);        
    }
  } else {
    // prepare upstream data transmission at the next possible time.
    // transmit on port 1 (the first parameter); you can use any value from 1 to 223 (others are reserved).
    // don't request an ack (the last parameter, if not zero, requests an ack from the network).
    // Remember, acks consume a lot of network resources; don't ask for an ack unless you really need it.
    Output("LW:OBS Queuing");
    LMIC_setTxData2(1, (uint8_t*)obs, strlen(obs), 0);
    Output("LW:OBS Queued");
    return(true);
  }
}

/* 
 *=======================================================================================================================
 * LW_initialize()
 *=======================================================================================================================
 */
void LW_initialize() {
  Output ("LW:INIT");

  if (cf_lw_mode == LORA_OTAA) {
    Output ("LW:MODE OTAA");
    
    if (!isValidHexString(cf_lw_appeui, 16)) {
       Output("cf_lw_appeui - Invalid");
    }
    else {
      hexStringToByteArray(cf_lw_appeui, APP_EUI, 16);
      Output ("APP_EUI"); for (int i=0; i<8; i++) { sprintf(msgbuf+(i*2), "%02X", APP_EUI[i]); } Output (msgbuf);
    }

    if (!isValidHexString(cf_lw_deveui, 16)) {
       Output("cf_lw_deveui - Invalid");
    }
    else {
      hexStringToByteArray(cf_lw_deveui, DEV_EUI, 16);
      Output ("DEV_EUI"); for (int i=0; i<8; i++) { sprintf(msgbuf+(i*2), "%02X", DEV_EUI[i]); } Output (msgbuf);
    }

    if (!isValidHexString(cf_lw_appkey, 32)) {
       Output("cf_lw_appkey - Invalid");
    }
    else {
      hexStringToByteArray(cf_lw_appkey, APP_KEY, 32);
      Output ("APP_KEY"); for (int i=0; i<16; i++) { sprintf(msgbuf+(i*2), "%02X", APP_KEY[i]); } Output (msgbuf);
    }
  }
  else {
    Output ("LW:MODE ABP");
    
    if (!hexStringToUint32(cf_lw_devaddr, &DEV_ADDR)) {
      Output("lw_devaddr - Invalid");
    }
    // sprintf(msgbuf, "DEV_ADDR %X", DEV_ADDR); Output (msgbuf);

    if (!isValidHexString(cf_lw_nwkskey, 32)) {
       Output("lw_nwkskey - Invalid");
    }
    else {
      hexStringToByteArray(cf_lw_nwkskey, NWK_SKEY, 32);
      // Output ("NWK_SKEY"); for (int i=0; i<16; i++) { sprintf(msgbuf+(i*2), "%02X", NWK_SKEY[i]); } Output (msgbuf);
    }

    if (!isValidHexString(cf_lw_appskey, 32)) {
      Output("lw_appskey - Invalid");
    }
    else {
      hexStringToByteArray(cf_lw_appskey, APP_SKEY, 32);
      // Output ("APP_SKEY"); for (int i=0; i<16; i++) { sprintf(msgbuf+(i*2), "%02X", APP_SKEY[i]); } Output (msgbuf);
    }
  }
 
  os_init();
  LMIC_reset();

  if (cf_lw_mode == LORA_ABP) {
    // Used For ABP Setup
    LMIC_setSession(0x1, DEV_ADDR, NWK_SKEY, APP_SKEY); // Set up LoRaWAN using ABP

    // TTN uses SF9 for its RX2 downlink window. - was after check mode
    LMIC.dn2Dr = DR_SF9;
  }

  LMIC_setLinkCheckMode(0); //Set No Acks 


  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7,14);

  // For the below defines see: MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h
  #if defined(CFG_eu868)
    Output ("LW:868MHz");
    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set. The LMIC doesn't let you change
    // the three basic settings, but we show them here.
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    // Remember to add the below freqs on the device configuration at TTN
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    
    // TTN defines an additional channel at 869.525Mhz using SF9 for class B
    // devices' ping slots. LMIC does not have an easy way to define set this
    // frequency and support for class B is spotty and untested, so this
    // frequency is not configured here.
    
  #elif defined(CFG_us915) || defined(CFG_au915)
    Output ("LW:915MHz");
    // NA-US and AU channels 0-71 are configured automatically
    // but only one group of 8 should (a subband) should be active
    // TTN recommends the second sub band, 1 in a zero based count.
    // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
    LMIC_selectSubBand(1);
    
  #elif defined(CFG_as923)
    Output ("LW:923MHz");
    // Set up the channels used in your country. Only two are defined by default,
    // and they cannot be changed.  Use BAND_CENTI to indicate 1% duty cycle.
    // LMIC_setupChannel(0, 923200000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    // LMIC_setupChannel(1, 923400000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);

    // ... extra definitions for channels 2..n here
    
  #elif defined(CFG_kr920)
    Output ("LW:920MHz");
    // Set up the channels used in your country. Three are defined by default,
    // and they cannot be changed. Duty cycle doesn't matter, but is conventionally
    // BAND_MILLI.
    // LMIC_setupChannel(0, 922100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);
    // LMIC_setupChannel(1, 922300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);
    // LMIC_setupChannel(2, 922500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);

    // ... extra definitions for channels 3..n here.
    
  #elif defined(CFG_in866)
    Output ("LW:866MHz");
    // Set up the channels used in your country. Three are defined by default,
    // and they cannot be changed. Duty cycle doesn't matter, but is conventionally
    // BAND_MILLI.
    // LMIC_setupChannel(0, 865062500, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);
    // LMIC_setupChannel(1, 865402500, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);
    // LMIC_setupChannel(2, 865985000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);

    // ... extra definitions for channels 3..n here.
    
  #else
    Output ("LW:ERR Freq");
    # error Region not supported
  #endif

  LW_valid = true;
 }
