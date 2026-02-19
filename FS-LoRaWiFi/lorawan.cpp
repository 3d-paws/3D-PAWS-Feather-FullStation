/*
 * ======================================================================================================================
 *  lorawan.cpp - LoRaWAN Functions
 * ======================================================================================================================
 */
#include "include/support.h"
#include "include/output.h"
#include "include/cf.h"
#include "include/main.h"
#include "include/lorawan.h"

/*
 * ======================================================================================================================
 *  Define Global Configuration File Variables
 * ======================================================================================================================
 */

// The static lmic_pinmap is defined at compile time and used by LMIC's global initialization 
// code before your setup(). So we have to use #define to 

// Pin mapping for Adafruit Feather M0 LoRa
const lmic_pinmap lmic_pins = {
    .nss = LORA_WIFI_SS,             // LORA SPI Slave Select - We set this pin high to disable LoRa, prior to accessing the SD card
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,              // LBT cal for the Adafruit Feather M0 LoRa, in dB
    .spi_freq = 8000000,
};

bool LW_valid = false;
unsigned long LW_LastTransmitTime = 0;

// LoRaWAN parameters (TTN) ABP
uint32_t DEV_ADDR;
uint8_t  NWK_SKEY[16];
uint8_t  APP_SKEY[16];

// LoRaWAN parameters (TTN) OTA
uint8_t  APP_EUI[8];   // This EUI must be in little-endian format, so least-significant-byte first. Aka flip the order
uint8_t  DEV_EUI[8];   // This EUI must be in little-endian format, so least-significant-byte first. Aka flip the order
uint8_t  APP_KEY[16];  // This key should be in big endian format. The key  can be copied from the TTN as-is.

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

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
 * LW_Display_OTAA_SessionKeys()
 * ======================================================================================================================
 */
void LW_Display_OTAA_SessionKeys() {
  u4_t netid = 0;
  devaddr_t devaddr = 0;
  u1_t nwkKey[16];
  u1_t artKey[16];

  LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);

  sprintf(msgbuf, "NETID:%d", netid); Output (msgbuf);
  sprintf(msgbuf, "DEVADDR:%X", devaddr); Output (msgbuf);

  Output ("NWKKEY"); 
  for (int i=0; i<16; i++) { 
    sprintf(msgbuf+(i*2), "%02X", nwkKey[i]); 
  } 
  Output (msgbuf);

  Output ("APPSKEY"); 
  for (int i=0; i<16; i++) { 
    sprintf(msgbuf+(i*2), "%02X", artKey[i]); 
  } 
  Output (msgbuf);
}

/*
 * ======================================================================================================================
 * LW_isJoined()
 * ======================================================================================================================
 */
bool LW_isJoined() {
  if (cf_lw_mode == LORA_OTAA) {
    if (LMIC.netid) {
      return (true);
    }
    else {
      return (false);
    }
  }
  else {
    return (true); // ABP
  }
}

/*
 * ======================================================================================================================
 * LW_Joining()
 * ======================================================================================================================
 */
bool LW_Joining() {
  if ((LMIC.netid == 0) && (LMIC.opmode & OP_JOINING)) {
    return (true);
  }
  else {
    return (false);
  }
}

/*
 * ======================================================================================================================
 * LW_NeedToJoin()
 * ======================================================================================================================
 */
bool LW_NeedToJoin() {
  if ((LMIC.netid == 0) && !(LMIC.opmode & OP_JOINING)) {
    return (true);
  }
  else {
    return (false);
  }
}

/*
 * ======================================================================================================================
 * onEvent() - Called by LoRa Library
 * 
 * The library declares onEvent as a weak external function that developers must define in their sketch to handle LoRaWAN 
 * events like EV_JOINED or EV_TXCOMPLETE. LMIC calls this callback during runtime events via the scheduler.
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
        case EV_JOINED: // ABP mode will never receive a join
            Output("LW:EV_JOINED - WE ARE ONLINE");
            LW_Display_OTAA_SessionKeys();
            LMIC_setLinkCheckMode(0);
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
 * LW_WaitGap() - force a wait period between messages
 * ======================================================================================================================
 */
void LW_WaitGap() {

  while ((LW_LastTransmitTime + (LORA_TRANSMIT_GAP * 1000)) > millis()) {
    BackGroundWork(); // Calls os_runloop_once() which will set EV_TXCOMPLETE when it arrives, clearing OP_TXRXPEND
  }
}

/*
 * ======================================================================================================================
 * LW_WaitBetween() - wait between messages
 * ======================================================================================================================
 */
