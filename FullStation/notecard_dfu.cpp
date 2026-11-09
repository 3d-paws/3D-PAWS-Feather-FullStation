/*
 * ======================================================================================================================
 *  notecard_dfu.cpp - Notecard Device Firmware Update Functions
 * ======================================================================================================================
 */

#include "include/support.h"
#include "include/output.h"
#include "include/time.h"
#include "include/nvcf.h"
#include "include/feather.h"
#include "include/main.h"
#include "include/sdcard.h"
#include "include/notecard.h"
#include "include/notecard_dfu.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
NCDFUI Notecard_DFU_Info;
unsigned long nc_dfu_check_time=300000; // 5 minutes after boot ask note card if there is a DFU, value 5*60*1000 ms

/*
 * ======================================================================================================================
 * Firmware download constants
 * ======================================================================================================================
 */

static constexpr size_t NC_DFU_CHUNK_SIZE = 512;
static constexpr uint8_t NC_DFU_RETRIES = 3;

static const char NC_DFU_TMP[] = "UPDATE.TMP";
static const char NC_DFU_BIN[] = "UPDATE.BIN";

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */


/*
 * ======================================================================================================================
 * nc_dfu_uint32() - Read a required unsigned 32-bit integer from a JSON object.
 *
 * Rejects missing fields, non-numeric values, negative values, fractions, and values
 * outside the uint32_t range.
 * ======================================================================================================================
 */
static bool nc_dfu_uint32(J *object, const char *name, uint32_t &value) {
  if (object == NULL) {
    return false;
  }

  J *item = JGetObjectItem(object, name);

  if (item == NULL || !JIsNumber(item)) {
    return false;
  }

  double number = JGetNumber(object, name);

  // Also rejects NaN and infinity.
  if (!(number >= 0.0 && number <= 4294967295.0)) {
    return false;
  }

  uint32_t converted = static_cast<uint32_t>(number);

  if (static_cast<double>(converted) != number) {
    return false;
  }

  value = converted;
  return true;
}

/*
 * ======================================================================================================================
 * nc_dfu_base64_value() - Convert one Base64 character to its six-bit value.
 * ======================================================================================================================
 */
static int nc_dfu_base64_value(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a' + 26;
  if (c >= '0' && c <= '9') return c - '0' + 52;
  if (c == '+') return 62;
  if (c == '/') return 63;
  return -1;
}

/*
 * ======================================================================================================================
 * nc_dfu_decode() - Decode one Base64 response into the caller's chunk buffer.
 *
 * Requires standard padded Base64 and exactly expectedLength decoded bytes.
 * Rejects invalid characters, invalid padding, and oversized/undersized payloads.
 * ======================================================================================================================
 */
static bool nc_dfu_decode(
    const char *encoded,
    uint8_t *buffer,
    size_t expectedLength)
{
  if (encoded == NULL || expectedLength > NC_DFU_CHUNK_SIZE) {
    return false;
  }

  size_t encodedLength = strlen(encoded);

  if (encodedLength != 4 * ((expectedLength + 2) / 3)) {
    return false;
  }

  size_t written = 0;

  for (size_t i = 0; i < encodedLength; i += 4) {
    int a = nc_dfu_base64_value(encoded[i]);
    int b = nc_dfu_base64_value(encoded[i + 1]);

    bool pad2 = encoded[i + 2] == '=';
    bool pad3 = encoded[i + 3] == '=';

    int c = pad2 ? 0 : nc_dfu_base64_value(encoded[i + 2]);
    int d = pad3 ? 0 : nc_dfu_base64_value(encoded[i + 3]);

    if (a < 0 || b < 0 || c < 0 || d < 0) {
      return false;
    }

    if (pad2 && !pad3) {
      return false;
    }

    if ((pad2 || pad3) && i + 4 != encodedLength) {
      return false;
    }

    // Padding must leave unused bits set to zero.
    if ((pad2 && (b & 0x0F)) || (!pad2 && pad3 && (c & 0x03))) {
      return false;
    }

    size_t count = pad2 ? 1 : (pad3 ? 2 : 3);

    if (written + count > expectedLength) {
      return false;
    }

    uint32_t word =
        (static_cast<uint32_t>(a) << 18) |
        (static_cast<uint32_t>(b) << 12) |
        (static_cast<uint32_t>(c) << 6) |
        static_cast<uint32_t>(d);

    buffer[written++] = static_cast<uint8_t>(word >> 16);

    if (count >= 2) {
      buffer[written++] = static_cast<uint8_t>(word >> 8);
    }

    if (count == 3) {
      buffer[written++] = static_cast<uint8_t>(word);
    }
  }

  return written == expectedLength;
}

