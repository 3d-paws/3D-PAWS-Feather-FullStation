/*
 * ======================================================================================================================
 *  notecard.h - Notecard Definations
 * ======================================================================================================================
 */

/*
Notehub Webside https://notehub.io

To enable trace on the notecard usb port send {"req":"card.trace","mode":"on"}

A compact template is a predefined binary record layout for Notes stored in one Notefile. You tell the Notecard exactly which fields, 
data types, and optional metadata each record will contain, and it then stores and transmits future Notes in a small fixed-format 
binary representation rather than repeatedly sending JSON field names and metadata.

You apply a compact template to a specific outbound queued Notefile, such as obs.qo, and every Note subsequently added to that
file for satellite/NTN transport must conform to that template’s field definitions and types.

It is required for Notes sent through a Starnote satellite connection—whether Skylo or Iridium—and for Notecard LoRa.
If a Notefile might ever be transmitted through the ntn fallback, every Note in that file must use a compact template, 
even when it usually goes out over cellular first.

For satellite/NTN operation, the Notecard encodes each queued Note as one satellite message and must be able to fit 
that Note within the satellite product’s per-message limit. It does not transparently split one oversized Note across 
multiple satellite packets for you.

With a compact template, the Notecard knows the fixed binary layout of the Note, can calculate its encoded size, and 
sends that single compact record through the selected satellite transport.
 
| Satellite product                        | One Note/message size                         | What happens                                                                           |
| ---------------------------------------- | --------------------------------------------- | -------------------------------------------------------------------------------------- |
| Starnote for Skylo or Notecard for Skylo | 50-byte minimum billed; 256-byte maximum      | A Note above 256 bytes cannot be sent over Skylo satellite                             |
| Starnote for Iridium                     | 10-byte minimum billed; 10,000-byte maximum   | A Note must fit within the Iridium maximum, though smaller messages are usually better |
| Cellular or Wi‑Fi                        | Much less restrictive for normal Notefile use | Large Notes can be uploaded normally, subject to other Notecard/Notehub limits         |

The template is sent to and stored by Notehub, so Notehub can decode the compact satellite payload back into a normal JSON Note 
before it routes the event to your cloud application. Your receiving application normally does not need an out-of-band copy of the compact binary layout

If a compact Note is larger than the maximum supported by the transport actually selected, it cannot be sent by that transport. 
The template does not cause automatic fragmentation into several satellite messages, and you should not assume the Notecard will 
automatically switch a too-large Note to cellular after it has selected NTN.

For an NTN fallback design, treat the maximum size as a hard protocol constraint and validate that every possible instance of the 
record fits before deployment.

| Template value | Compact type            | C/C++ equivalent      | Typical range / use                                     |
| -------------- | ----------------------- | --------------------- | ------------------------------------------------------- |
| 11             | 1-byte signed integer   | int8_t                | −128 to 127                                             |
| 12             | 2-byte signed integer   | int16_t               | −32,768 to 32,767                                       |
| 14             | 4-byte signed integer   | int32_t               | −2,147,483,648 to 2,147,483,647                         |
| 18             | 8-byte signed integer   | int64_t               | Very large signed counters/timestamps                   |
| 21             | 1-byte unsigned integer | uint8_t               | 0 to 255                                                |
| 22             | 2-byte unsigned integer | uint16_t              | 0 to 65,535                                             |
| 24             | 4-byte unsigned integer | uint32_t              | 0 to 4,294,967,295                                      |
| 28             | 8-byte unsigned integer | uint64_t              | Very large non-negative counters                        |
| 14.1           | 4-byte float            | float / float32_t     | Sensor values with fractions                            |
| 18.1           | 8-byte float            | double / float64_t    | Higher precision, 8-byte cost                           |
| true           | Boolean                 | bool                  | true or false                                           |
| "text"         | String                  | const char * / string | Variable length; avoid for satellite-critical telemetry |

{
  "req": "note.template",
  "file": "status.qo",
  "format": "compact",
  "port": 10,
  "body": {
    "battery_v": 14.1,
    "temperature_c": 14.1,
    "fault_code": 14,
    "message": "text"
  }
}

port is the compact-template identifier used on the constrained radio link. Think of it as a small numeric schema/channel 
ID that tells Notehub which compact template to use when decoding a satellite (or LoRa) message. For satellite templates, 
assign each compact Notefile its own unique port number—normally in the range 1 through 100.

When Notehub receives a message tagged with port 10, it uses the template previously synced for port 10 and reconstructs 
the normal JSON fields before routing the Note to your application.

The algorithm used to determine the template format.  Has to be used when sending the observations.

Do not plan on syncing a new or changed compact template through satellite. You must create the note.template on the 
Notecard and then complete a successful cellular or Wi‑Fi sync with Notehub before sending Notes using that template 
through Skylo or Iridium satellite transport.

1. Host creates template on Notecard
          │
          ▼
2. Force/use cellular or Wi‑Fi transport
          │
          ▼
3. hub.sync successfully reaches Notehub
          │
          ▼
4. Notehub now knows port 10 and its schema
          │
          ▼
5. Permit cell-ntn / wifi-cell-ntn / ntn transport
          │
          ▼
6. Satellite compact Notes can be decoded and routed

INFO messages get interesting.  Might as well code for the 256 so we can use both Skylo or Iridium

For portable LoRaWAN compatibility, use fixed-width compact fields and target no more than about 40–45 bytes, 
leaving margin for platform/transport overhead.
*/
#include <Notecard.h>


#define NOTECARD_I2C_ADDRESS       0x17

// Extern variables
extern bool notecard_exists;
extern bool notecard_ntn_enabled;       // Non-Terrestrial Network 

// Function prototypes
bool notecard_hubsync_status();
bool notecard_wireless_status();
bool notecard_wifi_status();
void notecard_initialize();
bool notecard_rtc_sync();
void notecard_qtt_obs(char *obs);
void notecard_qtt_info(char *info);

