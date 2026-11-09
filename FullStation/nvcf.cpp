/*
 * ======================================================================================================================
 *  nvcf.cpp - Non Volitile Configuration File Functions
 * ======================================================================================================================
 */
#include <fcntl.h>

#include "include/qc.h"
#include "include/support.h"
#include "include/main.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/sensor_rgop.h"
#include "include/eeprom.h"
#include "include/nvcf_epfs.h"
#include "include/nvcf.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
SCV scv; // System Configuration Variables

/*
 * ======================================================================================================================
 *  Define Global Configuration File Variables
 * ======================================================================================================================
 */
static const char* SD_CONFIG_FILE = "/CONFIG.TXT";
static const char* SD_TEMP_FILE   = "/CONFIG.TMP";
static const char* SD_BACKUP_FILE = "/CONFIG.BAK";


/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

 /*
 * ======================================================================================================================
 * nv_op1_state() - null,raw,d5m,d10m,rg2
 * =======================================================================================================================
 */
int nv_op1_state() {

  // Option Pin 1 A4
  if (scv.op1 == "raw") {
    return(OP1_STATE_RAW);
  }
  else if (scv.op1 == "d5m") {
    return(OP1_STATE_DIST_5M);
  }
  else if (scv.op1 == "d10m") {
    return(OP1_STATE_DIST_10M);
  }
  else if (scv.op1 == "rg2") {
    return(OP1_STATE_RAIN);
  }
  else { // null
    return(OP1_STATE_NULL);
  }
}

/*
 * ======================================================================================================================
 * nv_op2_state() - null,raw,voltaic
 * =======================================================================================================================
 */
int nv_op2_state() {

  // Option Pin 1 A4
  if (scv.op1 == "raw") {
    return(OP2_STATE_RAW);
  }
  else if (scv.op1 == "voltaic") {
    return(OP2_STATE_VOLTAIC);
  }
  else { // null
    return(OP2_STATE_NULL);
  }
}

/*
 * ======================================================================================================================
 * nv_parseLine()
 * =======================================================================================================================
 */
bool nv_parseLine(const String& line) {
    String s = line;
    s.trim();
    if (s.length() == 0) return true;
    if (s.startsWith("#") || s.startsWith("//")) return true;

    int eq = s.indexOf('=');
    if (eq < 1) return false;

    String key = s.substring(0, eq);
    key.trim();
    String val = s.substring(eq + 1);
    val.trim();

    // Feather M0 LoRaWAN Board
    if (key == "lw_mode")           scv.lw_mode = val;
    else if (key == "lw_sf")        scv.lw_sf = val.toInt();
    else if (key == "lw_txpwr")     scv.lw_txpwr = val.toInt();
    // OTAA
    else if (key == "otaa_joineui") scv.otaa_joineui = val;
    else if (key == "otaa_deveui")  scv.otaa_deveui = val;
    else if (key == "otaa_appkey")  scv.otaa_appkey = val;
    // ABP
    else if (key == "abp_devaddr")  scv.abp_devaddr = val;
    else if (key == "abp_nwkskey")  scv.abp_nwkskey = val;
    else if (key == "abp_appskey")  scv.abp_appskey = val;

    // Feather M0 WiFi Board
    else if (key == "wifi_ssid")    scv.wifi_ssid = val;
    else if (key == "wifi_pw")      scv.wifi_pw = val;
    else if (key == "wifi_wepkey")  scv.wifi_wepkey = val;
    // Log Server - For example a CHORDS web server
    else if (key == "ls_fqdn")      scv.ls_fqdn = val;
    else if (key == "ls_port")      scv.ls_port = val.toInt();
    else if (key == "ls_urlpath")   scv.ls_urlpath = val;
    else if (key == "ls_apikey")    scv.ls_apikey = val;
    else if (key == "ls_instr_id")  scv.ls_instr_id = val.toInt();
    // Info Server
    else if (key == "is_fqdn")      scv.is_fqdn = val;
    else if (key == "is_port")      scv.is_port = val.toInt();
    else if (key == "is_urlpath")   scv.is_urlpath = val;
    else if (key == "is_apikey")    scv.is_apikey = val;
    // Time Server
    else if (key == "ts_fqdn")      scv.ts_fqdn = val;

    // Feather M0 Board with NoteCard
    else if (key == "nc_debug")     scv.nc_debug = val.toInt();
    else if (key == "nc_product")   scv.nc_product = val;
    else if (key == "nc_mode")      scv.nc_mode = val;
    else if (key == "nc_outbound")  scv.nc_outbound = val.toInt();
    else if (key == "nc_inbound")   scv.nc_inbound = val.toInt();
    else if (key == "nc_dfu")       scv.nc_dfu = val.toInt();
    else if (key == "nc_doa")       scv.nc_doa = val.toInt();
    else if (key == "nc_method")    scv.nc_method = val;
    // else if (key == "nc_qo_obs")    scv.nc_qo_obs = val;
    // else if (key == "nc_qo_info")   scv.nc_qo_info = val;
    // else if (key == "nc_qi_doa")   scv.nc_qi_doa = val;
    // else if (key == "nc_rr")        scv.nc_rr = val.toInt();
    else if (key == "nc_esim_apn")  scv.nc_esim_apn = val;
    else if (key == "nc_esim_method")scv.nc_esim_method = val;
    else if (key == "nc_wifi_ssid") scv.nc_wifi_ssid = val;
    else if (key == "nc_wifi_pw")   scv.nc_wifi_pw = val; 

    // Instruments
    else if (key == "aqs")          scv.aqs = val.toInt();
    else if (key == "wind")         scv.wind = val.toInt();
    else if (key == "rg1")          scv.rg1 = val.toInt();
    else if (key == "op1")          scv.op1 = val;
    else if (key == "op2")          scv.op2 = val;
    else if (key == "ds_baseline")  scv.ds_baseline = val.toInt();

    // System Configuration
    else if (key == "daily_reboot") scv.daily_reboot = val.toInt();
    else if (key == "obsi")         scv.obsi = val.toInt();
    else if (key == "rtro_hour")    scv.rtro_hour = val.toInt();
    else if (key == "rtro_minute")  scv.rtro_minute = val.toInt();
    else if (key == "elevation")    scv.elevation = val.toInt();
    else if (key == "lat_deg")      scv.lat_deg = val.toFloat();
    else if (key == "lon_deg")      scv.lon_deg = val.toFloat();
    else return false;

    return true;
}