/*
 * ======================================================================================================================
 * nc_dfu_read_chunk() - Read and decode one chunk from the Notecard. Store in pointer to buffer
 * ======================================================================================================================
 */
static bool nc_dfu_read_chunk(uint32_t offset, size_t length, uint8_t *buffer) {

  // Try and read the chunk multiple times before calling it quits.
  for (uint8_t attempt = 1; attempt <= NC_DFU_RETRIES; ++attempt) {
    bool ok = false;

    J *req = notecard.newRequest("dfu.get");

    if (req != NULL) {
      JAddNumberToObject(req, "offset", offset);
      JAddNumberToObject(req, "length", length);

      J *rsp = notecard.requestAndResponse(req);

      if (rsp != NULL) {
        if (!JHasObjectItem(rsp, "err")) {
          // Chunk is base64 encoded, the follow decodes the chunk and stores it in buffer.
          // Note: Each dfu.get response is seperately base64 encoded. 
          ok = nc_dfu_decode(JGetString(rsp, "payload"), buffer, length);
        } else {
          sprintf (msgbuf, "DFU:get ERR %.100s", JGetString(rsp, "err"));
          Output(msgbuf);
        }

        notecard.deleteResponse(rsp);
      }
    }

    if (ok) {
      return true;
    }

    sprintf (msgbuf,"DFU:Read offset=%lu attempt=%u/%u ERR",
      static_cast<unsigned long>(offset),
      static_cast<unsigned>(attempt),
      static_cast<unsigned>(NC_DFU_RETRIES));
    Output(msgbuf);

    if (attempt < NC_DFU_RETRIES) {
      delay(250);
    }
  }

  return false;
}

/*
 * ======================================================================================================================
 * nc_dfu_crc32() - Update a standard reflected IEEE CRC32 accumulator.
 *
 * Initial value: 0xFFFFFFFF
 * Polynomial:    0xEDB88320
 * Final XOR:    0xFFFFFFFF, applied by the caller after the final block.
 * ======================================================================================================================
 */
static uint32_t nc_dfu_crc32(uint32_t crc, const uint8_t *data,size_t length) {
  while (length--) {
    crc ^= *data++;

    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc >> 1) ^ ((crc & 1U) ? 0xEDB88320UL : 0UL);
    }
  }

  return crc;
}

/*
 * ======================================================================================================================
 * notecard_update_download() - Download, verify, and stage the host firmware.
 * ======================================================================================================================
 * 
 * Requirements:
 *   - sd is your existing initialized SdFat object.
 *   - info came from notecard_update_ready().
 *   - Call from normal program context, not an interrupt.
 *   - Do not concurrently access the Notecard or these update files.
 *
 * Process:
 *   1. Recheck current firmware metadata.
 *   2. Download in 512-byte chunks to UPDATE.TMP.
 *   3. Sync and close UPDATE.TMP.
 *   4. Reopen UPDATE.TMP and calculate CRC32 from its SD contents.
 *   5. Compare size and CRC32 with the Notecard metadata.
 *   6. Remove any existing UPDATE.BIN.
 *   7. Rename verified UPDATE.TMP to UPDATE.BIN.
 *
 * Returns true only when UPDATE.BIN has been created successfully.
 *
 * Existing UPDATE.BIN is preserved until UPDATE.TMP passes verification.
 * Immediately before rename, any existing UPDATE.BIN is removed.
 * If removal fails, the function returns false without attempting rename.
 * If rename fails after removal, the old UPDATE.BIN is already gone;
 * the verified UPDATE.TMP is retained.
 *
 * On earlier failures, UPDATE.TMP may remain incomplete or invalid.
 * The next download attempt replaces any existing UPDATE.TMP.
 *
 * Does not change Notecard configuration or mark the firmware installed.
 * ======================================================================================================================
 */
