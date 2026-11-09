# Sending Device Actions Through Notehub
[←Top](../README.md)<BR>

## Overview

Send a device action by adding a JSON Note to the device's inbound queue, `doaction.qi`:

The required body format is `{"DO":"some action"}`. Replace `some action` with a supported action string below. Use uppercase `DO` and the exact action spelling and capitalization. Send one action per Note; do not combine commands into a comma-separated string or JSON array.

Notehub queues the Note, the Notecard downloads it during an inbound synchronization, and the host firmware reads the Note and process the `DO`  action.

## Prerequisites

- A device associated with the intended Notehub project.
- Access to add inbound Notes to that device.
- Host firmware that reads `doaction.qi`, extracts `body.DO` as a string, and calls `notecard_doa_perform()`.
- For API requests: a valid Notehub access token, ProjectUID, and DeviceUID. Notehub API requests use bearer authentication.

## Create `doaction.qi` if it does not exist

### Option 1: Create it in Notehub

1. Open the intended project and select the target device.
2. Open the device's Notefile view.
3. Use `Create Notefile` to create the inbound queue named exactly `doaction.qi`.
4. Select that file when adding action Notes.

### Option 2: Create it automatically with the first action

The Notehub API's Add QI Note endpoint creates the Notefile if it does not already exist. A separate existence check or empty-file creation request is unnecessary. The first request both creates the queue if needed and enqueues a real action.

For example, use the API request in the next section with `INFO`. This is not a no-op: when processed, it requests system information and synchronization.

Do not create a local SD-card file named `doaction.qi`; this is a Notehub/Notecard Notefile, not a host filesystem configuration file.

## Send an action from the Notehub UI

1. Open the project's Devices dashboard.
2. Select the intended device and click `+ Note`.
3. Select `doaction.qi` instead of the default `data.qi`.
4. Enter the action object in the Note body editor:

   ```json
   {"DO":"INFO"}
   ```

5. Click `Add Note` to queue it.

The UI body editor takes the body object directly; do not wrap it in another `body` object. Notes are delivered on the next inbound sync, not necessarily when you click `Add Note`.

## Send an action with the Notehub API

Set these shell variables to your actual values. Keep the token private and do not commit it to source control.

```bash
export NOTEHUB_PROJECT_UID='<your-project-uid>'
export NOTEHUB_DEVICE_UID='<your-device-uid>'
export NOTEHUB_TOKEN='<your-access-token>'
```

Send an `INFO` action:

```bash
curl --fail-with-body --show-error --request POST \
  "https://api.notefile.net/v1/projects/${NOTEHUB_PROJECT_UID}/devices/${NOTEHUB_DEVICE_UID}/notes/doaction.qi" \
  --header "Authorization: Bearer ${NOTEHUB_TOKEN}" \
  --header 'Content-Type: application/json' \
  --data '{"body":{"DO":"INFO"}}'
```

Unlike the UI body editor, the HTTP request requires the action object inside the top-level `body` property. The queue name is in the URL. This endpoint creates `doaction.qi` if necessary.

To send another action, change only the `DO` value, for example:

```json
{"body":{"DO":"OBSI:15"}}
```

A successful HTTP request confirms acceptance by Notehub, not successful execution by the host firmware. Delivery still depends on inbound synchronization.

## Supported actions

These tables describe the supplied firmware handler, not built-in Notehub commands. JSON examples are Note bodies for the UI; wrap them in `{"body": ...}` for the API.

### System and maintenance

| Action | Note body | Behavior |
| --- | --- | --- |
| `INFO` | `{"DO":"INFO"}` | Calls `INFO_Do_NoteCard()` to generate system information, then calls `notecard_hubsync()`. |
| `SEND` | `{"DO":"SEND"}` | Calls `notecard_hubsync()`. Intended by the code comment to send observations now; this branch does not explicitly take a new observation. |
| `REBOOT` | `{"DO":"REBOOT"}` | Saves unreported rain using `EEPROM_SaveUnreportedRain()`, waits 1 second, then calls `DeviceReset()`. |
| `CRT` | `{"DO":"CRT"}` | Clears rain totals through `EEPROM_ClearRainTotals()`, passing the current RTC Unix timestamp. |
| `CNV` | `{"DO":"CNV"}` | Calls `nv_deleteConfigIfCnvFileExists()`. The comment describes clearing nonvolatile configuration files and says the helper does not return on success. Exact conditions, files affected, and restart behavior require the helper implementation. |

### Wind and primary rain inputs

