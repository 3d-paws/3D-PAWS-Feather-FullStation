/*
 * ======================================================================================================================
 *  LoRa.h - LoRa Functions
 * ======================================================================================================================
 */

// Prototyping functions to aviod compile function unknown issue.
void SD_NeedToSend_Add(char *observation);
 
 /*
 * ======================================================================================================================
 *  Define who we are on LoRa network and who we are seding as 
 * ======================================================================================================================
 */
// #define LORA_RECEIVING_STATION_ID  1   // This unit's LoRa ID for Receiving Messages

/*
 * ======================================================================================================================
 *  Singleton instance of the radio driver
 *
 *  The RH_RF95 driver uses interrupts to react to events in the RFM module,
 *  such as the reception of a new packet, or the completion of transmission
 *  of a packet.  The RH_RF95 driver interrupt service routine reads status from
 *  and writes data to the the RFM module via the SPI interface. It is very
 *  important therefore, that if you are using the RH_RF95 driver with another
 *  SPI based deviced, that you disable interrupts while you transfer data to
 *  and from that other device.  Use cli() to disable interrupts and sei() to
 *  reenable them.
 * 
 *  Note: We are dedicating SPI1 to the RH_RF95 driver. No need to mask interrupts.
 * ======================================================================================================================
 */
#define LORA_IRQ_PIN  D6    // G0 on LoRa board
#define LORA_SS       D10   // Slave Select Pin
#define LORA_RESET    D9    // Used by lora_initialize()
#define LORA_RESET_NOACTIVITY 30 // 30 minutes
RH_RF95 rf95(LORA_SS, LORA_IRQ_PIN, hardware_spi); // SPI1
bool LORA_exists = false;
uint64_t lora_alarm_timer;   // Must get a LoRa mesage by the time set here, else we call lora_initialize()

/*
 * =======================================================================================================================
 *  AES Encryption - These need to be changed here and on the RaspberryPi (They need to match)
 * =======================================================================================================================
 */
// Private Key
uint8_t  AES_KEY[16]; // Now filled in from CONFIG.TXT


// Initialization Vector must be and always will be 128 bits (16 bytes.)
// The real iv is actually my_iv repeated twice EX: 01234567 = 0123456701234567
// 01234567 - > 0x12D687 = 0x00 0x00 0x00 0x00 0x00 0x12 0xD6 0x87
// unsigned long long int my_iv = 01234567;
unsigned long long int AES_MYIV;


/*
 * =======================================================================================================================
 *  AES Instance
 * =======================================================================================================================
 */
AES aes;

/*
 * ======================================================================================================================
 *  LoRa Connected Rain Gauges and Soil Moisture
 * ======================================================================================================================
 */
#define LORA_RELAY_MSGCNT     64  // Set to the number of LoRa RS devices this station will be supporting
#define LORA_RELAY_MSG_LENGTH 256
const char *relay_msgtypes[] = {"UNKN", "INFO", "RS", "SG"}; // Particle Message Types being received for relay
typedef struct {
  bool          need2log;
  int           message_type;
  char          message[LORA_RELAY_MSG_LENGTH];
} LORA_MSG_RELAY_STR;
LORA_MSG_RELAY_STR lora_msg_relay[LORA_RELAY_MSGCNT];


/* 
 *=======================================================================================================================
 * lora_relay_msg_free()
 *=======================================================================================================================
 */
void lora_relay_msg_free(LORA_MSG_RELAY_STR *m) {
  m->need2log = false;
  m->message_type = 0;
  memset (m->message, 0, LORA_RELAY_MSG_LENGTH);
}


/* 
 *=======================================================================================================================
 * lora_relay_need2log() - Return true if we have a relay that needs to be logged
 *=======================================================================================================================
 */
bool lora_relay_need2log() {
  for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
    if (lora_msg_relay[i].need2log) {
      return (true);
    }
  }
  return(false);
}

/* 
 *=======================================================================================================================
 * lora_relay_notinuse() - return first open spot or -1 if full
 *=======================================================================================================================
 */
int lora_relay_notinuse() {
  for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
    if (!lora_msg_relay[i].need2log) {
      return (i);
    }
  }
  return(-1);
}

/* 
 *=======================================================================================================================
 * lora_relay_need2log_idx() - return first need2log spot or -1 if none
 *=======================================================================================================================
 */
int lora_relay_need2log_idx() {
  for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
    if (lora_msg_relay[i].need2log) {
      return (i);
    }
  }
  return(-1);
}

/* 
 *=======================================================================================================================
 * lora_msgs_to_n2s()
 *=======================================================================================================================
 */