bool notecard_update_download( SdFat &sd, NCDFUI &info)
{
  // Recheck notecard update file to make sure it has not changed since initial check
  NCDFUI current = {};

  if (!notecard_update_ready(current)) {
    return false;
  }

  if (current.length != info.length || current.crc32 != info.crc32) {
    Output ("DFU:Image changed; check readiness again");
    return false;
  }

  // Remove old UPDATE.TMP file
  if (sd.exists(NC_DFU_TMP) && !sd.remove(NC_DFU_TMP)) {
    Output ("DFU:Cannot remove old UPDATE.TMP");
    return false;
  }

  // Prepare for receiving the update file
  File32 file;

  if (!file.open(NC_DFU_TMP, O_WRONLY | O_CREAT | O_TRUNC)) {
     Output ("DFU:Cannot create UPDATE.TMP");
    return false;
  }

  // One buffer is reused for downloading and SD verification.
  // The entire firmware file is never held in RAM.
  uint8_t buffer[NC_DFU_CHUNK_SIZE];

  uint32_t offset = 0;
  uint32_t lastProgress = millis();

  sprintf (msgbuf, "DFU:Download starting: %lu bytes", static_cast<unsigned long>(info.length));
  Output(msgbuf);

  // ========================================================================
  // Work through reading notecard file in chunks writing to SD card tmp file
  // ========================================================================
  while (offset < info.length) {
    uint32_t remaining = info.length - offset;
    size_t count = (remaining > sizeof(buffer)) ? sizeof(buffer) : static_cast<size_t>(remaining);

    // Get chunk from File Offset
    if (!nc_dfu_read_chunk(offset, count, buffer)) {
      file.close();

      sprintf (msgbuf, "DFU:Download failed at %lu", static_cast<unsigned long>(offset));
      Output(msgbuf);
      return false;
    }

    // Write Chunk to SD card tmp file
    if (file.write(buffer, count) != count) {
      file.close();

      sprintf (msgbuf, "DFU:SD write ERR at %lu", static_cast<unsigned long>(offset));
      Output(msgbuf);
      return false;
    }

    offset += count;

    // Report progress
    if (offset == info.length || static_cast<uint32_t>(millis() - lastProgress) >= 2000) {
      const unsigned long percent = (info.length != 0) ? static_cast<unsigned long>((static_cast<uint64_t>(offset) * 100) / info.length) : 0;

      sprintf (msgbuf, "DFU:Downloaded %lu/%lu bytes (%lu%%)", 
        static_cast<unsigned long>(offset), 
        static_cast<unsigned long>(info.length), 
        percent);
      Output(msgbuf);

      lastProgress = millis();
    }
  }

  // Flush file data and metadata before closing.
  bool synced = file.sync();
  file.close();

  if (!synced) {
     Output ("DFU:SD sync ERR");
    return false;
  }

  // ============================================================
  // Start SD tmp file verification 
  // ============================================================
  Output("DFU:Verifying UPDATE.TMP from SD");

  if (!file.open(NC_DFU_TMP, O_RDONLY)) {
     Output ("DFU:Cannot reopen UPDATE.TMP");
    return false;
  }

  // Check file size
  uint32_t actualSize = file.fileSize();
  if (actualSize != info.length) {
    file.close();
    sprintf (msgbuf, "DFU:Size ERR expected=%lu actual=%lu", static_cast<unsigned long>(info.length), static_cast<unsigned long>(actualSize));
    Output(msgbuf);
    return false;
  }

  // Check CRC

  // Get SD file CRC
  uint32_t remaining = info.length;
  uint32_t crc = 0xFFFFFFFFUL;

  while (remaining > 0) {
    size_t count = (remaining > sizeof(buffer)) ? sizeof(buffer) : static_cast<size_t>(remaining);

    int bytesRead = file.read(buffer, count);

    if (bytesRead != static_cast<int>(count)) {
      file.close();
       Output ("DFU:SD verification read ERR");
      return false;
    }

    crc = nc_dfu_crc32(crc, buffer, count);
    remaining -= count;
  }

  file.close();
  crc ^= 0xFFFFFFFFUL;

  // Show Result Summary
  sprintf (msgbuf, "DFU:CRC32 expected=%08lX actual=%08lX", static_cast<unsigned long>(info.crc32), static_cast<unsigned long>(crc));
  Output(msgbuf);

  // Check file CRC against Notecard CRC information
  if (crc != info.crc32) {
     Output ("DFU:CRC32 ERR; UPDATE.TMP not promoted");
    return false;
  }

  Output ("DFU:UPDATE.TMP size and CRC32 verified");

  // ============================================================
  // Make UPDATE.TMP into UPDATE.BIN 
  // ============================================================ 

  // Only now remove the previous UPDATE.BIN. - It remains untouched if downloading or verification fails.
  if (sd.exists(NC_DFU_BIN)) {
    Output ("DFU:Existing UPDATE.BIN found; removing");

    if (!sd.remove(NC_DFU_BIN)) {
      Output ("DFU:Remove UPDATE.BIN ERR");
      Output ("DFU:Verified UPDATE.TMP retained");
      return false;
    }

    Output ("DFU:Existing UPDATE.BIN removed");
  }

  // Do the actual move of the tmp file
  Output ("DFU:Renaming UPDATE.TMP -> UPDATE.BIN");

  if (!sd.rename(NC_DFU_TMP, NC_DFU_BIN)) {
    Output ("DFU:Rename UPDATE.TMP -> UPDATE.BIN ERR");
    Output ("DFU:Verified UPDATE.TMP retained");
    return false;
  }

  Output ("DFU:Rename UPDATE.TMP -> UPDATE.BIN OK");
  Output ("DFU:Verified; UPDATE.BIN ready");

  // A reboot is required to install the new firmware via SDUBoot 
  return true;
}

