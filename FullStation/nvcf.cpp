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
static const char* NV_CONFIG_FILE = "/CONFIG.TXT";
static const char* NV_TEMP_FILE   = "/CONFIG.TMP";

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

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
    else if (key == "nc_product")   scv.nc_product = val;
    else if (key == "nc_mode")      scv.nc_mode = val;
    else if (key == "nc_outbound")  scv.nc_outbound = val.toInt();
    else if (key == "nc_inbound")   scv.nc_inbound = val.toInt();
    else if (key == "nc_method")    scv.nc_method = val;
    // else if (key == "nc_qo_obs")    scv.nc_qo_obs = val;
    // else if (key == "nc_qo_info")   scv.nc_qo_info = val;
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
 * sd_writeAll() -- Use FsFile with SdFat v2 / SdFs.
 * =======================================================================================================================
 */
static ssize_t sd_writeAll(FsFile& fp, const char* data, size_t len) {
    size_t total = 0;

    while (total < len) {
        size_t n = fp.write(
            reinterpret_cast<const uint8_t*>(data + total),
            len - total
        );

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
    File fp = SD.open(NV_CONFIG_FILE, FILE_READ);
    if (!fp) {
        sprintf(msgbuf, "SDLC:%s NF, Using Defaults", NV_CONFIG_FILE);
        Output(msgbuf);
        return false;
    }
    sprintf(msgbuf, "SDLC:%s Found", NV_CONFIG_FILE);
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
 * sd_saveConfig()
 * =======================================================================================================================
 */
bool sd_saveConfig() {
  if (SD_exists) {
    File fp = SD.open(NV_TEMP_FILE, O_WRONLY | O_CREAT | O_TRUNC);

    if (!fp) {
        return false;
    }

    bool ok = true;

    // Feather M0 LoRaWAN Board
    ok &= sd_writeLine(fp, "lw_mode=%s\n",      scv.lw_mode);
    ok &= sd_writeLine(fp, "lw_sf=%d\n",        scv.lw_sf);
    ok &= sd_writeLine(fp, "lw_txpwr=%d\n",     scv.lw_txpwr);
    // OTAA
    ok &= sd_writeLine(fp, "otaa_joineui=%s\n", scv.otaa_joineui);
    ok &= sd_writeLine(fp, "otaa_deveui=%s\n",  scv.otaa_deveui);
    ok &= sd_writeLine(fp, "otaa_appkey=%s\n",  scv.otaa_appkey);
    // ABP
    ok &= sd_writeLine(fp, "abp_devaddr=%s\n",  scv.abp_devaddr);
    ok &= sd_writeLine(fp, "abp_nwkskey=%s\n",  scv.abp_nwkskey);
    ok &= sd_writeLine(fp, "abp_appskey=%s\n",  scv.abp_appskey);

    // Feather M0 WiFi Board
    ok &= sd_writeLine(fp, "wifi_ssid=%s\n",    scv.wifi_ssid);
    ok &= sd_writeLine(fp, "wifi_pw=%s\n",      scv.wifi_pw);
    ok &= sd_writeLine(fp, "wifi_wepkey=%s\n",  scv.wifi_wepkey);
    // Log Server - For example a CHORDS web server
    ok &= sd_writeLine(fp, "ls_fqdn=%s\n",      scv.ls_fqdn);
    ok &= sd_writeLine(fp, "ls_port=%d\n",      scv.ls_port);
    ok &= sd_writeLine(fp, "ls_urlpath=%s\n",   scv.ls_urlpath);
    ok &= sd_writeLine(fp, "ls_apikey=%s\n",    scv.ls_apikey);
    ok &= sd_writeLine(fp, "ls_instr_id=%d\n",  scv.ls_instr_id);
    // Info Server
    ok &= sd_writeLine(fp, "is_fqdn=%s\n",      scv.is_fqdn);
    ok &= sd_writeLine(fp, "is_port=%d\n",      scv.is_port);
    ok &= sd_writeLine(fp, "is_urlpath=%s\n",   scv.is_urlpath);
    ok &= sd_writeLine(fp, "is_apikey=%s\n",    scv.is_apikey);
    // Time Server
    ok &= sd_writeLine(fp, "ts_fqdn=%s\n",      scv.ts_fqdn);

    // Feather M0 Board with NoteCard
    ok &= sd_writeLine(fp, "nc_product=%s\n",   scv.nc_product);
    ok &= sd_writeLine(fp, "nc_mode=%s\n",      scv.nc_mode);
    ok &= sd_writeLine(fp, "nc_outbound=%d\n",  scv.nc_outbound);
    ok &= sd_writeLine(fp, "nc_inbound=%d\n",   scv.nc_inbound);
    ok &= sd_writeLine(fp, "nc_method=%s\n",    scv.nc_method);
    // ok &= sd_writeLine(fp, "nc_qo_obs=%s\n",    scv.nc_qo_obs);
    // ok &= sd_writeLine(fp, "nc_qo_info=%s\n",   scv.nc_qo_info);
    // ok &= sd_writeLine(fp, "nc_rr=%d\n",        scv.nc_rr);
    ok &= sd_writeLine(fp, "nc_esim_apn=%s\n",  scv.nc_esim_apn);
    ok &= sd_writeLine(fp, "nc_esim_method=%s\n",scv.nc_esim_method);
    ok &= sd_writeLine(fp, "nc_wifi_ssid=%s\n"  ,scv.nc_wifi_ssid);
    ok &= sd_writeLine(fp, "nc_wifi_pw=%s\n"    ,scv.nc_wifi_pw);

    // Instruments
    ok &= sd_writeLine(fp, "aqs=%d\n",          scv.aqs);
    ok &= sd_writeLine(fp, "wind=%d\n",         scv.wind);
    ok &= sd_writeLine(fp, "rg1=%d\n",          scv.rg1);
    ok &= sd_writeLine(fp, "op1=%s\n",          scv.op1);
    ok &= sd_writeLine(fp, "op2=%s\n",          scv.op2);
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
        SD.remove(NV_TEMP_FILE);
        return false;
    }

    SD.remove(NV_CONFIG_FILE);
    return SD.rename(NV_TEMP_FILE, NV_CONFIG_FILE);
  }
  else {
    return false;
  }
}

/*
 * ======================================================================================================================
 * saveConfig() - save system config variables to nv and sd 
 * =======================================================================================================================
 */
bool saveConfig() {
    // return ep_saveConfig() && sd_saveConfig();
    return sd_saveConfig();
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
        {"nc_product", "%s",    scv.nc_product.c_str()},
        {"nc_mode", "%s",       scv.nc_mode.c_str()},
        {"nc_outbound", "%d",   &scv.nc_outbound},
        {"nc_inbound", "%d",    &scv.nc_inbound},
        {"nc_method", "%s",     scv.nc_method.c_str()},
        {"nc_qo_obs", "%s",     scv.nc_qo_obs.c_str()},
        {"nc_qo_info", "%s",    scv.nc_qo_info.c_str()},
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
            snprintf(msgbuf, sizeof(msgbuf), "  %s:%s", items[i].key, (const char*)items[i].ptr);
        }
        else if (strcmp(items[i].fmt, "%ld") == 0) {
            snprintf(msgbuf, sizeof(msgbuf), "  %s:%ld", items[i].key, *(const long*)items[i].ptr);
        }
        else if (strcmp(items[i].fmt, "%d") == 0) {
            snprintf(msgbuf, sizeof(msgbuf), "  %s:%d", items[i].key, *(const int*)items[i].ptr);
        }
        else if (strcmp(items[i].fmt, "%f") == 0) {
            snprintf(msgbuf, sizeof(msgbuf), "  %s:%f", items[i].key, *(const float*)items[i].ptr);
        }

        Output(msgbuf);
    }
}

