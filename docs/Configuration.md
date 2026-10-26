# Station Configuration File
[←Top](../README.md)<BR>

This document describes configuration settings for station communications,
instruments, and system operation. Configurations are store on the SD card in a file named CONFIG.TXT.

Settings are grouped by hardware interface and function. Configure the sections
that apply to your station.

## Configuration File Format

The tables below document configuration key names, types, defaults, and accepted
values.

The configuration-file syntax is not defined by the supplied data structure.
Use the syntax supported by the firmware's configuration parser.

Defaults shown here are the initial values declared in the firmware. A dash
(`—`) means that no explicit default was provided in the supplied declaration.

Settings explicitly marked as firmware-only are documented separately and should
not be added to the configuration file.

## Feather M0 LoRaWAN Board

### General Settings

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `lw_mode` | String | — | Activation mode: `OTAA` or `ABP`. |
| `lw_sf` | Integer | `7` | Spreading factor. Supported values: `7`, `8`. |
| `lw_txpwr` | Integer | `14` | Transmit power. Documented range: `2`–`17`. The source comment labels the unit as `db`; confirm the intended unit in the radio implementation. |

### OTAA Settings

Use these settings when `lw_mode` is `OTAA`.

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `otaa_joineui` | String | — | JoinEUI, formerly called AppEUI. Enter in little-endian byte order. |
| `otaa_deveui` | String | — | Device EUI. Enter in little-endian byte order. |
| `otaa_appkey` | String | — | Application key. Enter in big-endian byte order. |

#### EUI Byte Order

The firmware comments require the JoinEUI and DevEUI to be supplied in
little-endian byte order.

Reverse the order of the byte pairs, not the individual hexadecimal digits.

Example:

```text
Original:       01 23 45 67 89 AB CD EF
Little-endian:  EF CD AB 89 67 45 23 01

Configuration value: EFCDAB8967452301
```

Do not apply this byte reversal to `otaa_appkey`; its documented byte order is
big-endian.

### ABP Settings

Use these settings when `lw_mode` is `ABP`.

All ABP values must contain hexadecimal characters and match the required lengths.

| Key | Type | Default | Required Length | Description |
| --- | --- | --- | --- | --- |
| `abp_devaddr` | String | — | 8 hexadecimal characters | Device address. |
| `abp_nwkskey` | String | — | 32 hexadecimal characters / 16 bytes | Network session key. |
| `abp_appskey` | String | — | 32 hexadecimal characters / 16 bytes | Application session key. |

## Feather M0 WiFi Board

### WiFi Connection

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `wifi_ssid` | String | — | WiFi network name. |
| `wifi_pw` | String | — | WiFi network password. |
| `wifi_wepkey` | String | — | WEP key, when applicable. Accepted lengths: 10 or 26 hexadecimal characters. |

### Log Server

These settings identify the observation-data destination, such as a CHORDS server.

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `ls_fqdn` | String | — | Log server fully qualified domain name. |
| `ls_port` | Integer | `80` | Log server port. |
| `ls_urlpath` | String | — | Log server URL path. |
| `ls_apikey` | String | — | Log server API key. |
| `ls_instr_id` | Integer | `0` | Instrument identifier used by the log server. |

### Info Server

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `is_fqdn` | String | — | Info server fully qualified domain name. |
| `is_port` | Integer | `80` | Info server port. |
| `is_urlpath` | String | — | Info server URL path. |
| `is_apikey` | String | — | Info server API key. |

### Time Server

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `ts_fqdn` | String | — | Time server fully qualified domain name. |

### HTTPS Preparation

For HTTPS connections on port `443`, upload the destination website's certificate
to the WiFi modem.

See the WiFi Board Preparation documentation for instructions.

## Feather M0 Board with Notecard

### Product and Connection Mode

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `nc_product` | String | — | Notehub Product UID associated with the Notecard. Example: `edu.ucar.wnicewonger:test_3dpaws`. |
| `nc_mode` | String | — | Notecard cloud connection mode. See the mode table below. |

