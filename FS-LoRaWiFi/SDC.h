/*
 * ======================================================================================================================
 * SD.h - SD Card
 * ======================================================================================================================
 */
#define SD_ChipSelect     10                // GPIO 10 is Pin 10 on Feather and D5 on Particle Boron Board

#define CF_NAME           "CONFIG.TXT"
#define KEY_MAX_LENGTH    30                // Config File Key Length
#define VALUE_MAX_LENGTH  30                // Config File Value Length
#define LINE_MAX_LENGTH   VALUE_MAX_LENGTH+KEY_MAX_LENGTH+3   // =, CR, LF 

// SdFat SD;                                // File system object.
// SD;                                      // File system object defined by the SD.h include file.
File SD_fp;
char SD_obsdir[] = "/OBS";                  // Observations stored in this directory. Created at power on if not exist
bool SD_exists = false;                     // Set to true if SD card found at boot
char SD_n2s_file[] = "N2SOBS.TXT";          // Need To Send Observation file
uint32_t SD_n2s_max_filesz = 512 * 60 * 24; // Keep a little over 1 day. When it fills, it is deleted and we start over.

/* 
 * =======================================================================================================================
 * SD_initialize()
 * =======================================================================================================================
 */
void SD_initialize() {

  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH); // We need to set this pin high to disable LoRa, prior to accessing the SD card
  
  if (!SD.begin(SD_ChipSelect)) {
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
  
  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
     
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
}

/* 
 * =======================================================================================================================
 * SD_N2S_Delete()
 * =======================================================================================================================
 */
bool SD_N2S_Delete() {
  bool result;

  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  
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

  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  
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
}

/* 
 * =======================================================================================================================
 * Support functions for Config file
 * 
 *  https://arduinogetstarted.com/tutorials/arduino-read-config-from-sd-card
 *  
 *  myInt_1    = SD_findInt(F("myInt_1"));
 *  myFloat_1  = SD_findFloat(F("myFloat_1"));
 *  myString_1 = SD_findString(F("myString_1"));
 *  
 *  CONFIG.TXT content example
 *  myString_1=Hello
 *  myInt_1=2
 *  myFloat_1=0.74
 * =======================================================================================================================
 */

int SD_findKey(const __FlashStringHelper * key, char * value) {
  
  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  
  File configFile = SD.open(CF_NAME);

  if (!configFile) {
    Serial.print(F("SD Card: error on opening file "));
    Serial.println(CF_NAME);
    return(0);
  }

  char key_string[KEY_MAX_LENGTH];
  char SD_buffer[KEY_MAX_LENGTH + VALUE_MAX_LENGTH + 1]; // 1 is = character
  int key_length = 0;
  int value_length = 0;

  // Flash string to string
  PGM_P keyPoiter;
  keyPoiter = reinterpret_cast<PGM_P>(key);
  byte ch;
  do {
    ch = pgm_read_byte(keyPoiter++);
    if (ch != 0)
      key_string[key_length++] = ch;
  } while (ch != 0);

  // check line by line
  while (configFile.available()) {
    int buffer_length = configFile.readBytesUntil('\n', SD_buffer, LINE_MAX_LENGTH);
    if (SD_buffer[buffer_length - 1] == '\r')
      buffer_length--; // trim the \r

    if (buffer_length > (key_length + 1)) { // 1 is = character
      if (memcmp(SD_buffer, key_string, key_length) == 0) { // equal
        if (SD_buffer[key_length] == '=') {
          value_length = buffer_length - key_length - 1;
          memcpy(value, SD_buffer + key_length + 1, value_length);
          break;
        }
      }
    }
  }

  configFile.close();  // close the file
  return value_length;
}

int HELPER_ascii2Int(char *ascii, int length) {
  int sign = 1;
  int number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

float HELPER_ascii2Float(char *ascii, int length) {
  int sign = 1;
  int decimalPlace = 0;
  float number  = 0;
  float decimal = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c == '.')
        decimalPlace = 1;
      else if (c >= '0' && c <= '9') {
        if (!decimalPlace)
          number = number * 10 + (c - '0');
        else {
          decimal += ((float)(c - '0') / pow(10.0, decimalPlace));
          decimalPlace++;
        }
      }
    }
  }

  return (number + decimal) * sign;
}

String HELPER_ascii2String(char *ascii, int length) {
  String str;
  str.reserve(length);
  str = "";

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str += String(c);
  }
  return str;
}

char* HELPER_ascii2CharStr(char *ascii, int length) {
  char *str;
  int i = 0;
  str = (char *) malloc (length+1);
  str[0] = 0;
  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str[i] = c;
    str[i+1] = 0;
  }
  return str;
}

bool SD_available(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return value_length > 0;
}

int SD_findInt(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Int(value_string, value_length);
}

float SD_findFloat(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Float(value_string, value_length);
}

String SD_findString(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2String(value_string, value_length);
}

char* SD_findCharStr(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2CharStr(value_string, value_length);
}