/*
 * ======================================================================================================================
 * sd_writeAll() -- 
 * =======================================================================================================================
 */
static ssize_t sd_writeAll(File& fp, const char* data, size_t len) {
    size_t total = 0;

    while (total < len) {
        size_t n = fp.write(reinterpret_cast<const uint8_t*>(data + total), len - total);

        // SdFat write() returns the count actually written.
        // A zero-byte write before completion is a failure/no-progress case.
        if (n == 0) {
            return -1;
        }

        total += n;
    }
    return static_cast<ssize_t>(total);
}

/*
 * ======================================================================================================================
 * sd_writeLine()
 * =======================================================================================================================
 */
static bool sd_writeLine(File& fp, const char* fmt, ...) {
    char buf[160];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n < 0 || static_cast<size_t>(n) >= sizeof(buf)) {
        return false;
    }
    return sd_writeAll(fp, buf, static_cast<size_t>(n)) == n;
}

/*
 * ======================================================================================================================
 * sd_loadConfig()
 * =======================================================================================================================
 */
bool sd_loadConfig() {
  if (SD_exists) {
    File fp = SD.open(SD_CONFIG_FILE, FILE_READ);
    if (!fp) {
        sprintf(msgbuf, "SDLC:%s NF, Using Defaults", SD_CONFIG_FILE);
        Output(msgbuf);
        return false;
    }
    sprintf(msgbuf, "SDLC:%s Found", SD_CONFIG_FILE);
    Output(msgbuf);

    String line;

    while (true) {
        int c = fp.read();
        if (c < 0) {
            break;
        }

        char ch = static_cast<char>(c);

        if (ch == '\n') {
            nv_parseLine(line);
            line = "";
        } else if (ch != '\r') {
            line += ch;
        }
    }

    if (line.length() > 0) {
        nv_parseLine(line);
    }

    fp.close();
    return true;
  }
  else {
    return false;
  }
}

/*
 * ======================================================================================================================
 * sd_replaceConfig()
 *
 * Replace CONFIG.TXT with the already-written CONFIG.TMP. Preserve the previous configuration as CONFIG.BAK.
 *
 * Preconditions:
 *   - CONFIG.TMP was successfully written, synced, and closed.
 *   - No other code is accessing these three files.
 *
 * Returns:
 *   true  - CONFIG.TMP was promoted to CONFIG.TXT.
 *   false - Replacement failed; see diagnostic output.
 *
 * This provides backup and rollback, not an atomic or power-loss-proof filesystem transaction.
  * ======================================================================================================================
 */
static bool sd_replaceConfig() {

    if (!SD_exists) {
        Output("SD Replace:SD unavailable");
        return false;
    }

    // Check before changing either the main file or its backup.
    if (!SD.exists(SD_TEMP_FILE)) {
        Output("SD Replace:CONFIG.TMP missing");
        return false;
    }

    /*
     * Recover an interrupted previous replacement.
     * Restore the backup before attempting another replacement.
     */
    if (!SD.exists(SD_CONFIG_FILE) && SD.exists(SD_BACKUP_FILE)) {
        if (!SD.rename(SD_BACKUP_FILE, SD_CONFIG_FILE)) {
            Output("SD Replace:Backup recovery failed");
            return false;
        }
    }

    const bool hadConfig = SD.exists(SD_CONFIG_FILE);

    if (hadConfig) {
        // Only discard an older backup while the main file exists.
        if (SD.exists(SD_BACKUP_FILE)) {
            if (!SD.remove(SD_BACKUP_FILE)) {
                Output("SD Replace:Remove old backup failed");
                return false;
            }
        }

        // Preserve the current main file.
        if (!SD.rename(SD_CONFIG_FILE, SD_BACKUP_FILE)) {
            Output("SD Replace:Backup main failed");
            return false;
        }
    }

    // The destination is now absent.
    if (SD.rename(SD_TEMP_FILE, SD_CONFIG_FILE)) {
        // Keep CONFIG.BAK for recovery.
        return true;
    }

    Output("SD Replace:Promote temporary file failed");

    // Promotion failed: attempt to restore the previous main file.
    if (hadConfig) {
        if (SD.rename(SD_BACKUP_FILE, SD_CONFIG_FILE)) {
            Output("SD Replace:Previous config restored");
        }
        else {
            Output("SD Replace:Rollback failed; check CONFIG.BAK");
        }
    }

    // Do not intentionally delete either recovery file on failure.
    return false;
}

