/*
 * ======================================================================================================================
 *  notecard_dfu.h - Notecard Device Firmware Update Definations
 * ======================================================================================================================
 */

/*
    Open the Notehub project containing your Notecard.

    Go to Settings → Firmware → Upload Firmware.

    Upload the firmware binary intended for your Feather M0 and its update implementation.

    Go to Devices and select the target device.

    In its Host Firmware tab, click Update.

    Select the uploaded firmware and click Apply.
*/

/*
    Notecard dfu.get(offset, length)
             |
             v
    One Base64-encoded chunk in the JSON response
             |
             v
    Decode into reusable 512-byte buffer
             |
             v
    Write to UPDATE.TMP
             |
             v
    Release response and repeat

    After all chunks: sync and close UPDATE.TMP
             |
             v
    Reopen UPDATE.TMP from SD
             |
             v
    Read through the same 512-byte buffer and calculate CRC32
             |  
             v
    Compare file size and CRC32 with Notecard metadata
             |
             v
    Rename to UPDATE.BIN only if both match
}
*/

typedef struct  {
  uint32_t length;
  uint32_t crc32;
} NCDFUI;

// Extern variables
extern NCDFUI Notecard_DFU_Info;
extern unsigned long nc_dfu_check_time;

// Function prototypes
void notecard_dfu_check();
bool notecard_update_ready(NCDFUI &info);
bool notecard_update_download(SdFat &sd, NCDFUI &info);