/*
 *=======================================================================================================================
 * notecard_update_clear()
 *
 * End the current host DFU after the firmware has been successfully
 * copied to SD, verified, and renamed to UPDATE.BIN.
 *
 * IMPORTANT:
 * This marks the DFU completed in Notehub. It does not install the
 * firmware on the host MCU.
 *
 * Returns true if the Notecard accepted the request.
 *=======================================================================================================================
 */
bool notecard_update_clear() {
  J *req = notecard.newRequest("dfu.status");
  if (req == NULL) {
    Output("NC:UPDATE New(dfu.status stop) ERR");
    return false;
  }

  JAddStringToObject(req, "name", "user");
  JAddBoolToObject(req, "stop", true);
  JAddStringToObject(req, "status", "Firmware verified and saved to SD; installation pending");

  // sendRequestWithRetry() frees req, including on failure.
  if (!notecard.sendRequestWithRetry(req, scv.nc_rr)) {
    Output("NC:UPDATE Clear DFU ERR");
    return false;
  }

  Output("NC:UPDATE DFU cleared; image staged on SD");
  return true;
}


/*
 * ======================================================================================================================
 * notecard_update_ready() - Check whether host firmware is downloaded and readable.
 * ======================================================================================================================
 * 
 * Returns true:
 *   dfu.status reports mode "ready", valid length/CRC32 metadata is available,
 *   and a zero-length dfu.get request succeeds.
 *
 * Returns false:
 *   No image is ready, metadata is invalid, the image is not currently readable,
 *   or a request fails.
 *
 * Does not configure the Notecard, initiate a sync, or change its operating mode.
 * info is cleared on failure.
 * ======================================================================================================================
 */