| Action | Note body | Behavior |
| --- | --- | --- |
| `NOWIND` | `{"DO":"NOWIND"}` | Sets `scv.wind = false` and saves configuration. |
| `DOWIND` | `{"DO":"DOWIND"}` | Sets `scv.wind = true` and saves configuration. |
| `NORAIN` | `{"DO":"NORAIN"}` | Sets `scv.rg1 = false` and saves configuration. |
| `DORAIN` | `{"DO":"DORAIN"}` | Sets `scv.rg1 = true` and saves configuration. |

`NORAIN` and `DORAIN` change `rg1`; they do not configure the secondary rain input on OP1 or clear rain totals.

### OP1 configuration

All OP1 actions call `saveConfig()` after updating the running configuration.

| Action | Note body | Behavior |
| --- | --- | --- |
| `OP1D5M` | `{"DO":"OP1D5M"}` | Selects the 5 m distance sensor mode: `scv.op1 = "d5m"`; sets `dg_resolution_adjust = 1.25`. |
| `OP1D10M` | `{"DO":"OP1D10M"}` | Selects the 10 m distance sensor mode: `scv.op1 = "d10m"`; sets adjustment to `2.5` and configures `OP1_PIN` as `INPUT`. |
| `OP1RAIN` | `{"DO":"OP1RAIN"}` | Selects secondary rain input: `scv.op1 = "rg2"`.|
| `OP1RAW` | `{"DO":"OP1RAW"}` | Selects raw mode: `scv.op1 = "raw"`. |
| `OP1CLR` | `{"DO":"OP1CLR"}` | Clears the OP1 selection: sets `scv.op1` to the literal string `"null"`. |

### OP2 configuration

All OP2 actions call `saveConfig()` after updating the running configuration.

| Action | Note body | Behavior |
| --- | --- | --- |
| `OP2RAW` | `{"DO":"OP2RAW"}` | Sets `scv.op2 = "raw"`. |
| `OP2VBV` | `{"DO":"OP2VBV"}` | Selects Voltaic battery-voltage mode: `scv.op2 = "voltaic"`. |
| `OP2CLR` | `{"DO":"OP2CLR"}` | Clears the OP2 selection by setting `scv.op2` to the literal string `"null"`. |

### Station mode

| Action | Note body | Behavior |
| --- | --- | --- |
| `OPTAQS` | `{"DO":"OPTAQS"}` | Enables air-quality-station mode by setting `scv.aqs = true`. |
| `OPTFS` | `{"DO":"OPTFS"}` | Selects full-station mode by setting `scv.aqs = false`. Also clears rain totals. |

### Parameterized actions

| Format | Example Note body | Behavior and validation |
| --- | --- | --- |
| `OBSI:<interval>` | `{"DO":"OBSI:15"}` | Sets `scv.obsi` and saves configuration. Accepts integers from `1` through `60`, inclusive. The handler does not establish the interval's units. |
| `SETELEV:<elevation>` | `{"DO":"SETELEV:1600"}` | Accepts an integer equal to `QC_ERR_ELEV`, or between `QC_MIN_ELEV` and `QC_MAX_ELEV`, inclusive. Sets `scv.elevation`, calls `mslp_initialize()`, and saves configuration. This reinitialization avoids a reboot according to the comment. The constants and elevation units are not provided. The example is valid only if `1600` is in the configured range. |
| `SETRTRO:<hour>` | `{"DO":"SETRTRO:6"}` | Sets `scv.rtro_hour` to `6` and `scv.rtro_minute` to `0`, then saves configuration. Hours are `0` through `23`. |
| `SETRTRO:<hour>:<minute>` | `{"DO":"SETRTRO:6:15"}` | Sets the RTRO hour and minute and saves configuration. Valid minutes are `0`, `15`, `30`, and `45`; valid hours are `0` through `23`. |

The snippet does not define what RTRO stands for, what scheduled operation uses it, or its timezone. Do not assume local time or UTC without checking the scheduler.

## Parameter formatting rules

### Observation interval and elevation

`OBSI:` and `SETELEV:` compare the integer converted back to a string against the original suffix. Use plain integer text with no extra spaces, leading zeros, plus sign, or decimal point.

Examples:

```json
{"DO":"OBSI:1"}
{"DO":"OBSI:60"}
```

These are invalid:

```json
{"DO":"OBSI:0"}
{"DO":"OBSI:61"}
{"DO":"OBSI:05"}
{"DO":"OBSI:5.0"}
{"DO":"OBSI: 5"}
```

Negative elevation values can be accepted if they meet the configured elevation limits or sentinel check; the snippet does not supply those limits.

### RTRO time

Use a clean 24-hour hour or quarter-hour time:

```json
{"DO":"SETRTRO:0"}
{"DO":"SETRTRO:6:15"}
{"DO":"SETRTRO:12:30"}
{"DO":"SETRTRO:23:45"}
```

`SETRTRO:24:00` and `SETRTRO:6:10` fail validation.