/*
 * ======================================================================================================================
 * sd_saveConfig()
 * =======================================================================================================================
 */
bool sd_saveConfig() {
  if (!SD_exists) {
    return (false);
  }
  File fp = SD.open(SD_TEMP_FILE, O_WRONLY | O_CREAT | O_TRUNC);

  if (!fp) {
    return false;
  }

  bool ok = true;

  // Feather M0 LoRaWAN Board
  ok &= sd_writeLine(fp, "lw_mode=%s\n",      scv.lw_mode.c_str());
  ok &= sd_writeLine(fp, "lw_sf=%d\n",        scv.lw_sf);
  ok &= sd_writeLine(fp, "lw_txpwr=%d\n",     scv.lw_txpwr);
  // OTAA
  ok &= sd_writeLine(fp, "otaa_joineui=%s\n", scv.otaa_joineui.c_str());
  ok &= sd_writeLine(fp, "otaa_deveui=%s\n",  scv.otaa_deveui.c_str());
  ok &= sd_writeLine(fp, "otaa_appkey=%s\n",  scv.otaa_appkey.c_str());
  // ABP
  ok &= sd_writeLine(fp, "abp_devaddr=%s\n",  scv.abp_devaddr.c_str());
  ok &= sd_writeLine(fp, "abp_nwkskey=%s\n",  scv.abp_nwkskey.c_str());
  ok &= sd_writeLine(fp, "abp_appskey=%s\n",  scv.abp_appskey.c_str());

  // Feather M0 WiFi Board
  ok &= sd_writeLine(fp, "wifi_ssid=%s\n",    scv.wifi_ssid.c_str());
  ok &= sd_writeLine(fp, "wifi_pw=%s\n",      scv.wifi_pw.c_str());
  ok &= sd_writeLine(fp, "wifi_wepkey=%s\n",  scv.wifi_wepkey.c_str());
  // Log Server - For example a CHORDS web server
  ok &= sd_writeLine(fp, "ls_fqdn=%s\n",      scv.ls_fqdn.c_str());
  ok &= sd_writeLine(fp, "ls_port=%d\n",      scv.ls_port);
  ok &= sd_writeLine(fp, "ls_urlpath=%s\n",   scv.ls_urlpath.c_str());
  ok &= sd_writeLine(fp, "ls_apikey=%s\n",    scv.ls_apikey.c_str());
  ok &= sd_writeLine(fp, "ls_instr_id=%d\n",  scv.ls_instr_id);
  // Info Server
  ok &= sd_writeLine(fp, "is_fqdn=%s\n",      scv.is_fqdn.c_str());
  ok &= sd_writeLine(fp, "is_port=%d\n",      scv.is_port);
  ok &= sd_writeLine(fp, "is_urlpath=%s\n",   scv.is_urlpath.c_str());
  ok &= sd_writeLine(fp, "is_apikey=%s\n",    scv.is_apikey.c_str());
  // Time Server
  ok &= sd_writeLine(fp, "ts_fqdn=%s\n",      scv.ts_fqdn.c_str());

  // Feather M0 Board with NoteCard
  ok &= sd_writeLine(fp, "nc_debug=%d\n",     scv.nc_debug);
  ok &= sd_writeLine(fp, "nc_product=%s\n",   scv.nc_product.c_str());
  ok &= sd_writeLine(fp, "nc_mode=%s\n",      scv.nc_mode.c_str());
  ok &= sd_writeLine(fp, "nc_outbound=%d\n",  scv.nc_outbound);
  ok &= sd_writeLine(fp, "nc_inbound=%d\n",   scv.nc_inbound);
  ok &= sd_writeLine(fp, "nc_dfu=%d\n",       scv.nc_dfu);
  ok &= sd_writeLine(fp, "nc_doa=%d\n",       scv.nc_doa);
  ok &= sd_writeLine(fp, "nc_method=%s\n",    scv.nc_method.c_str());
  // ok &= sd_writeLine(fp, "nc_qo_obs=%s\n",    scv.nc_qo_obs.c_str());
  // ok &= sd_writeLine(fp, "nc_qo_info=%s\n",   scv.nc_qo_info.c_str());
  // ok &= sd_writeLine(fp, "nc_qi_doa=%s\n",    scv.nc_qi_doa.c_str());
  // ok &= sd_writeLine(fp, "nc_rr=%d\n",        scv.nc_rr);
  ok &= sd_writeLine(fp, "nc_esim_apn=%s\n",  scv.nc_esim_apn.c_str());
  ok &= sd_writeLine(fp, "nc_esim_method=%s\n",scv.nc_esim_method.c_str());
  ok &= sd_writeLine(fp, "nc_wifi_ssid=%s\n"  ,scv.nc_wifi_ssid.c_str());
  ok &= sd_writeLine(fp, "nc_wifi_pw=%s\n"    ,scv.nc_wifi_pw.c_str());

  // Instruments
  ok &= sd_writeLine(fp, "aqs=%d\n",          scv.aqs);
  ok &= sd_writeLine(fp, "wind=%d\n",         scv.wind);
  ok &= sd_writeLine(fp, "rg1=%d\n",          scv.rg1);
  ok &= sd_writeLine(fp, "op1=%s\n",          scv.op1.c_str());
  ok &= sd_writeLine(fp, "op2=%s\n",          scv.op2.c_str());
  ok &= sd_writeLine(fp, "ds_baseline=%d\n",  scv.ds_baseline);

  // System Configuration
  ok &= sd_writeLine(fp, "daily_reboot=%d\n", scv.daily_reboot);
  ok &= sd_writeLine(fp, "obsi=%d\n",         scv.obsi);
  ok &= sd_writeLine(fp, "rtro_hour=%d\n",    scv.rtro_hour);
  ok &= sd_writeLine(fp, "rtro_minute=%d\n",  scv.rtro_minute);
  ok &= sd_writeLine(fp, "elevation=%d\n",    scv.elevation);
  ok &= sd_writeLine(fp, "lat_deg=%.6f\n",    scv.lat_deg);
  ok &= sd_writeLine(fp, "lon_deg=%.6f\n",    scv.lon_deg);

  // Commit buffered file data and filesystem metadata.
  ok &= fp.sync();
  fp.close();

  if (!ok) {
    SD.remove(SD_TEMP_FILE);
    return false;
  }

  return sd_replaceConfig();
}