/* 
 * =======================================================================================================================
 * SD_ReadConfigFile()
 * =======================================================================================================================
 */
void SD_ReadConfigFile() {

  // LoRa
  cf_lw_mode      = SD_findInt(F("lw_mode"));
  sprintf(msgbuf, "CF:%s=[%d]", F("lw_mode"),   cf_lw_mode);   Output (msgbuf);

  // LoRa OTAA
  cf_lw_appeui    = SD_findCharStr(F("lw_appeui"));
  sprintf(msgbuf, "CF:%s=[%s]", F("lw_appeui"), cf_lw_appeui); Output (msgbuf);

  cf_lw_deveui    = SD_findCharStr(F("lw_deveui"));
  sprintf(msgbuf, "CF:%s=[%s]", F("lw_appkey"), cf_lw_deveui); Output (msgbuf);

  cf_lw_appkey    = SD_findCharStr(F("lw_appkey"));
  sprintf(msgbuf, "CF:%s=[%s]", F("lw_appkey"), cf_lw_appkey); Output (msgbuf);

  // LoRa ABP
  cf_lw_devaddr   = SD_findCharStr(F("lw_devaddr"));
  sprintf(msgbuf, "CF:%s=[%s]", F("lw_devaddr"), cf_lw_devaddr); Output (msgbuf);

  cf_lw_nwkskey   = SD_findCharStr(F("lw_nwkskey"));
  sprintf(msgbuf, "CF:%s=[%s]", F("lw_nwkskey"), cf_lw_nwkskey); Output (msgbuf);

  cf_lw_appskey   = SD_findCharStr(F("lw_appskey"));
  sprintf(msgbuf, "CF:%s=[%s]", F("lw_appkey"), cf_lw_appskey); Output (msgbuf);
  
  // WiFi
  cf_wifi_ssid    = SD_findCharStr(F("wifi_ssid"));
  sprintf(msgbuf, "CF:%s=[%s]", F("wifi_ssid"), cf_wifi_ssid); Output (msgbuf);

  cf_wifi_pw      = SD_findCharStr(F("wifi_pw"));
  sprintf(msgbuf, "CF:%s=[%s]", F("wifi_pw"), cf_wifi_pw); Output (msgbuf);

  cf_wifi_wepkey  = SD_findCharStr(F("wifi_wepkey"));
  sprintf(msgbuf, "CF:%s=[%s]", F("wifi_wepkey"), cf_wifi_wepkey); Output (msgbuf);

  // Web Server
  cf_webserver      = SD_findCharStr(F("webserver"));
  sprintf(msgbuf, "CF:%s=[%s]", F("webserver"), cf_webserver); Output (msgbuf);
  
  cf_webserver_port = SD_findInt(F("webserver_port"));
  sprintf(msgbuf, "CF:%s=[%d]", F("webserver_port"), cf_webserver_port); Output (msgbuf);

  cf_urlpath        = SD_findCharStr(F("urlpath"));
  sprintf(msgbuf, "CF:%s=[%s]", F("urlpath"), cf_urlpath); Output (msgbuf);
  
  cf_apikey         = SD_findCharStr(F("apikey"));
  sprintf(msgbuf, "CF:%s=[%s]", F("apikey"), cf_apikey); Output (msgbuf);
  
  cf_instrument_id  = SD_findInt(F("instrument_id"));
  sprintf(msgbuf, "CF:%s=[%d]", F("instrument_id"), cf_instrument_id); Output (msgbuf);

  //Time Server
  cf_ntpserver      = SD_findCharStr(F("ntpserver"));
  sprintf(msgbuf, "CF:%s=[%s]", F("ntpserver"), cf_ntpserver); Output (msgbuf);

  // Rain
  cf_rg1_enable   = SD_findInt(F("rg1_enable"));
  sprintf(msgbuf, "CF:%s=[%d]", F("rg1_enable"), cf_rg1_enable); Output (msgbuf);

  cf_rg2_enable   = SD_findInt(F("rg2_enable"));
  sprintf(msgbuf, "CF:%s=[%d]", F("rg2_enable"), cf_rg2_enable); Output (msgbuf);

  // Distance
  cf_ds_enable    = SD_findInt(F("ds_enable"));
  sprintf(msgbuf, "CF:%s=[%d]", F("ds_enable"), cf_ds_enable); Output (msgbuf);

  cf_ds_baseline = SD_findInt(F("ds_baseline"));
  sprintf(msgbuf, "CF:ds_baseline=[%d]", cf_ds_baseline); Output (msgbuf);

  cf_obs_period   = SD_findInt(F("obs_period"));
  if (cf_obs_period == 0) {
    cf_obs_period = 1;
  }
  sprintf(msgbuf, "CF:obs_period=[%d]", cf_obs_period); Output (msgbuf);
  
  // Misc
  cf_daily_reboot = SD_findInt(F("daily_reboot"));
  sprintf(msgbuf, "CF:%s=[%d]", F("daily_reboot"), cf_daily_reboot); Output (msgbuf);
}
