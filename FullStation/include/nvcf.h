/*
 * ======================================================================================================================
 *  nvcf.h - Non Volitile Configuration File Definations
 * ======================================================================================================================
 */
#include <Arduino.h>

/*
 * ======================================================================================================================
 *  Define Global Configuration File Variables
 * ======================================================================================================================
 */
#define DEFAULT_OBS_INTERVAL            1        // 1 minute
#define DEFAULT_DAILY_REBOOT            22       // Set to 0 to disable feature, approx every 22 hours

// The below shows what is in the CONFIG.TXT file
 typedef struct {
    // ========================
    // Feather M0 LoRaWAN Board
    // ========================
    String  lw_mode;          // (OTAA or ABP)
    int     lw_sf=7;          // Spreading factor (7,8)
    int     lw_txpwr=14;      // Transmit Power (2 to 17) db

    // ==================================
    // OTAA (Over The Air Authentication)
    // ==================================
    String  otaa_joineui;               // JoinEUI was formally called AppEUI
                                        // Convert lw_joineui and lw_deveui to little endian format
                                        //  Examples 0123456798BACDEF -> EFCDAB8967452301
                                        //           000000000000001 -> 0100000000000000

    String  otaa_deveui;                // This key is in little endian format
    String  otaa_appkey;                // This key is in big endian format

    // =======================================
    // ABP (Authentication By Personalization)
    // =======================================
                                        // Input must be hex and match the lengths below
    String  abp_devaddr;                // 12345678
    String  abp_nwkskey;                // 12345678901234567890123456789012
    String  abp_appskey;                // 32 HEX characters, 16 Bytes
    
    // ======================================================================================
    // Feather M0 WiFi Board
    //   For HTTPS / Port 443 you need to upload the destination website's cert to the modem.
    //   See WiFi Board Preparation documentation.
    // ======================================================================================
    String  wifi_ssid;
    String  wifi_pw;
    String  wifi_wepkey;                // WEPKEY can be 40bit (10 HEX Characters) or 128bit WEP key (26 HEX Characters)
    // Log Server - For example a CHORDS web server
    String  ls_fqdn;
    int     ls_port=80;
    String  ls_urlpath;
    String  ls_apikey;
    int     ls_instr_id=0;
    // Info Server
    String  is_fqdn;
    int     is_port=80;
    String  is_urlpath;
    String  is_apikey;
    // Time Server
    String  ts_fqdn;

    // ==============================
    // Feather M0 Board with NoteCard
    // ==============================
    String  nc_product;       // Notecard's association to a Product (edu.ucar.wnicewonger:test_3dpaws)
    String  nc_mode;          // Notecard's cloud connection mode (continuous,periodic)
                              //   pm        Periodic Mode
                              //   cm        Continuous Mode 
                              //   mm        Minimum Mode - we are responsible for calling hub.sync, aka synchronize with Notehub now

                              // Notecard’s Minimum Mode does not maintain an always-on cellular connection. 
                              // It connects when explicitly told to synchronize, rather than staying connected or 
                              // connecting on a periodic schedule.

                              // If Periodic Mode set and "nc_outbound" is set to "0". You are causing notecard to disconnect
                              // and reconnect to the cell/wifi network on each observation. 

#ifdef NOWAY
if (!NOTECARD_CONTINUOUS_MODE) {
    // In periodic mode, ask the Notecard to connect and sync right now rather than waiting
    // for the next NOTECARD_OUTBOUND_MINUTES check-in.
    JAddBoolToObject(req, "sync", true);
}
#endif



    int     nc_outbound=10;   // Schedule Notecard's sending of queued .qo Notes and send on the scheduled interval
                              //   0 disables scheduled outbound synchronization, we are responsible for calling hub.sync
                              //   1 to 43,200 minutes aka from 1 minute to 30 days
    int     nc_inbound=60;    // Schedule Notecard's periodic check for inbound commands & configurations
                              //   0 Disables scheduled inbound polling. The Notecard will not periodically check Notehub for inbound Notes, 
                              //     environment-variable changes, configuration changes, or pending DFU/host-firmware-update work. 
                              //     Inbound content can still arrive when the device happens to establish an outbound sync.
                              //   1 to 43,200 minutes aka from 1 minute to 30 days
    String  nc_method="c";    // Card Tansport Method
                              //  c     cell
                              //  w     wifi
                              //  n     ntn   -- Not supporting (Non-Terrestrial Network only) because a template needs to be set first via cell or wireless to nothub.
                              //  cn    cell-ntn
                              //  wc    wifi-cell
                              //  wn    wifi-ntn
                              //  wcn   wifi-cell-ntn            

                                  // We have 2 outbound queues because each will have their own template.
                                  // At NoteHub, these queues will be associated with routes. A route (aka webhook) forwards the data to the ingest service.
    String  nc_qo_obs ="obs.qo";  // Queue out file name for observations.   - Not in config file
    String  nc_qo_info="info.qo"; // Queue out file name for info messages.  - Not in config file 
    int     nc_qo_port_obs  = 10; // Port used for templated - Not in config file  
    int     nc_qo_port_info = 11; // Port used for templated - Not in config file   

    int     nc_rr=1;          // Host to notecard via i2c retry count. - Not in config file

    String  nc_esim_apn;      // External Sim APN (your.carrier.apn)
    String  nc_esim_method;   // External Sim order methods
                              //  p      primary            Use the internal embedded Blues SIM only (default)
                              //  s      secondary          Use the external SIM only, provided an external SIM is physically detected.
                              //  dps    dual-primary-secondary    Try internal SIM first; after cellular registration failures, switch to external SIM.
                              //  dsp    dual-secondary-primary    Try external SIM first; after cellular registration failures, switch to internal Blues SIM.

    String  nc_wifi_ssid;     // StationWiFi
    String  nc_wifi_pw;       // secret

    // ===========
    // Instruments
    // ===========
    int     aqs = 0;          // 0=Full Station, 1=Aquality Station
    int     wind = 1;         // 0=Disabled, 1=Enabled
    int     rg1 = 1;          // 0=Disabled, 1=Enabled
    String  op1;              // null,raw,d5m,d10m,rg2      
    String  op2;              // null,raw,voltaic
    int     ds_baseline=0;    // OP1 Distance Sensor milimeters from sensor to observation surface.

    // ====================
    // System Configuration
    // ====================
    int     daily_reboot = DEFAULT_DAILY_REBOOT;          // Daily Reboot Hours, Reboot Every N hours
    int     obsi      = DEFAULT_OBS_INTERVAL;             // observation interval, time between observations 1-60 minutes - was obs_period=0
    int     rtro_hour = 0;                                // Rain Total Rollover Hour 0-23
    int     rtro_minute=0;                                // Rain Total Rollover Minute 0,15,30,45
    int     elevation = -999;                             // Station Elevation in m. 
    float   lat_deg   = 0.0f;                             // If set will report in INFO if no GPS lat and lon
    float   lon_deg   = 0.0f;                             // If set will report in INFO if no GPS lat and lon
} SCV; // System Configuration Variables

// Extern variables
extern SCV scv;
extern bool nv_config_enabled;

// Function prototypes
bool nv_loadConfig();
bool nv_saveConfig();
bool sd_loadConfig();
bool sd_saveConfig();
bool saveConfig();
void nv_printCfg();
bool nv_deleteConfigFiles();
bool nv_deleteConfigIfCnvFileExists();
int nv_op1_state();
int nv_op2_state();
void nv_readConfig();