/*
 * ======================================================================================================================
 *  nv_writeAll() - Write all requested bytes or return -1 on error.
 * ======================================================================================================================
 */
ssize_t nv_writeAll(int fd, const char *data, size_t length) {
  size_t total = 0;

  while (total < length) {
    const char *position = data == NULL ? NULL : data + total;

    ssize_t count = epfs_FileWrite(fd, position, length - total);

    if (count < 0) {
      return -1;
    }

    if (count == 0) {
      epfs_errno = EPFS_EIO;
      return -1;
    }

    total += (size_t)count;
  }

  return (ssize_t)total;
}

/*
 * ======================================================================================================================
 * nv_writeLine()
 * =======================================================================================================================
 */
static bool nv_writeLine(int fd, const char* fmt, ...) {
    char buf[160];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n < 0 || static_cast<size_t>(n) >= sizeof(buf)) {
        return false;
    }
    return nv_writeAll(fd, buf, static_cast<size_t>(n)) == n;
}

/*
 * ======================================================================================================================
 * nv_DumpConfigLine() - Temporary diagnostic: print a configuration line as
 * text and hexadecimal bytes using Output()/msgbuf.
 * ======================================================================================================================
 */
void nv_DumpConfigLine(const String &line) {
  snprintf(msgbuf, sizeof(msgbuf),
           "EPFS Line %u: [%s]",
           (unsigned)line.length(),
           line.c_str());
  Output(msgbuf);

  unsigned int offset = 0;

  while (offset < line.length()) {
    size_t used = 0;

    msgbuf[0] = '\0';

    /* Print up to 16 hexadecimal bytes per Output() call. */
    while (offset < line.length() && used < sizeof(msgbuf) - 8) {
      unsigned int remaining = line.length() - offset;
      unsigned int chunk = remaining > 16 ? 16 : remaining;

      for (unsigned int i = 0; i < chunk; ++i) {
        snprintf(msgbuf + used, sizeof(msgbuf) - used,
                 " %02X", (uint8_t)line[offset + i]);
        used += 3;
      }

      offset += chunk;
    }

    Output(msgbuf);
  }
}

/*
 * ======================================================================================================================
 * nv_loadConfig() - Load and parse CONFIG.TXT.
 * Ignores carriage returns and handles a final line without a newline.
 * Skips lines exceeding 128 bytes, excluding CR/LF.
 * Uses the existing application function nv_parseLine().
 * ======================================================================================================================
 */