/*
 * ======================================================================================================================
 * nv_readConfig()
 * =======================================================================================================================
 */
void nv_readConfig() {
  if (sd_loadConfig()) {
    nv_printCfg();
    
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
        dg_resolution_adjust = 5;
      pinMode(OP1_PIN, INPUT);
    }
    else if (op1_state == OP1_STATE_DIST_10M) {
      dg_resolution_adjust = 10;
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

    if ((scv.obsi != 1) &&
        (scv.obsi != 5) && 
        (scv.obsi != 6) && 
        (scv.obsi != 10) &&
        (scv.obsi != 15) &&
        (scv.obsi != 20) &&
        (scv.obsi != 30)) {
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
}

#ifdef NOWAY
/*
 * ======================================================================================================================
 * nv_writeAll()
 * =======================================================================================================================
 */
static ssize_t nv_writeAll(int fd, const char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = write(fd, data + total, len - total);
        if (n < 0) return -1;
        total += (size_t)n;
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

    if (n < 0 || (size_t)n >= sizeof(buf)) return false;
    return nv_writeAll(fd, buf, (size_t)n) == n;
}
/*
 * ======================================================================================================================
 * nv_loadConfig()
 * =======================================================================================================================
 */
bool nv_loadConfig() {
    int fd = open(NV_CONFIG_FILE, O_RDONLY);
    if (fd < 0) {
      sprintf(msgbuf, "NVLC:%s NF, Using Defaults", NV_CONFIG_FILE);
      Output(msgbuf);
      return false;
    }
    sprintf(msgbuf, "NVLC:%s Found", NV_CONFIG_FILE);
    Output(msgbuf);

    String line;
    char ch;

    while (true) {
        ssize_t n = read(fd, &ch, 1);
        if (n <= 0) break;

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

    close(fd);
    return true;
}
/*
 * ======================================================================================================================
 * nv_saveConfig()
 * =======================================================================================================================
 */
bool nv_saveConfig() {
    int fd = open(NV_TEMP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        return false;
    }

    bool ok = true;
    ok &= nv_writeLine(fd, "aqs=%d\n", scv.aqs);
    ok &= nv_writeLine(fd, "wind=%d\n", scv.wind);
    ok &= nv_writeLine(fd, "rg1_enable=%d\n", scv.rg1);
    ok &= nv_writeLine(fd, "op1=%d\n", scv.op1);
    ok &= nv_writeLine(fd, "op1d5m=%d\n", scv.op1d5m);
    ok &= nv_writeLine(fd, "op2=%d\n", scv.op2);
    ok &= nv_writeLine(fd, "elevation=%d\n", scv.elevation);
    ok &= nv_writeLine(fd, "rtro_hour=%d\n", scv.rtro_hour);
    ok &= nv_writeLine(fd, "rtro_minute=%d\n", scv.rtro_minute);
    ok &= nv_writeLine(fd, "daily_reboot=%d\n", scv.daily_reboot);
    ok &= nv_writeLine(fd, "txi=%d\n", scv.txi);
    ok &= nv_writeLine(fd, "obsi=%d\n", scv.obsi);
    ok &= nv_writeLine(fd, "aes_pkey=%s\n", scv.aes_pkey.c_str());
    ok &= nv_writeLine(fd, "aes_myiv=%ld\n", scv.aes_myiv);
    ok &= nv_writeLine(fd, "lora_unitid=%d\n", scv.lora_unitid);
    ok &= nv_writeLine(fd, "lora_txpower=%d\n", scv.lora_txpower);
    ok &= nv_writeLine(fd, "lora_freq=%d\n", scv.lora_freq);
    ok &= nv_writeLine(fd, "wifi_auth=%s\n", scv.wifi_auth.c_str());
    ok &= nv_writeLine(fd, "wifi_ssid=%s\n", scv.wifi_ssid.c_str());
    ok &= nv_writeLine(fd, "wifi_pw=%s\n", scv.wifi_pw.c_str());
#ifdef ENABLE_Evapotranspiration
    ok &= nv_writeLine(fd, "lat_deg=%.6f\n", scv.lat_deg);
    ok &= nv_writeLine(fd, "lon_deg=%.6f\n", scv.lon_deg);
    ok &= nv_writeLine(fd, "albedo=%.3f\n", scv.albedo);
    ok &= nv_writeLine(fd, "crop_kc=%.3f\n", scv.crop_kc);
    ok &= nv_writeLine(fd, "sr_cal=%.3f\n", scv.sr_cal);
    ok &= nv_writeLine(fd, "sr_dark_offset=%.3f\n", scv.sr_dark_offset);
#endif

    fsync(fd);
    close(fd);

    if (!ok) {
        remove(NV_TEMP_FILE);
        return false;
    }

    remove(NV_CONFIG_FILE);
    return rename(NV_TEMP_FILE, NV_CONFIG_FILE) == 0;
}

/*
 * ======================================================================================================================
 * nv_deleteConfigFiles()
 * Deletes CONFIG.TXT and CONFIG.TMP when present.
 *
 * Returns true only if both files are either successfully deleted or did not exist.
 * ======================================================================================================================
 */
bool nv_deleteConfigFiles() {
    const char* files[] = {
        NV_CONFIG_FILE,
        NV_TEMP_FILE
    };

    bool success = true;

    for (size_t i = 0; i < (sizeof(files) / sizeof(files[0])); i++) {
        const char* fileName = files[i];

        // unlink() deletes the file. ENOENT means it was already absent.
        if (unlink(fileName) == 0) {
            snprintf(msgbuf, sizeof(msgbuf), "NVCF:%s Deleted", fileName);
            Output(msgbuf);
        }
        else if (errno == ENOENT) {
            snprintf(msgbuf, sizeof(msgbuf), "NVCF:%s Not Found", fileName);
            Output(msgbuf);
        }
        else {
            snprintf(msgbuf, sizeof(msgbuf), "NVCF:%s Delete Failed: %s",
                     fileName, strerror(errno));
            Output(msgbuf);
            success = false;
        }
    }

    return success;
}

/*
 * ======================================================================================================================
 * nv_deleteConfigIfCnvFileExists()
 * If the SD card is available and CNV.TXT exists:
 *   1. Delete CONFIG.TXT and CONFIG.TMP from NV
 *   2. Delete CNV.TXT after successful config deletion
 *
 * Returns true only when CNV.TXT was found and all requested deletes succeeded.
 * ======================================================================================================================
 */
bool nv_deleteConfigIfCnvFileExists() {
    // Do not access the SD filesystem unless it is known to be available.
    if (!SD_exists) {
        Output("CNV:SD Not Available, config files retained");
        return false;
    }

    // CNV.TXT is the trigger file.
    if (access(SD_cnv_file, F_OK) != 0) {
        if (errno == ENOENT) {
            snprintf(msgbuf, sizeof(msgbuf), "CNV:%s Not Found, config files retained", SD_cnv_file);
        }
        else {
            snprintf(msgbuf, sizeof(msgbuf), "CNV:%s Check Failed: %s", SD_cnv_file, strerror(errno));
        }
        Output(msgbuf);
        return false;
    }

    snprintf(msgbuf, sizeof(msgbuf), "CNV:%s Found, deleting config files", SD_cnv_file);
    Output(msgbuf);

    // Keep CNV.TXT in place if deleting the config files failed.
    if (!nv_deleteConfigFiles()) {
        Output("CNV:Config delete failed, retaining CNV.TXT");
        return false;
    }

    // Config files were successfully deleted; remove the trigger file too.
    if (unlink(SD_cnv_file) == 0) {
        snprintf(msgbuf, sizeof(msgbuf), "NVCF:%s Deleted", SD_cnv_file);
        Output(msgbuf);
        Output("CNV: HALT-entering infinite loop");
        while (true) {
            HeartBeat();
            delay (750);
        }
    }

    snprintf(msgbuf, sizeof(msgbuf), "CNV:%s Delete Failed: %s", SD_cnv_file, strerror(errno));
    Output(msgbuf);

    return false;
}
#endif