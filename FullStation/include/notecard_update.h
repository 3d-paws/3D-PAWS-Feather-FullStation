/*
 * ======================================================================================================================
 *  notecard_update.h - Notecard SDU (SD Update) Boot Definations
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


    Main loop check
    if (notecard_update_ready(updateInfo)) {
      if (notecard_update_download(sd, updateInfo)) {
        // Verified UPDATE.BIN is now on the SD card.
        // Your existing updater can install it when appropriate.
    }
}
*/

#include <Notecard.h>

typedef struct  {
  uint32_t length;
  uint32_t crc32;
} NCUI;

// Extern variables
extern NCUI NotecardUpdateInfo;

// Function prototypes
bool notecard_update_ready(NCUI &info);
bool notecard_update_download(SdFat &sd, NCUI &info);