void LW_WaitBetween(int wait_seconds) {
  uint64_t TimeFromNow = millis() + (wait_seconds *1000);

  sprintf (Buffer32Bytes, "LW:WaitBTWN(%ds)", wait_seconds);
  Output(Buffer32Bytes);

  while(TimeFromNow > millis()) {
    BackGroundWork(); // Calls os_runloop_once() which will set EV_TXCOMPLETE when it arrives, clearing OP_TXRXPEND
  }
}

/*
 * ======================================================================================================================
 * LW_WaitOnTXRXPEND() - wait a while and see if the pending message clears, return true if cleared
 * ======================================================================================================================
 */
bool LW_WaitOnTXRXPEND(int wait_seconds) {
  uint64_t TimeFromNow = millis() + (wait_seconds *1000);

  sprintf (Buffer32Bytes, "LW:WaitOnTX(%ds)", wait_seconds);
  Output(Buffer32Bytes);

  while(TimeFromNow > millis()) {
    if (LMIC.opmode & OP_TXRXPEND) {
      BackGroundWork(); // Calls os_runloop_once() which will set EV_TXCOMPLETE when it arrives, clearing OP_TXRXPEND
    }
    else {
      Output("LW:TX NOT BUSY");
      return (true);
    }
  }
  Output("LW:TX STILL BUSY");
  return(false);
}

/*
 * ======================================================================================================================
 * LW_Send()
 * ======================================================================================================================
 */
bool LW_Send(char *msg) {

  Output("LW_Send()");

  if (!LW_isJoined()) {
    Output("LW:NotJoined");
    return (false);
  }

  LW_WaitGap();

  if (LMIC.opmode & OP_TXRXPEND) {
    // The below wait and retry might not be needed since adding LW_WaitGap()

    // Do not queue a new transmission while the previous one is still pending
    // If this is true all the time, it usually indicates LMIC never got to EV_TXCOMPLETE 
    // (radio wiring, DIO pins, timing, or join issues), so the busy flag never clears.

    // Lets wait a while and see if the pending message clears
    LW_WaitOnTXRXPEND(10); 
    
    Output("LW:Retry");    
    if (LMIC.opmode & OP_TXRXPEND) {
      Output("LW:Busy, MSG NOT Sent");
      return (false);
    }
  }

  // We only get here if OP_TXRXPEND is not set.

  // Prepare upstream data transmission at the next possible time.
  // Transmit on port 1 (the first parameter); you can use any value from 1 to 223 (others are reserved).
  // Don't request an ack (the last parameter, if not zero, requests an ack from the network).
  // Remember, acks consume a lot of network resources; don't ask for an ack unless you really need it.
  Output("LW:MSG Queuing");
  LMIC_setTxData2(1, (uint8_t*)msg, strlen(msg), 0);
  Output("LW:MSG Queued");

  // Using the time of the actual transmission, i.e., when the packet is queued and sent, 
  // not when EV_TXCOMPLETE fires. EV_TXCOMPLETE only occurs after the uplink plus both RX 
  // windows have finished, so using that would add extra seconds that are not airtime and 
  // will distort your duty‑cycle / fair‑use spacing.
  LW_LastTransmitTime = millis(); // Set Last Transmit Time, aids in forcing a time gap between transmissions
  return(true);
}

/* 
 *=======================================================================================================================
 * LW_Region()
 *=======================================================================================================================
 */
const char* LW_Region() {
#ifdef CFG_eu868
  return "EU868";
#elif defined(CFG_us915)
  return "US915"; 
#elif defined(CFG_au915)
  return "AU915";
#elif defined(CFG_as923)
  return "AS923";
#elif defined(CFG_kr920)
  return "KR920";
#elif defined(CFG_in866)
  return "IN866";
#else
  return "UNKNOWN";
#endif
}

/* 
 *=======================================================================================================================
 * LW_Show_FreqPlan()
 *=======================================================================================================================
 */