void lora_msgs_to_n2s() {
  if (LORA_exists) {
    LORA_MSG_RELAY_STR *m;

    for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
      m = &lora_msg_relay[i];
      if (m->need2log) {
        sprintf (msgbuf, "%s,%s", m->message, relay_msgtypes[m->message_type]);
        SD_NeedToSend_Add(msgbuf); // Save to N2F File
        lora_relay_msg_free(m);
        sprintf (Buffer32Bytes, "LoRaMsg[%d]->N2S", i);
        Output (Buffer32Bytes);
      }
    }
  }
}


/* 
 *=======================================================================================================================
 * lora_device_initialize()
 *=======================================================================================================================
 */
void lora_device_initialize() {
  if (LORA_exists) {
    // Init LoRa Relay Message structure
    for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
      lora_relay_msg_free(&lora_msg_relay[i]);
    }
  }
}
/* 
 *=======================================================================================================================
 * lora_cf_validate() - Validate LoRa variables from CONFIG.TXT
 *=======================================================================================================================
 */
bool lora_cf_validate() {
  if (cf_aes_pkey == NULL) {
    Output ("AES PKEY !SET");
    return (false);
  }
  else if (strlen (cf_aes_pkey) != 16) {
    Output ("AES PKEY !16 Bytes");
    return (false);    
  }
  else if (cf_aes_myiv == 0) {
    Output ("AES MYIV !SET");
    return (false);
  }
  else if ((cf_lora_txpower<5) || (cf_lora_txpower>23)) {
    Output ("LORA PWR ERR");
    return (false);
  }
  else if ((cf_lora_freq!=915) && (cf_lora_freq!=866) && (cf_lora_freq!=433)) {
    Output ("LORA FREQ ERR");
    return (false);        
  }
  else if ((cf_lora_unitid<0) || (cf_lora_unitid>254)) {
    Output ("LORA ADDR ERR");
    return (false);
  }
  else { 
    memcpy ((char *)AES_KEY, cf_aes_pkey, 16);
    sprintf(msgbuf, "AES_KEY[%s]", cf_aes_pkey); Output (msgbuf);

    AES_MYIV=cf_aes_myiv;
    sprintf(msgbuf, "AES_MYIV[%u]", AES_MYIV); Output (msgbuf);

    Output ("LORA CFV OK");
    return (true);
  }
}

/* 
 *=======================================================================================================================
 * lora_initialize()
 *=======================================================================================================================
 */
void lora_initialize() {

  // Validate LoRa variables from CONFIG.TXT
  if (lora_cf_validate()) {
    // Output ("LORA Init");
    pinMode(LORA_RESET, OUTPUT);
    // manual reset
    digitalWrite(LORA_RESET, LOW);
    delay(100);
    digitalWrite(LORA_RESET, HIGH);
    delay(100);

    if (rf95.init()) {
      // The default transmitter power is 13dBm, using PA_BOOST.
      // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
      // you can set transmitter powers from 5 to 23 dBm:
      rf95.setTxPower(cf_lora_txpower, false);

      // If you are using Modtronix inAir4 or inAir9,or any other module which uses the
      // transmitter RFO pins and not the PA_BOOST pins
      // then you can configure the power transmitter power for -1 to 14 dBm and with useRFO true. 
      // Failure to do that will result in extremely low transmit powers.
      //  driver.setTxPower(14, true);

      rf95.setFrequency(cf_lora_freq);

      // If we need to send something
      rf95.setThisAddress(cf_lora_unitid);
      rf95.setHeaderFrom(cf_lora_unitid);

      // Be sure to grab all node packet 
      rf95.setPromiscuous(true);

      // We're ready to listen for incoming message
      rf95.setModeRx();

      LORA_exists=true;

      Output ("LORA OK");
    }
    else {
      SystemStatusBits |= SSB_LORA;  // Turn On Bit
    
      Output ("LORA NF");
    }
  }
  else {
    Output ("LORA INIT ERR");
  }
  // Even if LoRa not found set the timer for time we call initialize after setup()
  lora_alarm_timer = System.millis() + (LORA_RESET_NOACTIVITY * 60000);  // Minutes * 60 seconds
}

/* 
 *=======================================================================================================================
 * lora_relay_msg() -LoRa Rain and Soil Remote Sensors we relay their messages to Particle
 *    
 * Relay Message Format
 *   NCS    Length (N) and Checksum (CS)
 *   XX,    Lora Message Type IF(INFO) RS, SG
 *   INT,   Station ID
 *   INT,   Message Counter
 *   OBS    JSON Observation
 *=======================================================================================================================
 */