The source comments list the following mode abbreviations:

| Mode | Name | Description |
| --- | --- | --- |
| `pm` | Periodic | Synchronize according to the configured schedule. |
| `cm` | Continuous | Maintain a continuous cloud connection. |
| `mm` | Minimum | Synchronize when explicitly requested by the host using `hub.sync`. |

Minimum mode does not maintain an always-on cellular connection and does not
connect on a periodic schedule.

The source also mentions the full mode names `continuous` and `periodic`.
Confirm whether the configuration parser expects abbreviations, full names, or
supports both.

### Synchronization Intervals

| Key | Type | Default | Valid Values | Description |
| --- | --- | --- | --- | --- |
| `nc_outbound` | Integer | `10` | `0`–`43200` minutes | Scheduled interval for sending queued outbound `.qo` Notes. |
| `nc_inbound` | Integer | `60` | `0`–`43200` minutes | Scheduled interval for checking inbound commands and configuration changes. |

#### Outbound Synchronization

- `0`: Disable scheduled outbound synchronization. The host is responsible for
  requesting synchronization using `hub.sync`.
- `1`–`43200`: Schedule outbound synchronization at the specified interval,
  from 1 minute to 30 days.

The source comments warn that using periodic mode with `nc_outbound=0` can result
in a disconnect/reconnect cycle for each observation when the host explicitly
requests a synchronization for each observation.

#### Inbound Synchronization

- `0`: Disable scheduled inbound polling.
- `1`–`43200`: Schedule inbound polling at the specified interval, from 1 minute
  to 30 days.

With scheduled inbound polling disabled, the Notecard does not periodically check
Notehub for:

- Inbound Notes.
- Environment-variable changes.
- Configuration changes.
- Pending device or host firmware-update work.

Inbound content can still arrive when the device establishes an outbound
synchronization.

### Transport Selection

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `nc_method` | String | `c` | Communication transport selection and order. |

| Value | Transport Order |
| --- | --- |
| `c` | Cellular |
| `w` | WiFi |
| `n` | Non-terrestrial network (NTN) only; not supported by this firmware configuration |
| `cn` | Cellular, then NTN |
| `wc` | WiFi, then cellular |
| `wn` | WiFi, then NTN |
| `wcn` | WiFi, then cellular, then NTN |

The source comments state that NTN-only operation is not supported because a
template must first be established with Notehub over cellular or WiFi.

### SIM Selection

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `nc_esim_apn` | String | — | External SIM access point name (APN). Example: `your.carrier.apn`. |
| `nc_esim_method` | String | — | SIM selection method. See the table below. |

| Value | Method | Behavior |
| --- | --- | --- |
| `p` | Primary | Use only the internal embedded Blues SIM. Documented as the default method. |
| `s` | Secondary | Use only the external SIM, provided an external SIM is physically detected. |
| `dps` | Dual, primary then secondary | Try the internal SIM first. Switch to the external SIM after cellular registration failures. |
| `dsp` | Dual, secondary then primary | Try the external SIM first. Switch to the internal Blues SIM after cellular registration failures. |

Although the source comments identify `p` as the default method,
`nc_esim_method` has no explicit initializer in the supplied declaration.

### Notecard WiFi Credentials

These settings are separate from the Feather M0 WiFi board's `wifi_ssid` and
`wifi_pw` settings.

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `nc_wifi_ssid` | String | — | WiFi network name used by the Notecard. |
| `nc_wifi_pw` | String | — | WiFi network password used by the Notecard. |

### Firmware-Only Notecard Settings

The following values are defined in firmware and are not configuration-file
settings.