bool notecard_update_ready(NCDFUI &info)
{
  info.length = 0;
  info.crc32 = 0;

  // ==============================================================
  // Get the dfu.status for user dfu file aka feather firmware file
  // ==============================================================
  J *req = notecard.newRequest("dfu.status");

  if (req == NULL) {
    Output("DFU:New(status) ERR");
    return false;
  }

  JAddStringToObject(req, "name", "user");

  // requestAndResponse() takes ownership of req.
  J *rsp = notecard.requestAndResponse(req);

  if (rsp == NULL) {
    notecard_exists = false;
    Output("DFU:status NO RESPONSE");
    return false;
  }

  if (JHasObjectItem(rsp, "err")) {
    sprintf (msgbuf, "DFU:status ERR %.100s", JGetString(rsp, "err"));
    Output(msgbuf);

    notecard.deleteResponse(rsp);
    return false;
  }

  /*
  {
  "mode": "ready",
  "status": "successfully downloaded",
  "on": true,
  "body": {
    "crc32": 2525287425,
    "length": 42892,
    "name": "stm32-new-firmware$20200903200351.bin",
    "source": "stm32-new-firmware.bin",
    "type": "firmware"
    }
  }
  */

  const char *mode = JGetString(rsp, "mode");

  if (mode == NULL || strcmp(mode, "ready") != 0) {
    sprintf (msgbuf, "DFU:Not ready: %.24s", mode ? mode : "unknown");
    Output(msgbuf);

    notecard.deleteResponse(rsp);
    return false;
  }

  NCDFUI candidate = {};
  J *body = JGetObjectItem(rsp, "body");

  bool valid =
      nc_dfu_uint32(body, "length", candidate.length) &&
      nc_dfu_uint32(body, "crc32", candidate.crc32) &&
      candidate.length > 0;

  notecard.deleteResponse(rsp);

  if (!valid) {
    Output("DFU:Invalid length/CRC32 metadata");
    return false;
  }

  // ======================================================================
  // Check whether firmware can actually be read without changing hub mode.
  // ======================================================================
  /*
    This asks: “Can you serve the downloaded host firmware right now? Do not send any firmware bytes.” The documented successful response is
    {
       "payload": ""
    }
  */

  req = notecard.newRequest("dfu.get");

  if (req == NULL) {
    Output("DFU:New(get) ERR");
    return false;
  }
  JAddNumberToObject(req, "length", 0);

  rsp = notecard.requestAndResponse(req);

  if (rsp == NULL) {
    notecard_exists = false;
    Output("DFU:get probe NO RESPONSE");
    return false;
  }

  if (JHasObjectItem(rsp, "err")) {
    sprintf (msgbuf, "DFU:Not readable: %.100s", JGetString(rsp, "err"));
    Output(msgbuf);

    notecard.deleteResponse(rsp);
    return false;
  }

  notecard.deleteResponse(rsp);

  info = candidate;

  sprintf (msgbuf, "DFU:READY bytes=%lu CRC32=%08lX", static_cast<unsigned long>(info.length), static_cast<unsigned long>(info.crc32));
  Output(msgbuf);

  return true;
}

/*
 * ======================================================================================================================
 * notecard_dfu_check() - Check and if ready perform all steps needed to do the Update. Including the reboot.
 * ======================================================================================================================
 */
void notecard_dfu_check() {
  if (notecard_exists && scv.nc_dfu && ((int32_t)(millis() - nc_dfu_check_time) >= 0)) {
    Output("DFU:Check");
    if (notecard_update_ready(Notecard_DFU_Info)) {
      if (notecard_update_download(SD, Notecard_DFU_Info)) {
        notecard_update_clear(); // Tell NoteCard the update was good.
                                 // The reboot from installing the firmware and startup hubsync will do the push and tell NoteHub we have updated. 
        Output("DFU:Rebooting");  
        delay(1000);
        DeviceReset();
      }
    }
    nc_dfu_check_time = millis() + (scv.nc_dfu * 60 * 1000);
  }
}