bool nv_loadConfig() {
  static constexpr size_t MAX_LINE_BYTES = 128;

  int fd = epfs_Open(EPFS_CONFIG_NAME, O_RDONLY);

  if (fd < 0) {
    if (epfs_errno == EPFS_ENOENT) {
      Output("EPFS Load:Config NF, Using Defaults");
    }
    else {
      Output("EPFS Load:Open failed");
    }
    return false;
  }

  Output("EPFS Load:Config Found");

  String line;
  char ch;
  bool skipLine = false;

  while (true) {
    ssize_t count = epfs_FileRead(fd, &ch, 1);

    if (count < 0) {
      epfs_Close(fd);
      Output("EPFS Load:Read failed");
      return false;
    }

    if (count == 0) {
      break;
    }

    if (ch == '\n') {
      if (!skipLine) {
        //nv_DumpConfigLine(line);
        nv_parseLine(line);
      }

      line = "";
      skipLine = false;
    }
    else if (ch != '\r' && !skipLine) {
      if (line.length() >= MAX_LINE_BYTES) {
        Output("EPFS Load:Skipping line exceeding 128 bytes");

        // Discard the entire line, including its buffered prefix.
        line = "";
        skipLine = true;
      }
      else {
        line += ch;
      }
    }
  }

  // Parse a final line without a newline only if it was not overlong.
  if (!skipLine && line.length() > 0) {
    //nv_DumpConfigLine(line);
    nv_parseLine(line);
  }

  epfs_Close(fd);
  return true;
}

/*
 * ======================================================================================================================
 *  nv_saveConfig() - Write a complete replacement to CONFIG.TMP, then rename it.
 *
 *  The prior CONFIG.TXT remains unchanged until epfs_Rename() succeeds.
 *  A failed write leaves the prior configuration intact.
 * ======================================================================================================================
 */
bool nv_saveConfig() {
  if (!eeprom_exists) {
    return(false);
  }

  int fd = epfs_Open(EPFS_TEMP_NAME, O_WRONLY | O_CREAT | O_TRUNC);

  if (fd < 0) {
    Output("EPFS Save:Open:TMP failed");
    return false;
  } 
  bool ok = true;

  // Feather M0 LoRaWAN Board
  ok &= nv_writeLine(fd, "lw_mode=%s\n",      scv.lw_mode.c_str());
  ok &= nv_writeLine(fd, "lw_sf=%d\n",        scv.lw_sf);
  ok &= nv_writeLine(fd, "lw_txpwr=%d\n",     scv.lw_txpwr);
  // OTAA
  ok &= nv_writeLine(fd, "otaa_joineui=%s\n", scv.otaa_joineui.c_str());
  ok &= nv_writeLine(fd, "otaa_deveui=%s\n",  scv.otaa_deveui.c_str());
  ok &= nv_writeLine(fd, "otaa_appkey=%s\n",  scv.otaa_appkey.c_str());
  // ABP
  ok &= nv_writeLine(fd, "abp_devaddr=%s\n",  scv.abp_devaddr.c_str());
  ok &= nv_writeLine(fd, "abp_nwkskey=%s\n",  scv.abp_nwkskey.c_str());
  ok &= nv_writeLine(fd, "abp_appskey=%s\n",  scv.abp_appskey.c_str());

  // Feather M0 WiFi Board
  ok &= nv_writeLine(fd, "wifi_ssid=%s\n",    scv.wifi_ssid.c_str());
  ok &= nv_writeLine(fd, "wifi_pw=%s\n",      scv.wifi_pw.c_str());
  ok &= nv_writeLine(fd, "wifi_wepkey=%s\n",  scv.wifi_wepkey.c_str());
  // Log Server - For example a CHORDS web server
  ok &= nv_writeLine(fd, "ls_fqdn=%s\n",      scv.ls_fqdn.c_str());
  ok &= nv_writeLine(fd, "ls_port=%d\n",      scv.ls_port);
  ok &= nv_writeLine(fd, "ls_urlpath=%s\n",   scv.ls_urlpath.c_str());
  ok &= nv_writeLine(fd, "ls_apikey=%s\n",    scv.ls_apikey.c_str());
  ok &= nv_writeLine(fd, "ls_instr_id=%d\n",  scv.ls_instr_id);
  // Info Server
  ok &= nv_writeLine(fd, "is_fqdn=%s\n",      scv.is_fqdn.c_str());
  ok &= nv_writeLine(fd, "is_port=%d\n",      scv.is_port);
  ok &= nv_writeLine(fd, "is_urlpath=%s\n",   scv.is_urlpath.c_str());
  ok &= nv_writeLine(fd, "is_apikey=%s\n",    scv.is_apikey.c_str());
  // Time Server
  ok &= nv_writeLine(fd, "ts_fqdn=%s\n",      scv.ts_fqdn.c_str());

  // Feather M0 Board with NoteCard
  ok &= nv_writeLine(fd, "nc_debug=%d\n",     scv.nc_debug);
  ok &= nv_writeLine(fd, "nc_product=%s\n",   scv.nc_product.c_str());
  ok &= nv_writeLine(fd, "nc_mode=%s\n",      scv.nc_mode.c_str());
  ok &= nv_writeLine(fd, "nc_outbound=%d\n",  scv.nc_outbound);
  ok &= nv_writeLine(fd, "nc_inbound=%d\n",   scv.nc_inbound);
  ok &= nv_writeLine(fd, "nc_dfu=%d\n",       scv.nc_dfu);
  ok &= nv_writeLine(fd, "nc_doa=%d\n",       scv.nc_doa);
  ok &= nv_writeLine(fd, "nc_method=%s\n",    scv.nc_method.c_str());
  // ok &= nv_writeLine(fd, "nc_qo_obs=%s\n",    scv.nc_qo_obs.c_str());
  // ok &= nv_writeLine(fd, "nc_qo_info=%s\n",   scv.nc_qo_info.c_str());
  // ok &= nv_writeLine(fd, "nc_qi_doa=%s\n",    scv.nc_qi_doa.c_str());
  // ok &= nv_writeLine(fd, "nc_rr=%d\n",        scv.nc_rr);
  ok &= nv_writeLine(fd, "nc_esim_apn=%s\n",  scv.nc_esim_apn.c_str());
  ok &= nv_writeLine(fd, "nc_esim_method=%s\n",scv.nc_esim_method.c_str());
  ok &= nv_writeLine(fd, "nc_wifi_ssid=%s\n"  ,scv.nc_wifi_ssid.c_str());
  ok &= nv_writeLine(fd, "nc_wifi_pw=%s\n"    ,scv.nc_wifi_pw.c_str());

  // Instruments
  ok &= nv_writeLine(fd, "aqs=%d\n",          scv.aqs);
  ok &= nv_writeLine(fd, "wind=%d\n",         scv.wind);
  ok &= nv_writeLine(fd, "rg1=%d\n",          scv.rg1);
  ok &= nv_writeLine(fd, "op1=%s\n",          scv.op1.c_str());
  ok &= nv_writeLine(fd, "op2=%s\n",          scv.op2.c_str());
  ok &= nv_writeLine(fd, "ds_baseline=%d\n",  scv.ds_baseline);

  // System Configuration
  ok &= nv_writeLine(fd, "daily_reboot=%d\n", scv.daily_reboot);
  ok &= nv_writeLine(fd, "obsi=%d\n",         scv.obsi);
  ok &= nv_writeLine(fd, "rtro_hour=%d\n",    scv.rtro_hour);
  ok &= nv_writeLine(fd, "rtro_minute=%d\n",  scv.rtro_minute);
  ok &= nv_writeLine(fd, "elevation=%d\n",    scv.elevation);
  ok &= nv_writeLine(fd, "lat_deg=%.6f\n",    scv.lat_deg);
  ok &= nv_writeLine(fd, "lon_deg=%.6f\n",    scv.lon_deg);

  // Commit buffered file data and filesystem metadata.
  if (epfs_Sync(fd) < 0) {
    ok = false;
  }

  if (!ok) {
    epfs_Close(fd);
    epfs_Unlink(EPFS_TEMP_NAME);
    Output("EPFS Save:Write:TMP failed");
    return false;
  }

  if (epfs_Close(fd) < 0) {
    epfs_Unlink(EPFS_TEMP_NAME);
    Output("EPFS Save:Close:TMP failed");
    return false;
  }

  if (epfs_Rename(EPFS_TEMP_NAME, EPFS_CONFIG_NAME) < 0) {
    epfs_Unlink(EPFS_TEMP_NAME);
    Output("EPFS Save:Rename:TMP failed");
    return false;
  }
  return (true);
}