| Firmware Variable | Type | Value | Description |
| --- | --- | --- | --- |
| `nc_qo_obs` | String | `obs.qo` | Outbound observation queue filename. |
| `nc_qo_info` | String | `info.qo` | Outbound information-message queue filename. |
| `nc_qo_port_obs` | Integer | `10` | Template port for observation messages. |
| `nc_qo_port_info` | Integer | `11` | Template port for information messages. |
| `nc_rr` | Integer | `1` | Host-to-Notecard I2C retry count. |

Observation and information messages use separate outbound queues because each
queue has its own template.

In Notehub, these queues are associated with routes. A route, also called a
webhook, forwards the data to the ingest service.

## Instruments

| Key | Type | Default | Valid Values | Description |
| --- | --- | --- | --- | --- |
| `aqs` | Integer | `0` | `0`, `1` | Station type: `0` = full station; `1` = air-quality station, labeled “Aquality Station” in the source. |
| `wind` | Integer | `1` | `0`, `1` | Wind instrumentation: `0` = disabled; `1` = enabled. |
| `rg1` | Integer | `1` | `0`, `1` | Rain gauge 1: `0` = disabled; `1` = enabled. |
| `op1` | String | — | `null`, `raw`, `d5m`, `d10m`, `rg2` | OP1 operating mode or attached instrument. |
| `op2` | String | — | `null`, `raw`, `voltaic` | OP2 operating mode or attached instrument. |
| `ds_baseline` | Integer | `0` | — | OP1 distance-sensor baseline, in millimeters from the sensor to the observation surface. |

### Optional Port Values

The supplied declarations list the accepted `op1` and `op2` values but do not
fully define their behavior.

| Port | Accepted Values |
| --- | --- |
| OP1 | `null`, `raw`, `d5m`, `d10m`, `rg2` |
| OP2 | `null`, `raw`, `voltaic` |

Use the exact tokens expected by the firmware. Whether `null` is a literal string,
an unquoted token, or another representation depends on the configuration parser.

## System Configuration

| Key | Type | Default | Valid Values / Units | Description |
| --- | --- | --- | --- | --- |
| `daily_reboot` | Integer | 22 | Hours | Reboot every configured number of hours. Despite the name, this is an interval rather than a time of day. |
| `obsi` | Integer | 1 | `1`–`60` minutes | Time between observations. |
| `rtro_hour` | Integer | `0` | `0`–`23` | Hour at which the rain total rolls over. |
| `rtro_minute` | Integer | `0` | `0`, `15`, `30`, `45` | Minute at which the rain total rolls over. |
| `elevation` | Integer | `-999` | Meters | Station


Example CONFIG.TXT file.
```
lw_mode=OTAA
lw_sf=7
lw_txpwr=14
otaa_joineui=0100000000000000
otaa_deveui=9552F0897674B370
otaa_appkey=A29FC7D4542BF675409D272A83C43971
abp_devaddr=260F0F01
abp_nwkskey=5F5B8DA76FF0456277B38210ED4B1AE3
abp_appskey=AF0B95941140C24ABAF089741BFF15C4
wifi_ssid=YourSSID
wifi_pw=SetPassword
wifi_wepkey=
ls_fqdn=3d.my.chords.site.com
ls_port=80
ls_urlpath=/measurements/url_create
ls_apikey=Set_Chords_api_key
ls_instr_id=53
is_fqdn=infosvr.somedomain.com
is_port=80
is_urlpath=/info/log.php
is_apikey=Set_your-x_api_key
ts_fqdn=pool.ntp.org
nc_product=edu.ucar.username:test_3dpaws
nc_mode=pm
nc_outbound=10
nc_inbound=60
nc_method=c
nc_rr=1
nc_esim_apn=
nc_esim_method=
nc_wifi_ssid=YourSSID
nc_wifi_pw=SetPassword
aqs=0
wind=0
rg1=1
op1=d5m
op2=voltaic
ds_baseline=0
daily_reboot=22
obsi=1
rtro_hour=0
rtro_minute=0
elevation=0
lat_deg=41.042385
lon_deg=-115.041656
```