void lora_relay_msg(char *obs) {
  LORA_MSG_RELAY_STR *m;

  int message_type = 0;
  int unit_id = 0;
  unsigned int message_counter = 0;
  char *message;
  char *p;

  if ((obs[0] == 'I') && (obs[1] == 'F')) {
    message_type = 1;
  }
  else if ((obs[0] == 'R') && (obs[1] == 'S')) {
    message_type = 2; 
  }
  else if ((obs[0] == 'S') && (obs[1] == 'G')) {
    message_type = 3;
  }
  else {
    sprintf (Buffer32Bytes, "LORA Relay %c%c Unkn", obs[0], obs[1]);
    Output (Buffer32Bytes);
    return;
  }

  p = &obs[2]; // Start after message type 
  unit_id = atoi (strtok_r(p, ",", &p));
  message_counter = atoi (strtok_r(p, ",", &p));
  message = p;

  sprintf (Buffer32Bytes, "Relay %s ID:%d CNT:%d", relay_msgtypes[message_type], unit_id, message_counter);
  Output (Buffer32Bytes);
  // Output (message);

  // Locate storage index
  int idx = lora_relay_notinuse();

  if (idx == -1) {
    Output ("LORA Relay NoSpace");

    // Dump all LoRA messages to N2S
    lora_msgs_to_n2s();  // No Space - Dump all LoRA messages to N2S, then save new message

    idx = lora_relay_notinuse(); // This better not be -1 after freeing
    if (idx == -1) {
      Output ("LORA Relay MsgLost");
      return;
    }
  }

  m = &lora_msg_relay[idx]; // Lets work with a pointer and not the index
  m->need2log = true;
  m->message_type = message_type;
  strncpy (m->message, message, LORA_RELAY_MSG_LENGTH-1); // minus 1 so last byte in array will always be null
  sprintf (Buffer32Bytes, "LORA Relay %s -> Queued:%d", relay_msgtypes[message_type], idx);
  Output (Buffer32Bytes);
}

/* 
 *=======================================================================================================================
 * lora_msg_check()
 *=======================================================================================================================
 */
void lora_msg_check() {

  if (LORA_exists) {
    // Output ("LoRa Check()");
    if (rf95.available()) {
      byte iv [N_BLOCK];
    
      // Should be a message for us now
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN]; // 251 Bytes
      uint8_t len  = sizeof(buf);
      uint8_t from = rf95.headerFrom();
      uint8_t to   = rf95.headerTo();
      uint8_t id   = rf95.headerId();
      uint8_t flags= rf95.headerFlags();
      int8_t  rssi = rf95.lastRssi(); 
      uint16_t checksum = 0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t i;
      uint8_t msglen = 0;
      char msg[256];             // Used to hold decrypted lora messages

      memset(buf, 0, RH_RF95_MAX_MESSAGE_LEN);
      memset(msg, 0, RH_RF95_MAX_MESSAGE_LEN+1);
    
      if (rf95.recv(buf, &len)) {
        // memcpy (msg, &buf[3], buf[0]);
        // Output (msg);
        // Serial_write ("LoRa Msg");
     
        aes.iv_inc();
        aes.set_IV(AES_MYIV);
        aes.get_IV(iv);
        aes.do_aes_decrypt(buf, len, (byte *) msg, AES_KEY, 128, iv);
      
        if ( ( msg[3] == 'I' && msg[4] == 'F') ||
             ( msg[3] == 'R' && msg[4] == 'S') || 
             ( msg[3] == 'S' && msg[4] == 'G') ) {

          // Get length of what follows
          msglen = msg[0];

          // Compute Checksum
          checksum=0;
          for (i=3; i<msglen; i++) {
            checksum += msg[i];
          }
          byte1 = checksum>>8;
          byte2 = checksum%256;

          // Validate Checksum against sent checksum
          if ((byte1 == msg[1]) && (byte2 == msg[2])) {
            // Make what follows a string
            msg[msglen]=0;

            char *payload = (char*)(msg+3); // After length and 2 checksum bytes

            // Display LoRa Message on Serial Console           
            Serial_write (payload);

            lora_relay_msg (payload);
          }
          else {
            Output ("LORA CS-ERR");
          }
        }
      }
      // Received LoRa Signal, Reset alarm
      lora_alarm_timer = System.millis() + (LORA_RESET_NOACTIVITY * 60000);
    }
    else {
      // Have not received LoRa message in N seconds
      if (System.millis() >= lora_alarm_timer) {
        Output ("LORA Init");
        // Reset and reinitialize LoRa
      
        // lora_reinitialize_count++   // Do a back on how often to reset
        // Need to set some system status bit here too
        lora_initialize();
      }
    }
  }
}

/* 
 *=======================================================================================================================
 * lora_msg_poll() -- Spend 750ms looking for LoRa Messages
 *=======================================================================================================================
 */
void lora_msg_poll() {
  for (int i=0; i<3; i++) {
    lora_msg_check();
    delay (250);
  }
}