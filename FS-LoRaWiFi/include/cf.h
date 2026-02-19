/*
 * ======================================================================================================================
 *  cf.h - Configuration File Definations
 * ======================================================================================================================
 */

/* 
 * ======================================================================================================================
#
# CONFIG.TXT
#
# Line Length is limited to 60 characters
#123456789012345678901234567890123456789012345678901234567890

# At boot time the software determines if the board is a Feather WiFi or LoRaWAN
# Set below LoRaWAN / WiFi configurations accordingly.

#################################################
# WiFi Feather Set the Below
#################################################
# WiFi Encryption Methods (WEP, WPA/2, Open Network)  
#   WPA/2  set wifi_ssid and wifi_pw
#   WEP    set wifi_ssid, wifi_wepkey
#   Open   set wifi_ssid
wifi_ssid=
wifi_pw=
# WEPKEY can be 40bit (10 HEX Characters) or 128bit WEP key (26 HEX Characters)
wifi_wepkey=

####################################################################
# For HTTPS / Port 443 you need to upload the destination website's 
# cert to the modem. See WiFi Board Preparation documentation.
####################################################################

# Web server aka Chords
webserver=some.domain.com
webserver_port=80
urlpath=/measurements/url_create
# Will be added to the HTML Header as X-API-Key also passed in data as instrument_id
apikey=1234
instrument_id=0

# Information web server, Not Chords
info_server=some.domain.com
info_server_port=80
info_urlpath=/info/log.php
# Will be added to the HTML Header as X-API-Key
info_apikey=1234

# Time Server
ntpserver=pool.ntp.org

#################################################
# LoRaWAN Feather Set the Below
#################################################
# Connect mode to LoRaWAN via (0=OTAA, 1=ABP)
lw_mode=0

# Spreading factor 7=1.75x faster bitrate than 8, 8=3dB more sensitive (-126dBm vs -123dBm) 
lw_sf=7

# Transmit Power (14 - 20) dBm are valid, with higher ones (17–20) drawing more current 
# (~120 mA (0.12 amp) peak at 20 dBm)  but offering better range
lw_txpw=14

############################################
# LoRaWAN OTAA (Over The Air Authentication)
############################################
# Input must be hex and match the length below
#
# Convert lw_joineui and lw_deveui to little endian format.
#     Example 0123456798BACDEF -> EFCDAB8967452301
#
# Note: JoinEUI formally called AppEUI    
# This key is in little endian format      
lw_joineui=EFCDAB8967452301

# This key is in little endian format
lw_deveui=EFCDAB8967452301

# This key is in big endian format
lw_appkey=0123456789ABCDEF01234567890ABCDE

#################################################
# LoRaWAN ABP (Authentication By Personalization)
#################################################
# Input must be hex and match the length below 
lw_devaddr=01234567
lw_nwkskey=0123456789ABCDEF01234567890ABCDE
lw_appskey=0123456789ABCDEF01234567890ABCDE

#################################################
# General Configurations Settings
#################################################
# Observation Period (1,5,6,10,15,20,30)
# 1 minute observation period is the default
obs_period=1

# Rain Gauge (rg1) - pin A3
# Options 0,1
rg1_enable=0

# OptionPin 1 - pin A4
# 0 = No sensor
# 1 = raw (op1r - average 5 samples spaced 10ms)
# 2 = 2nd rain gauge (rg2)
# 5 = 5m distance sensor (ds, dsr)
# 10 = 10m distance sensor (ds, dsr)
op1=0

# OptionPin 2 - pin A5
# 0 = No sensor (Pin in use if pm25aqi air quality dectected)
# 1 = raw (op2r - average 5 samples spaced 10ms)
# 2 = read Voltaic battery voltage (vbv)
op2=0

# Distance sensor baseline. If positive, distance = baseline - ds_median
ds_baseline=0

# Number of hours between daily reboots
# A value of 0 disables this feature
daily_reboot=22

# elevation used for MSLP
elevation=0

 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Define Global Configuration File Variables
 * ======================================================================================================================
 */
#define CF_NAME           "CONFIG.TXT"
#define KEY_MAX_LENGTH    30                // Config File Key Length
#define VALUE_MAX_LENGTH  30                // Config File Value Length
#define LINE_MAX_LENGTH   VALUE_MAX_LENGTH+KEY_MAX_LENGTH+3   // =, CR, LF

// Extern variables

// 0=OTAA, 1=ABP
extern int cf_lw_mode;
// Spreading factor
extern int cf_lw_sf;
// Transmit Power
extern int cf_lw_txpw;
// OTAA
extern char *cf_lw_joineui;
extern char *cf_lw_deveui;
extern char *cf_lw_appkey;
// ABP
extern char *cf_lw_devaddr;
extern char *cf_lw_nwkskey;
extern char *cf_lw_appskey;
// WiFi
extern char *cf_wifi_ssid;
extern char *cf_wifi_pw;
extern char *cf_wifi_wepkey;
// Web Server
extern char *cf_webserver;
extern int  cf_webserver_port;
extern char *cf_urlpath;
extern char *cf_apikey;
extern int  cf_instrument_id;
// Info Server
extern char *cf_info_server;
extern int  cf_info_server_port;
extern char *cf_info_urlpath;
extern char *cf_info_apikey; 
// Time Server
extern char *cf_ntpserver;

extern int cf_rg1_enable;
extern int cf_op1;
extern int cf_op2;
extern int cf_ds_baseline;
extern int cf_obs_period;
extern int cf_daily_reboot;
extern int cf_elevation;

// Function prototypes
void SD_ReadConfigFile();
void SD_ReadElevationFile();