/*
 * ======================================================================================================================
 * saveConfig() - save system config variables to nv and sd 
 * =======================================================================================================================
 */
bool saveConfig() {
  const bool nvOk = nv_saveConfig();
  const bool sdOk = sd_saveConfig();
  return nvOk && sdOk;
}

/*
 * ======================================================================================================================
 * nv_printCfg()
 * =======================================================================================================================
 */
void nv_printCfg() {
    struct NvItem {
        const char* key;
        const char* fmt;
        const void* ptr;
    };

    NvItem items[] = {
        // Feather M0 LoRaWAN Board
        {"lw_mode", "%s",       scv.lw_mode.c_str()},
        {"lw_sf", "%d",         &scv.lw_sf},
        {"lw_txpwr", "%d",      &scv.lw_txpwr},
        // OTAA
        {"otaa_joineui", "%s",  scv.otaa_joineui.c_str()},
        {"otaa_deveui", "%s",   scv.otaa_deveui.c_str()},
        {"otaa_appkey", "%s",   scv.otaa_appkey.c_str()},
        // ABP
        {"abp_devaddr", "%s",   scv.abp_devaddr.c_str()},
        {"abp_nwkskey", "%s",   scv.abp_nwkskey.c_str()},
        {"abp_appskey", "%s",   scv.abp_appskey.c_str()},

        // Feather M0 WiFi Board
        {"wifi_ssid", "%s",     scv.wifi_ssid.c_str()},
        {"wifi_pw", "%s",       scv.wifi_pw.c_str()},
        {"wifi_wepkey", "%s",   scv.wifi_wepkey.c_str()},

        // Log Server - For example a CHORDS web server
        {"ls_fqdn", "%s",       scv.ls_fqdn.c_str()},
        {"ls_port", "%d",       &scv.ls_port},
        {"ls_urlpath", "%s",    scv.ls_urlpath.c_str()},
        {"ls_apikey", "%s",     scv.ls_apikey.c_str()},
        {"ls_instr_id", "%d",   &scv.ls_instr_id},

        // Info Server
        {"is_fqdn", "%s",       scv.is_fqdn.c_str()},
        {"is_port", "%d",       &scv.is_port},
        {"is_urlpath", "%s",    scv.is_urlpath.c_str()},
        {"is_apikey", "%s",     scv.is_apikey.c_str()},

        // Time Server
        {"ts_fqdn", "%s",       scv.ts_fqdn.c_str()},

        // Feather M0 Board with NoteCard
        {"nc_debug", "%d",      &scv.nc_debug},
        {"nc_product", "%s",    scv.nc_product.c_str()},
        {"nc_mode", "%s",       scv.nc_mode.c_str()},
        {"nc_outbound", "%d",   &scv.nc_outbound},
        {"nc_inbound", "%d",    &scv.nc_inbound},
        {"nc_dfu", "%d",        &scv.nc_dfu},
        {"nc_doa", "%d",        &scv.nc_doa},
        {"nc_method", "%s",     scv.nc_method.c_str()},
        {"nc_qo_obs", "%s",     scv.nc_qo_obs.c_str()},
        {"nc_qo_info", "%s",    scv.nc_qo_info.c_str()},
        {"nc_qi_doa", "%s",     scv.nc_qi_doa.c_str()},
        {"nc_rr", "%d",         &scv.nc_rr},
        {"nc_esim_apn", "%s",   scv.nc_esim_apn.c_str()},
        {"nc_esim_method", "%s",scv.nc_esim_method.c_str()},
        {"nc_wifi_ssid", "%s",  scv.nc_wifi_ssid.c_str()},
        {"nc_wifi_pw", "%s",    scv.nc_wifi_pw.c_str()},

        // Instruments
        {"aqs", "%d",           &scv.aqs},
        {"wind", "%d",          &scv.wind},
        {"rg1", "%d",           &scv.rg1},
        {"op1", "%s",           scv.op1.c_str()},
        {"op2", "%s",           scv.op2.c_str()},
        {"ds_baseline", "%d",   &scv.ds_baseline},

        // System Configuration
        {"daily_reboot", "%d",  &scv.daily_reboot},
        {"obsi", "%d",          &scv.obsi},
        {"rtro_hour", "%d",     &scv.rtro_hour},
        {"rtro_minute", "%d",   &scv.rtro_minute},
        {"elevation", "%d",     &scv.elevation},
        {"lat_deg", "%f",       &scv.lat_deg},
        {"lon_deg", "%f",       &scv.lon_deg},
    };

    Output("NV Settings:");

    for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); i++) {
        if (strcmp(items[i].fmt, "%s") == 0) {
            snprintf(msgbuf, sizeof(msgbuf), "%s=%s", items[i].key, (const char*)items[i].ptr);
        }
        else if (strcmp(items[i].fmt, "%ld") == 0) {
            snprintf(msgbuf, sizeof(msgbuf), "%s=%ld", items[i].key, *(const long*)items[i].ptr);
        }
        else if (strcmp(items[i].fmt, "%d") == 0) {
            snprintf(msgbuf, sizeof(msgbuf), "%s=%d", items[i].key, *(const int*)items[i].ptr);
        }
        else if (strcmp(items[i].fmt, "%f") == 0) {
            snprintf(msgbuf, sizeof(msgbuf), "%s=%f", items[i].key, *(const float*)items[i].ptr);
        }

        Output(msgbuf);
    }
    Output("NV Settings:End");
}

