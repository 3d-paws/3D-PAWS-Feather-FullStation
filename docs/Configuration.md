# Configuration
[←Top](../README.md)<BR>

On the SD card, create a file named CONFIG.TXT. If you are setting up Wi-Fi, you do not need to configure LoRaWAN, and vice versa. Under LoRaWAN configuration section you have the choices of configuring for OTAA (Over The Air Authentication) or ABP (Authentication By Personalization). The selection of which is set by "lw_mode" seting of (0=OTAA, 1=ABP).

Example CONFIG.TXT file.
```
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
info_server_port=443
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
# 0 = No sensor (Pin in use if pm25aqi air quality dectected)
# 1 = raw (op1r - average 5 samples spaced 10ms)
# 2 = 2nd rain gauge (rg2)
# 5 = 5m distance sensor (ds, dsr)
# 10 = 10m distance sensor (ds, dsr)
op1=0

# OptionPin 2 - pin A5
# 0 = No sensor
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
```

At initialization:
- I2C sensors are auto detected. For example, if the AS5600 wind direction sensor is not detected, wind-related measurements and dependent observations will not be performed or reported.
- Software will discover the Afafruit board type (LoRa, WiFi) and configure accordingly.