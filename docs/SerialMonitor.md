# Serial Monitor
[←Top](../README.md)<BR>

WiFi Example Start Up output
```
17:35:58.493 -> OLED:Enabled
17:35:58.530 -> SC:Enabled
17:35:58.530 -> SER:OK
17:36:00.555 -> Copyright [2026] [University Corporation for Atmospheric Research]
17:36:00.555 -> FSLW-260211
17:36:00.590 -> DevID:73eee600304d98c9fc7fc5f4
17:36:00.590 -> REBOOTPN SET
17:36:00.626 -> HEARTBEAT SET
17:36:00.883 -> SD:INIT
17:36:00.918 -> SD:Online
17:36:00.954 -> SD:OBS DIR Exists
17:36:00.988 -> CF:CF:lw_mode=[0]
17:36:01.022 -> CF:CF:lw_sf=[7]
17:36:01.056 -> CF:CF:lw_txpw=[14]
17:36:01.090 -> CF:lw_joineui=[0100000000000000]
17:36:01.128 -> CF:lw_deveui=[955207D07ED5B370]
17:36:01.201 -> CF:lw_appkey=[NOT_THE_REAL_API_KEY]
17:36:01.236 -> CF:lw_devaddr=[]
17:36:01.311 -> CF:lw_nwkskey=[]
17:36:01.345 -> CF:lw_appskey=[]
17:36:01.381 -> CF:wifi_ssid=[-]
17:36:01.418 -> CF:wifi_pw=[InTheAir!]
17:36:01.457 -> CF:wifi_wepkey=[]
17:36:01.493 -> CF:webserver=[some.domain.com]
17:36:01.530 -> CF:webserver_port=[80]
17:36:01.568 -> CF:urlpath=[/measurements/url_create]
17:36:01.602 -> CF:apikey=[NOTTHEAPI]
17:36:01.637 -> CF:instrument_id=[1]
17:36:01.673 -> CF:info_server=[some.domain.com]
17:36:01.706 -> CF:info_server_port=[80]
17:36:01.744 -> CF:info_urlpath=[/info/log.php]
17:36:01.779 -> CF:info_apikey=[NOT_THE_REAL_API_KEY]
17:36:01.812 -> CF:ntpserver=[pool.ntp.org]
17:36:01.848 -> CF:rg1_enable=[1]
17:36:01.965 -> CF:op1=[5]
17:36:01.965 ->  DIST5M Set
17:36:01.965 -> CF:op2=[2]
17:36:02.039 -> CF:ds_baseline=[0]
17:36:02.073 -> CF:obs_period:obs_period=[1]
17:36:02.140 -> CF:daily_reboot=[22]
17:36:02.216 -> CF:elevation=[0]
17:36:02.216 -> OBS Interval:1m
17:36:02.252 -> INFO_Server:OK
17:36:02.252 -> 2026-02-13T00:36:07R
17:36:02.286 -> RTC:VALID
17:36:02.286 -> GPS:FOUND
17:36:02.320 -> GPS:TM NOT VALID
17:36:02.389 -> 
17:36:02.422 -> GPS:AQUIRED
17:36:02.422 -> GPS->RTC Set
17:36:02.456 -> RTC:VALID
17:36:02.456 -> GPN INFO:
17:36:02.490 ->  DATE:2026-02-13
17:36:02.524 ->  TIME:00:35:55
17:36:02.524 ->  LAT:41.072108
17:36:02.557 ->  LON:-102.034435
17:36:02.557 ->  ALT:1551.600000m
17:36:02.590 ->  ALT:5090.551189ft
17:36:02.624 ->  SAT:13
17:36:02.624 -> EEPROM NF
17:36:02.658 -> OBSAQS:INIT
17:36:02.658 -> OPTAQS NF
17:36:02.693 -> RG1:ENABLED
17:36:02.731 -> RG2:NOT ENABLED
17:36:02.731 -> AS5600:INIT
17:36:02.765 -> WD:OK
17:36:02.765 -> WS:Enabled
17:36:02.804 -> MUX:INIT
17:36:02.804 -> MUX NF
17:36:02.839 -> TSM:INIT
17:36:02.839 -> TSM NF
17:36:02.839 -> BMX:INIT
17:36:02.874 -> get_Bosch_ChipID()
17:36:02.911 ->   I2C:77 Reg:00
17:36:02.911 ->   ERR_ET:2
17:36:02.947 ->   I2C:77 Reg:D0
17:36:02.947 ->   ERR_ET:2
17:36:02.980 -> BMX_1 NF
17:36:02.980 -> get_Bosch_ChipID()
17:36:03.013 ->   I2C:76 Reg:00
17:36:03.013 ->   ERR_ET:2
17:36:03.047 ->   I2C:76 Reg:D0
17:36:03.080 ->   ERR_ET:2
17:36:03.080 -> BMX_2 NF
17:36:03.115 -> HTU21D:INIT
17:36:03.115 -> HTU NF
17:36:03.152 -> MCP9808:INIT
17:36:03.152 -> MCP1 OK
17:36:03.189 -> MCP2 NF
17:36:03.189 -> MCP3 NF
17:36:03.223 -> MCP4 NF
17:36:03.223 -> SHT:INIT
17:36:03.258 -> SHT1 OK
17:36:03.258 -> SHT2 NF
17:36:03.292 -> HIH8:INIT
17:36:03.292 -> HIH8 NF
17:36:03.330 -> SI1145:INIT
17:36:03.396 -> SI:OK
17:36:03.396 -> SI:VI[262.00]
17:36:03.431 -> SI:IR[255.00]
17:36:03.431 -> SI:UV[0.02]
17:36:03.465 -> PM25AQI:INIT
17:36:03.465 -> PM:NF
17:36:03.498 -> TLW:INIT
17:36:03.498 -> TLW NF
17:36:03.536 -> TSM:INIT
17:36:03.536 -> TSM NF
17:36:03.570 -> TMSM:INIT
17:36:03.570 -> TMSM NF
17:36:03.607 -> WBT:INIT
17:36:03.607 -> WBT:OK
17:36:03.641 -> HI:INIT
17:36:03.641 -> HI:OK
17:36:03.679 -> WBGT:INIT
17:36:03.679 -> WBGT:OK wo/Globe
17:36:04.258 -> BOARD:WiFi
17:36:04.258 -> WiFi:INIT
17:36:04.293 -> WiFi FW: 19.5.2
17:36:04.293 -> WiFiConnWait
17:36:04.331 -> WiFiCon SSID: [SSID]
17:36:04.331 -> WiFi Type: WPA/2
17:36:05.643 -> WiFi IP: 10.237.128.215
17:36:05.643 -> WiFi RSSI: -58
17:36:05.678 -> WiFi GetTime()
17:36:05.715 -> WiFi NTP Req
17:36:05.753 -> WiFi NTP Wait
17:36:05.787 -> WiFi UDP Read
17:36:05.821 -> WiFi NTP OK
17:36:05.821 -> WiFi RTC SET
17:36:05.855 -> 2026-02-13T00:36:10W
17:36:05.889 -> WiFi Valid
17:36:05.889 -> Start Main Loop
17:36:05.922 -> WDA:Init()
17:36:06.922 -> ............................................................
17:37:06.915 -> WS:0.00 WD:273
17:37:06.915 -> DS:0
17:37:06.950 -> {"MT":"INFO","at":"2026-02-13T00:37:11","devid":"73eee600304d98c9fc7fc5f4","board":"AFM0WiFi","ver":"FSLW-260211","bv":17.35,"hth":1,"elev":0,"ls":"3d.chordsrt.com","lsp":80,"lsurl":"/measurements/url_create","lsapi":"NOTTHEAPI","lsid":53,"ntp":"pool.ntp.org","ssid":"-","obsi":"1m","t2nt":"4294907s","drbt":"22m","n2s":"NF","devs":"sd,gps","wifi":{"fw":"19.5.2","ssid":"-","auth":"WPA/2","ip":"10.237.128.215","rssi":-58},"lat":41.072108,"lon":-102.034435,"alt":1551.600000,"sat":13,"sensors":"MCP1,SHT1,SI,AS5600,WS(A2),HI,WBT,WBGT WO/GLOBE,RG1(A3),5MDIST(A4,0),VBV(A5)"}
17:37:06.950 -> WiFi:SEND->HTTP
17:37:07.801 -> WiFi:HTTP CONNECTED
17:37:07.871 -> WiFi:HTTP SENT
17:37:07.907 -> WiFi:HTTP WAIT
17:37:07.941 -> WiFi:HTTP/1.1 200 OK
17:37:07.975 -> WiFi:Posted=1
17:37:07.975 -> {"MT":"INFO","at":"2026-02-13T00:37:11","devid":"73eee600304d98c9fc7fc5f4","board":"AFM0WiFi","ver":"FSLW-260211","bv":17.35,"hth":1,"elev":0,"ls":"3d.chordsrt.com","lsp":80,"lsurl":"/measurements/url_create","lsapi":"NOTTHEAPI","lsid":53,"ntp":"pool.ntp.org","ssid":"-","obsi":"1m","t2nt":"4294907s","drbt":"22m","n2s":"NF","devs":"sd,gps","wifi":{"fw":"19.5.2","ssid":"-","auth":"WPA/2","ip":"10.237.128.215","rssi":-58},"lat":41.072108,"lon":-102.034435,"alt":1551.600000,"sat":13,"sensors":"MCP1,SHT1,SI,AS5600,WS(A2),HI,WBT,WBGT WO/GLOBE,RG1(A3),5MDIST(A4,0),VBV(A5)"}
17:37:08.008 -> INFO->SD OK
17:37:08.042 -> Do OBS
17:37:08.042 -> OBS_DO()
17:37:08.076 -> OBS_TAKE()
17:37:08.255 -> OBS_TAKE(DONE)
17:37:08.255 -> OBS_LOG_ADD()
17:37:08.288 -> OBS->SD
17:37:08.288 -> {"at":"2026-02-13T00:37:12","bv":17.34,"hth":1,"rg1":0.0,"rgt1":0.0,"rgp1":0.0,"ds":5580.0,"dsr":5580.0,"vbv":5.0,"ws":0.0,"wd":273,"wg":0.0,"wgd":-999,"st1":22.3,"sh1":33.0,"sv1":261.0,"si1":256.0,"su1":0.0,"mt1":22.0,"hi":-999.9,"wbt":-4.2,"wbgt":-999.9}
17:37:08.325 -> /OBS/20260213.log
17:37:08.361 -> OBS Logged to SD
17:37:08.396 -> OBS_BUILD()
17:37:08.396 -> OBSBLD:OK
17:37:08.396 -> /measurements/url_create?key=NOTTHEAPI&instrument_id=1&devid=73eee600304d98c9fc7fc5f4at=2026-02-13T00%3A37%3A12&bv=17.34&hth=1&rg1=0.0&rgt1=0.0&rgp1=0.0&ds=5580.0&dsr=5580.0&vbv=5.0&ws=0.0&wd=273&wg=0.0&wgd=-999&st1=22.3&sh1=33.0&sv1=261.0&si1=256.0&su1=0.0&mt1=22.0&hi=-999.9&wbt=-4.2&wbgt=-999.9
17:37:08.430 -> OBS_SEND()
17:37:08.468 -> WiFi:SEND->HTTP
17:37:08.541 -> WiFi:HTTP CONNECTED
17:37:08.647 -> WiFi:HTTP SENT
17:37:09.686 -> WiFi:HTTP WAIT
17:37:09.686 -> WiFi:HTTP/1.1 200 OK
17:37:09.720 -> WiFi:Posted=1
17:37:09.755 -> FS->PUB OK
17:37:09.755 -> OBS:N2S Publish
```