void LW_Show_FreqPlan() {
  int maxCh;

  if (!LW_valid) {
    return;
  }
  
  sprintf(Buffer32Bytes, "LW %s Channels:", LW_Region());
  Output(Buffer32Bytes);

#if defined(CFG_eu868) || defined(CFG_in866) || defined(CFG_kr920)
  // Classic LMIC EU868: SINGLE u2_t channelMap
  maxCh = 16;
  for (int i = 0; i < maxCh; i++) {
    if (LMIC.channelMap & (1U << i)) {  // Direct bit test
#elif defined(CFG_us915) || defined(CFG_au915) || defined(CFG_as923)
  // MCCI LMIC US915: ARRAY u2_t channelMap[]
  maxCh = 72 + MAX_XCHANNELS;
  for (int i = 0; i < maxCh; i++) {
    if (LMIC.channelMap[i >> 4] & (1U << (i & 15))) {  // Array bitmask
#else
  Output("Unknown region");
  return;
#endif

      // Common frequency formatting (both regions)
      uint32_t freq100Hz = LMIC.channelFreq[i];  // Hz * 100
      uint16_t mhz = freq100Hz / 1000000;        // MHz (902)
      uint16_t khz = (freq100Hz / 10000) % 100;  // kHz fraction (30)

      sprintf (Buffer32Bytes, "Ch %d: %u.%02u", i, mhz, khz);  // "Ch 0: 902.30"
      Output (Buffer32Bytes);
    }
  }

  // LMIC.txpow is determined dynamically in functions like updateTx() based on band settings, 
  // regulatory limits, and frequency plan. It often starts at per-band defaults
  sprintf(Buffer32Bytes, "TX Power (dBm): %d", LMIC.txpow);
  Output (Buffer32Bytes); // Current plan's power
}

/* 
 *=======================================================================================================================
 * LW_initialize()
 *=======================================================================================================================
 */
void LW_initialize() {
  Output ("LW:INIT");
  bool invalid=false;

  if (cf_lw_mode == LORA_OTAA) {
    Output ("LW:MODE OTAA");
    
    if (!isValidHexString(cf_lw_joineui, 16)) {
       Output("cf_lw_joineui - Invalid");
       invalid=true;
    }
    else {
      hexStringToByteArray(cf_lw_joineui, APP_EUI, 16);
      Output ("APP_EUI"); for (int i=0; i<8; i++) { sprintf(msgbuf+(i*2), "%02X", APP_EUI[i]); } Output (msgbuf);
    }

    if (!isValidHexString(cf_lw_deveui, 16)) {
       Output("cf_lw_deveui - Invalid");
       invalid=true;
    }
    else {
      hexStringToByteArray(cf_lw_deveui, DEV_EUI, 16);
      Output ("DEV_EUI"); for (int i=0; i<8; i++) { sprintf(msgbuf+(i*2), "%02X", DEV_EUI[i]); } Output (msgbuf);
    }

    if (!isValidHexString(cf_lw_appkey, 32)) {
       Output("cf_lw_appkey - Invalid");
       invalid=true;
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
      invalid=true;
    }
    // sprintf(msgbuf, "DEV_ADDR %X", DEV_ADDR); Output (msgbuf);

    if (!isValidHexString(cf_lw_nwkskey, 32)) {
       Output("lw_nwkskey - Invalid");
       invalid=true;
    }
    else {
      hexStringToByteArray(cf_lw_nwkskey, NWK_SKEY, 32);
      // Output ("NWK_SKEY"); for (int i=0; i<16; i++) { sprintf(msgbuf+(i*2), "%02X", NWK_SKEY[i]); } Output (msgbuf);
    }

    if (!isValidHexString(cf_lw_appskey, 32)) {
      Output("lw_appskey - Invalid");
      invalid=true;
    }
    else {
      hexStringToByteArray(cf_lw_appskey, APP_SKEY, 32);
      // Output ("APP_SKEY"); for (int i=0; i<16; i++) { sprintf(msgbuf+(i*2), "%02X", APP_SKEY[i]); } Output (msgbuf);
    }
  }

  if (invalid) {
    Output("LW:CONFIG ERROR");
    return;
  }
 
  // os_init();
  os_init_ex(&lmic_pins);
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);  // Compensate clock drift
                                                  // Open RX windows 1% earlier and extends them slightly longer
                                                  // Matches real crystal error so JoinAccept/downlinks arrive in time

  if (cf_lw_mode == LORA_ABP) {
    // Used For ABP Setup
    LMIC_setSession(0x1, DEV_ADDR, NWK_SKEY, APP_SKEY); // Set up LoRaWAN using ABP

    // TTN uses SF9 for its RX2 downlink window. - was after check mode
    LMIC.dn2Dr = DR_SF9;
  }

  // LMIC_setAdrMode(0) vs LMIC_setAdrMode(1)
  // Mode	Uplink  ADR Bit	Server Behavior	                        Your Control
  //  0 (Static)	ADR=0	  Ignores you - you pick fixed DR/power	  LMIC_setDrTxpow(DR_SF7,14) works
  // 1 (Dynamic)	ADR=1	  Controls you via LinkADRReq	            Server decides everything
  LMIC_setAdrMode(0);

   // 0 = Disabled: No extra LinkCheck messages (saves airtime, ~1% duty cycle friendly). No margin/NbTrans checks.
   // 1 = Enabled: Every 32 uplinks, device sends LinkCheckReq. Server responds in downlink with margin 
   //        (signal quality) and gateway count. EV_LINK_DEAD triggers if poor.
   // It is disabled post-join aka EV_JOINED
  LMIC_setLinkCheckMode(0);

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  // Close  (<1km)	SF7–8   (DR4–5)	222 bytes
  // Medium (1–5km)	SF9–10  (DR2–3)	115 bytes 
  // Far    (>5km)	SF11–12 (DR0–1)

  /* 
     EU Spreading Factor (SF)
     DR | Spreading Factor | Bitrate | Airtime (51B)
      --|------------------|---------|---------------
     0  | SF12/125kHz     | 250 bps | ~2.5s
     1  | SF11/125kHz     | 440 bps | ~1.4s
     2  | SF10/125kHz     | 980 bps | ~0.7s
     3  | SF9/125kHz      | 1760 bps| ~0.4s
     4  | SF8/125kHz      | 3125 bps| ~0.2s
     5  | SF7/125kHz      | 5470 bps| ~0.1s
     6  | SF7/250kHz      | 11kbps  | ~0.07s
     7  | FSK/50kbps      | 50kbps  | ~0.01s

    US Spreading Factor (SF)
    DR | Spreading Factor  | Bitrate    | Airtime (51B)
    --|--------------------|------------|---------------
    0  | SF10/125kHz       |  5.5kbps  | ~0.1s
    1  | SF9/125kHz        |  9.8kbps  | ~0.06s
    2  | SF8/125kHz        | 17.5kbps  | ~0.03s
    3  | SF7/125kHz        | 31.3kbps  | ~0.02s
    4  | SF8/500kHz        |  12.5kbps | ~0.04s
    5  | SF7/500kHz        |  22.0kbps | ~0.03s
    6  | SF7/500kHz        |  22.0kbps | ~0.03s
    7  | SF7/500kHz        |  22.0kbps | ~0.03s
    8  | SF12/500kHz       |    980bps | ~0.7s
    9  | SF11/500kHz       |   1.8kbps | ~0.4s
    10 | SF10/500kHz       |   3.9kbps | ~0.2s
    11 | SF9/500kHz        |   7.8kbps | ~0.1s
    12 | SF8/500kHz        |  17.5kbps | ~0.03s
    13 | SF7/500kHz        |  22.0kbps | ~0.03s

    TX Power (dBm) | EU868 Max | US915 Max | Notes
    ---------------|-----------|-----------|------
    2              | 2         | 2         | Min
    5              | 5         | 5         | 
    8              | 8         | 8         | 
    11             | 11        | 11        | 
    14             | 14        | 14        | Common - Safe default - No antenna gain assumed
    17             | 16        | 17        | EU duty cycle limits - TTN Fair Use (1% duty cycle)
    20             | 20        | 20        | Max SX1276 - Max EIRP (14dBm + 6dBi antenna)

    Practical Issues with 20dBm
      Current draw: ~120mA (vs 30mA @14dBm) drains Particle Boron battery fast
      Heat: SX1276 gets hot during SF12's 2.5s airtime
      Duty cycle: EU868 1% limit → 36s wait after each SF12/20dBm transmit
      LMIC auto-clamp: Library caps at MAX_TX_POWER (often 14dBm for EU868)

    For a Feather in EU868: keep txpow at 14 dBm to stay compliant.
    For a Feather in US915: you can request up to 20 dBm, but using 14–20 dBm is usually a 
    good balance between range and current draw; going above 14 brings diminishing returns 
    unless you really need the extra link margin.

    All values 14–20 dBm are valid, with higher ones (17–20) drawing more current 
    (~120 mA (0.12 amp) peak at 20 dBm)  but offering better range.
  */
  // Optional: Set initial Data Rate (DR) (server will override) unless LMIC_setAdrMode(0);
  if (cf_lw_sf == 8) {
    LMIC_setDrTxpow(DR_SF8, cf_lw_txpw);
  }
  else { // Default
    LMIC_setDrTxpow(DR_SF7, cf_lw_txpw);
  }

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

  if (cf_lw_mode == LORA_OTAA) {
    // Explicitly start OTAA join
    LMIC_startJoining();
  }

  LW_valid = true;
  LW_Show_FreqPlan();
 }