/*
 * ======================================================================================================================
 * nv_deleteConfigIfCnvFileExists()
 * If the SD card is available and CNV.TXT exists:
 *   1. EEPROM EPFS is formated, zeroing out all bytes
 *   2. Delete CNV.TXT after successful config deletion
 *
 * Will hang until watchdog reboots us and blink when all requested deletes succeeded.
 * ======================================================================================================================
 */
bool nv_deleteConfigIfCnvFileExists() {
    // Do not access the SD filesystem unless it is known to be available.
    if (!SD_exists) {
        Output("CNV:SD Not Available, config files retained");
        return false;
    }

    // CNV.TXT is the trigger file.
    if (!SD.exists(SD_cnv_file)) {
      snprintf(msgbuf, sizeof(msgbuf), "CNV:%s NF, NV files retained", SD_cnv_file);
      Output(msgbuf);
      return false;
    }

    snprintf(msgbuf, sizeof(msgbuf), "CNV:%s Found, deleting config files", SD_cnv_file);
    Output(msgbuf);

    // Keep CNV.TXT in place if deleting the config files failed.
    if (!epfs_Format()) {
        Output("CNV:Format failed, retaining CNV.TXT");
        return false;
    }

    // Config files were successfully deleted; remove the trigger file too.
    if (SD.remove(SD_cnv_file)) {
        snprintf(msgbuf, sizeof(msgbuf), "NVCF:%s Deleted", SD_cnv_file);
        Output(msgbuf);
        Output("CNV: Entering loop - Powerdown device");
        while (true) {
          // HeartBeat();
          // delay (750);
          Blink(2, 500);
          Blink(2, 750);
        }
    }

    snprintf(msgbuf, sizeof(msgbuf), "CNV:%s Delete Failed", SD_cnv_file);
    Output(msgbuf);

    return false;
}