LoRaWAN Example Start Up output
```
16:33:36.935 -> 
16:33:36.935 -> OLED:Enabled
16:33:36.971 -> SC:Enabled
16:33:36.971 -> SER:OK
16:33:38.997 -> Copyright [2026] [University Corporation for Atmospheric Research]
16:33:38.997 -> FSLW-260211
16:33:39.033 -> DevID:330eff6367815b7d93bfbcec
16:33:39.033 -> REBOOTPN SET
16:33:39.071 -> HEARTBEAT SET
16:33:39.325 -> SD:INIT
16:33:39.363 -> SD:Online
16:33:39.399 -> SD:OBS DIR Exists
16:33:39.434 -> CF:CF:lw_mode=[0]
16:33:39.468 -> CF:CF:lw_sf=[7]
16:33:39.506 -> CF:CF:lw_txpw=[14]
16:33:39.544 -> CF:lw_joineui=[0100000000000000]
16:33:39.581 -> CF:lw_deveui=[955207D07ED5B370]
16:33:39.616 -> CF:lw_appkey=[NOT_THE_REAL_API_KEY]
16:33:39.684 -> CF:lw_devaddr=[]
16:33:39.721 -> CF:lw_nwkskey=[]
16:33:39.791 -> CF:lw_appskey=[]
16:33:39.824 -> CF:CF:wifi_ssid=[SSID]
16:33:39.857 -> CF:CF:wifi_pw=[PASSWORD]
16:33:39.925 -> CF:wifi_wepkey=[]
16:33:39.925 -> CF:webserver=[some.domain.com]
16:33:39.958 -> CF:webserver_port=[80]
16:33:39.996 -> CF:urlpath=[/measurements/url_create]
16:33:40.033 -> CF:apikey=[NOTAPIKEY]
16:33:40.066 -> CF:instrument_id=[53]
16:33:40.100 -> CF:info_server=[iot.icdp.ucar.edu]
16:33:40.133 -> CF:info_server_port=[443]
16:33:40.169 -> CF:info_urlpath=[/pi/pilog.php]
16:33:40.207 -> CF:info_apikey=[NOT_THE_REAL_API_KEY]
16:33:40.240 -> CF:ntpserver=[pool.ntp.org]
16:33:40.275 -> CF:CF:rg1_enable=[1]
16:33:40.347 -> CF:op1=[5]
16:33:40.382 ->  DIST5M Set
16:33:40.415 -> CF:op2=[2]
16:33:40.489 -> CF:ds_baseline=[0]
16:33:40.523 -> CF:obs_period:obs_period=[1]
16:33:40.592 -> CF:daily_reboot=[22]
16:33:40.626 -> CF:elevation=[0]
16:33:40.659 -> OBS Interval:1m
16:33:40.659 -> INFO_Server:OK
16:33:40.694 -> 2026-02-11T23:33:37R
16:33:40.731 -> RTC:VALID
16:33:40.731 -> GPS:FOUND
16:33:40.764 -> GPS:TM NOT VALID
16:33:40.871 -> 
16:33:40.871 -> GPS:AQUIRED
16:33:40.908 -> GPS->RTC Set
16:33:40.908 -> RTC:VALID
16:33:40.946 -> GPN INFO:
16:33:40.946 ->  DATE:2026-02-11
16:33:40.982 ->  TIME:23:33:33
16:33:40.982 ->  LAT:41.072108
16:33:41.020 ->  LON:-102.034435
16:33:41.054 ->  ALT:1578.100000m
16:33:41.054 ->  ALT:5177.493446ft
16:33:41.089 ->  SAT:14
16:33:41.089 -> EEPROM NF
16:33:41.123 -> OBSAQS:INIT
16:33:41.123 -> OPTAQS NF
16:33:41.157 -> RG1:ENABLED
16:33:41.190 -> RG2:NOT ENABLED
16:33:41.190 -> AS5600:INIT
16:33:41.227 -> WD:OK
16:33:41.227 -> WS:Enabled
16:33:41.261 -> MUX:INIT
16:33:41.261 -> MUX NF
16:33:41.295 -> TSM:INIT
16:33:41.295 -> TSM NF
16:33:41.331 -> BMX:INIT
16:33:41.331 -> get_Bosch_ChipID()
16:33:41.367 ->   I2C:77 Reg:00
16:33:41.367 ->   ERR_ET:2
16:33:41.404 ->   I2C:77 Reg:D0
16:33:41.404 ->   ERR_ET:2
16:33:41.441 -> BMX_1 NF
16:33:41.441 -> get_Bosch_ChipID()
16:33:41.476 ->   I2C:76 Reg:00
16:33:41.476 ->   ERR_ET:2
16:33:41.512 ->   I2C:76 Reg:D0
16:33:41.549 ->   ERR_ET:2
16:33:41.549 -> BMX_2 NF
16:33:41.587 -> HTU21D:INIT
16:33:41.587 -> HTU NF
16:33:41.587 -> MCP9808:INIT
16:33:41.620 -> MCP1 OK
16:33:41.655 -> MCP2 NF
16:33:41.655 -> MCP3 NF
16:33:41.688 -> MCP4 NF
16:33:41.688 -> SHT:INIT
16:33:41.724 -> SHT1 OK
16:33:41.724 -> SHT2 NF
16:33:41.758 -> HIH8:INIT
16:33:41.758 -> HIH8 NF
16:33:41.793 -> SI1145:INIT
16:33:41.866 -> SI:OK
16:33:41.866 -> SI:VI[261.00]
16:33:41.902 -> SI:IR[260.00]
16:33:41.902 -> SI:UV[0.01]
16:33:41.936 -> PM25AQI:INIT
16:33:41.936 -> PM:NF
16:33:41.972 -> TLW:INIT
16:33:41.972 -> TLW NF
16:33:42.006 -> TSM:INIT
16:33:42.006 -> TSM NF
16:33:42.041 -> TMSM:INIT
16:33:42.041 -> TMSM NF
16:33:42.075 -> WBT:INIT
16:33:42.075 -> WBT:OK
16:33:42.109 -> HI:INIT
16:33:42.109 -> HI:OK
16:33:42.143 -> WBGT:INIT
16:33:42.143 -> WBGT:OK wo/Globe
16:33:42.628 -> BOARD:LoRaWAN
16:33:52.659 -> LW:INIT
16:33:52.659 -> LW:MODE OTAA
16:33:52.695 -> APP_EUI
16:33:52.695 -> 0100000000000000
16:33:52.734 -> DEV_EUI
16:33:52.734 -> 955207D07ED5B370
16:33:52.767 -> APP_KEY
16:33:52.767 -> A29FC7D4542B5BA368ED272A83C43971
16:33:52.801 -> LW:868MHz
16:33:52.837 -> LW EU868 Channels:
16:33:52.874 -> Ch 0: 868.10
16:33:52.874 -> Ch 1: 868.30
16:33:52.908 -> Ch 2: 868.50
16:33:52.908 -> TX Power (dBm): 0
16:33:52.942 -> Start Main Loop
16:33:52.942 -> WDA:Init()
16:33:53.219 -> LW:EV_JOINING
16:33:53.962 -> ..LW:EV_TXSTART
16:33:55.988 -> .....LW:EV_JOINED - WE ARE ONLINE
16:34:00.459 -> NETID:19
16:34:00.459 -> DEVADDR:260BC48F
16:34:00.496 -> NWKKEY
16:34:00.534 -> FC583955BD5996E6DDF839B7CF386FD9
16:34:00.534 -> APPSKEY
16:34:00.569 -> 28CE1604D55F1116705AB31F93FDB3FD
16:34:01.061 -> .....................................................
16:34:53.865 -> WS:0.00 WD:43
16:34:53.898 -> DS:0
16:34:53.933 -> INFO:LW READY
16:34:53.933 -> {"MT":"INFO","at":"2026-02-11T23:34:46","devid":"330eff6367815b7d93bfbcec","board":"AFM0LoRa","ver":"FSLW-260211","bv":17.13,"hth":1,"elev":0,"obsi":"1m","t2nt":"4294907s","drbt":"22m","n2s":"NF"}
16:34:53.966 -> LW:WaitOnTX(60s)
16:34:53.966 -> LW:TX NOT BUSY
16:34:54.004 -> LW_Send()
16:34:54.039 -> LW:MSG Queuing
16:34:54.076 -> LW:EV_TXSTART
16:34:54.076 -> LW:MSG Queued
16:34:54.113 -> INFO->PUB LW OK
16:34:54.147 -> LW:WaitOnTX(60s)
16:35:00.656 -> LW:EV_TXCMPLT,0 recv
16:35:01.161 -> LW:TX NOT BUSY
16:35:01.161 -> {"MT":"INFO","LW":"EU868,OTAA,7,14","devs":"sd,gps(1)","lat":41.072108,"lon":-102.034435,"alt":1578.100000,"sat":14}
16:35:01.195 -> LW:WaitOnTX(60s)
16:35:01.195 -> LW:TX NOT BUSY
16:35:01.228 -> LW_Send()
16:35:24.259 -> LW:MSG Queuing
16:35:24.259 -> LW:MSG Queued
16:35:24.294 -> INFO->PUB LW OK
16:35:24.328 -> LW:WaitOnTX(60s)
16:35:24.328 -> LW:TX NOT BUSY
16:35:24.328 -> {"MT":"INFO","sensors":"MCP1,SHT1,SI,AS5600,WS(A2),HI,WBT,WBGT WO/GLOBE,RG1(A3),5MDIST(A4,0),VBV(A5)"}
16:35:24.364 -> LW:WaitOnTX(60s)
16:35:24.398 -> LW:TX NOT BUSY
16:35:24.398 -> LW_Send()
16:35:27.351 -> LW:EV_TXSTART
16:35:33.954 -> LW:EV_TXCMPLT,0 recv
16:35:54.417 -> LW:MSG Queuing
16:35:54.452 -> LW:EV_TXSTART
16:35:54.485 -> LW:MSG Queued
16:35:54.521 -> INFO->PUB LW OK
16:35:54.521 -> LW:WaitOnTX(60s)
16:36:01.083 -> LW:EV_TXCMPLT,0 recv
16:36:01.547 -> LW:TX NOT BUSY
16:36:01.547 -> {"MT":"INFO","at":"2026-02-11T23:34:46","devid":"330eff6367815b7d93bfbcec","board":"AFM0LoRa","ver":"FSLW-260211","bv":17.13,"hth":1,"elev":0,"obsi":"1m","t2nt":"4294907s","drbt":"22m","n2s":"NF","LW":"EU868,OTAA,7,14","devs":"sd,gps(1)","lat":41.072108,"lon":-102.034435,"alt":1578.100000,"sat":14,"sensors":"MCP1,SHT1,SI,AS5600,WS(A2),HI,WBT,WBGT WO/GLOBE,RG1(A3),5MDIST(A4,0),VBV(A5)"}
16:36:01.619 -> INFO->SD OK
16:36:01.619 -> Do OBS
16:36:01.657 -> OBS_DO()
16:36:01.657 -> OBS_TAKE()
16:36:01.832 -> OBS_TAKE(DONE)
16:36:01.832 -> OBS_LOG_ADD()
16:36:01.866 -> OBS->SD
16:36:01.866 -> {"at":"2026-02-11T23:35:54","bv":17.12,"hth":1,"rg1":0.2,"rgt1":0.0,"rgp1":0.0,"ds":4565.0,"dsr":4565.0,"vbv":5.3,"ws":0.0,"wd":43,"wg":0.0,"wgd":-999,"st1":22.8,"sh1":37.1,"sv1":263.0,"si1":260.0,"su1":0.0,"mt1":23.2,"hi":-999.9,"wbt":-4.1,"wbgt":-999.9}
16:36:01.902 -> /OBS/20260211.log
16:36:01.938 -> OBS Logged to SD
16:36:01.973 -> OBS_BUILD()
16:36:01.973 -> OBSBLD:OK
16:36:01.973 -> at=2026-02-11T23%3A35%3A54&bv=17.12&hth=1&rg1=0.2&rgt1=0.0&rgp1=0.0&ds=4565.0&dsr=4565.0&vbv=5.3&ws=0.0&wd=43&wg=0.0&wgd=-999&st1=22.8&sh1=37.1&sv1=263.0&si1=260.0&su1=0.0&mt1=23.2&hi=-999.9&wbt=-4.1&wbgt=-999.9
16:36:02.007 -> OBS_SEND()
16:36:02.040 -> LW_Send()
16:36:25.035 -> LW:MSG Queuing
16:36:25.072 -> LW:EV_TXSTART
16:36:25.106 -> LW:MSG Queued
16:36:25.106 -> FS->PUB OK
16:36:25.143 -> OBS:N2S Publish
16:36:31.702 -> LW:EV_TXCMPLT,0 recv
```