/*
 * ======================================================================================================================
 * nv_scv_check() - future check for QC
 * =======================================================================================================================
 */
void nv_scv_check() {
    Output ("SCV Check");
    // Instruments
    if ((scv.aqs != 0) && (scv.aqs != 1)) {
      scv.aqs = 0;
      Output("AQS:Set to Disable");
    }
    if ((scv.wind != 0) && (scv.wind != 1)) {
      scv.wind = 1;
      Output("WIND:Set to Enable");
    }
    if ((scv.rg1 != 0) && (scv.rg1 != 1)) {
      Output("RG1:Set to Enable");
      scv.rg1 = 1;
    }
    if (scv.ds_baseline<0) {
      Output("DS_BASELINE:Set to 0");
      scv.ds_baseline = 0;
    }

    // Option Pin 1 A4
    int op1_state = nv_op1_state();
    if (op1_state == OP1_STATE_DIST_5M) {
        dg_resolution_adjust = 1.25;
      pinMode(OP1_PIN, INPUT);
    }
    else if (op1_state == OP1_STATE_DIST_10M) {
      dg_resolution_adjust = 2.5;
      pinMode(OP1_PIN, INPUT);
    }
    else if ((op1_state == OP1_STATE_RAW) ||  (op1_state == OP1_STATE_RAIN)) {
      pinMode(OP1_PIN, INPUT);
    }
  
    // Option Pin 2 A5
    int op2_state = nv_op2_state();
    if ((op2_state == OP2_STATE_RAW) || (op2_state == OP2_STATE_VOLTAIC)) {
      pinMode(OP2_PIN, INPUT);
    }

    // System Configuration
    if ((scv.daily_reboot < 0) || (scv.daily_reboot > 24)) { // though this is a loop counter 
      Output("daily_reboot:Set to default");
      scv.daily_reboot = DEFAULT_DAILY_REBOOT;
    }

    if ((scv.obsi < 1) || (scv.obsi > 60)) {
      sprintf (Buffer32Bytes, "OBSI:%dm Now:1m", scv.obsi);
      Output(Buffer32Bytes);
      scv.obsi = 1; 
    }

    if ((scv.rtro_hour < 0) || (scv.rtro_hour > 23) ||
        ((scv.rtro_minute != 0) && (scv.rtro_minute != 15) && (scv.rtro_minute != 30) && (scv.rtro_minute != 45))) {
      Output("RTRO:Set to 0:0");
      scv.rtro_hour = 0;
      scv.rtro_minute = 0;
    }

    if ((scv.elevation != QC_ERR_ELEV) &&
       ((scv.elevation < QC_MIN_ELEV || scv.elevation > QC_MAX_ELEV))) {
      Output("ELEV:Set to -999");
      scv.elevation = QC_ERR_ELEV;
    }
}

/*
 * ======================================================================================================================
 * nv_scv_init() - address where we are getting our configuration from.
 * =======================================================================================================================
 */
void nv_scv_init() {
  if (eeprom_exists) {  
    epfs_Begin();
    epfs_PrintInfo();
  }

  if (!SD_exists) {
    if (!nv_loadConfig()) {
      Output ("SCV Using Defaults");
      nv_scv_check();
      if (eeprom_exists) {
        (nv_saveConfig()) ? Output ("EPSaveNew:OK") : Output ("EPSaveNew:ERR");
      }
    }
    else {
      nv_scv_check();
    }
  }
  else { // We have SD card enabled 
    nv_deleteConfigIfCnvFileExists(); // Check if CNV.TXT exists on SD. Delete NV configs and loop forever blinking led
    SD_CheckN2SandSetSSB();

    if (sd_loadConfig()) {
      // We read SD CFG File. Now check config and update EP
      nv_scv_check();
      if (eeprom_exists) {
        (nv_saveConfig()) ? Output ("EPUpdate:OK") : Output ("EPUpdate:ERR");
      }
    }
    else {
      // SD is online and no SD CFG File. See if we have NV CFG to use.
      if (nv_loadConfig()) {
        // We read the NV config so Make a new SD config file.
        nv_scv_check();
        (sd_saveConfig()) ? Output ("SDSaveNew:OK") : Output ("SDSaveNew:ERR");
      }
      else {
        // We have no config file in SD or EP. Use defaults and save to these locations
        nv_scv_check();
        (sd_saveConfig()) ? Output ("SDSaveNew:OK") : Output ("SDSaveNew:ERR");
        if (eeprom_exists) {
          (nv_saveConfig()) ? Output ("EPSaveNew:OK") : Output ("EPSaveNew:ERR");
        }
      }
    }
  }
  // Show what we are running with
  nv_printCfg();
}
