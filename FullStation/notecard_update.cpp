/*
 * ======================================================================================================================
 *  notecard_update.cpp - Notecard SDU (SD Update) Boot Functions
 * ======================================================================================================================
 */

#include "include/support.h"
#include "include/ssbits.h"
#include "include/output.h"
#include "include/time.h"
#include "include/nvcf.h"
#include "include/main.h"
#include "include/sdcard.h"
#include "include/notecard.h"
#include "include/notecard_update.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
NCUI NotecardUpdateInfo;

/*
 * ======================================================================================================================
 * Firmware download constants
 * ======================================================================================================================
 */

static constexpr size_t NC_UPDATE_CHUNK_SIZE = 512;
static constexpr uint8_t NC_UPDATE_RETRIES = 3;

static const char NC_UPDATE_TMP[] = "UPDATE.TMP";
static const char NC_UPDATE_BIN[] = "UPDATE.BIN";

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * nc_update_output() - Format a message and pass it to the existing Output(char *).
 *
 * Output() must consume/copy the supplied text before returning.
 * ======================================================================================================================
 */
static void nc_update_output(const char *format, ...)
{
  char message[160];

  va_list args;
  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);

  Output(message);
}


/*
 * ======================================================================================================================
 * nc_update_uint32() - Read a required unsigned 32-bit integer from a JSON object.
 *
 * Rejects missing fields, non-numeric values, negative values, fractions, and values
 * outside the uint32_t range.
 * ======================================================================================================================
 */
static bool nc_update_uint32(J *object, const char *name, uint32_t &value)
{
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
 * nc_update_base64_value() - Convert one Base64 character to its six-bit value.
 * ======================================================================================================================
 */
static int nc_update_base64_value(char c)
{
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a' + 26;
  if (c >= '0' && c <= '9') return c - '0' + 52;
  if (c == '+') return 62;
  if (c == '/') return 63;

  return -1;
}


/*
 * ======================================================================================================================
 * nc_update_decode() - Decode one Base64 response into the caller's chunk buffer.
 *
 * Requires standard padded Base64 and exactly expectedLength decoded bytes.
 * Rejects invalid characters, invalid padding, and oversized/undersized payloads.
 * ======================================================================================================================
 */
static bool nc_update_decode(
    const char *encoded,
    uint8_t *buffer,
    size_t expectedLength)
{
  if (encoded == NULL || expectedLength > NC_UPDATE_CHUNK_SIZE) {
    return false;
  }

  size_t encodedLength = strlen(encoded);

  if (encodedLength != 4 * ((expectedLength + 2) / 3)) {
    return false;
  }

  size_t written = 0;

  for (size_t i = 0; i < encodedLength; i += 4) {
    int a = nc_update_base64_value(encoded[i]);
    int b = nc_update_base64_value(encoded[i + 1]);

    bool pad2 = encoded[i + 2] == '=';
    bool pad3 = encoded[i + 3] == '=';

    int c = pad2 ? 0 : nc_update_base64_value(encoded[i + 2]);
    int d = pad3 ? 0 : nc_update_base64_value(encoded[i + 3]);

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
    if ((pad2 && (b & 0x0F)) ||
        (!pad2 && pad3 && (c & 0x03))) {
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
 * nc_update_read_chunk() - Read and decode one chunk from the Notecard.
 *
 * Each retry requests the same offset.
 * Every response is released before returning or retrying.
 * ======================================================================================================================
 */
static bool nc_update_read_chunk(
    uint32_t offset,
    size_t length,
    uint8_t *buffer)
{
  for (uint8_t attempt = 1; attempt <= NC_UPDATE_RETRIES; ++attempt) {
    bool ok = false;

    J *req = notecard.newRequest("dfu.get");

    if (req != NULL) {
      JAddNumberToObject(req, "offset", offset);
      JAddNumberToObject(req, "length", length);

      J *rsp = notecard.requestAndResponse(req);

      if (rsp != NULL) {
        if (!JHasObjectItem(rsp, "err")) {
          ok = nc_update_decode(
              JGetString(rsp, "payload"), buffer, length);
        } else {
          nc_update_output("NC:UPDATE dfu.get ERR %.100s",
                           JGetString(rsp, "err"));
        }

        notecard.deleteResponse(rsp);
      }
    }

    if (ok) {
      return true;
    }

    nc_update_output("NC:UPDATE Read offset=%lu attempt=%u/%u ERR",
                     static_cast<unsigned long>(offset),
                     static_cast<unsigned>(attempt),
                     static_cast<unsigned>(NC_UPDATE_RETRIES));

    if (attempt < NC_UPDATE_RETRIES) {
      delay(250);
    }
  }

  return false;
}


/*
 * ======================================================================================================================
 * nc_update_crc32() - Update a standard reflected IEEE CRC32 accumulator.
 *
 * Initial value: 0xFFFFFFFF
 * Polynomial:    0xEDB88320
 * Final XOR:    0xFFFFFFFF, applied by the caller after the final block.
 * ======================================================================================================================
 */
static uint32_t nc_update_crc32(
    uint32_t crc,
    const uint8_t *data,
    size_t length)
{
  while (length--) {
    crc ^= *data++;

    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc >> 1) ^
            ((crc & 1U) ? 0xEDB88320UL : 0UL);
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
bool notecard_update_download( SdFat &sd, NCUI &info)
{
  // Recheck immediately before downloading so stale caller metadata
  // cannot silently be used for a different staged image.
  NCUI current = {};

  if (!notecard_update_ready(current)) {
    return false;
  }

  if (current.length != info.length ||
      current.crc32 != info.crc32) {
    nc_update_output("NC:UPDATE Image changed; check readiness again");
    return false;
  }

  if (sd.exists(NC_UPDATE_TMP) && !sd.remove(NC_UPDATE_TMP)) {
    nc_update_output("NC:UPDATE Cannot remove old UPDATE.TMP");
    return false;
  }

  File32 file;

  if (!file.open(NC_UPDATE_TMP, O_WRONLY | O_CREAT | O_TRUNC)) {
    nc_update_output("NC:UPDATE Cannot create UPDATE.TMP");
    return false;
  }

  // One buffer is reused for downloading and SD verification.
  // The entire firmware file is never held in RAM.
  uint8_t buffer[NC_UPDATE_CHUNK_SIZE];

  uint32_t offset = 0;
  uint32_t lastProgress = millis();

  nc_update_output("NC:UPDATE Download starting: %lu bytes",
                   static_cast<unsigned long>(info.length));

  while (offset < info.length) {
    uint32_t remaining = info.length - offset;

    size_t count = remaining > sizeof(buffer)
                       ? sizeof(buffer)
                       : static_cast<size_t>(remaining);

    if (!nc_update_read_chunk(offset, count, buffer)) {
      file.close();

      nc_update_output("NC:UPDATE Download failed at %lu",
                       static_cast<unsigned long>(offset));
      return false;
    }

    if (file.write(buffer, count) != count) {
      file.close();

      nc_update_output("NC:UPDATE SD write ERR at %lu",
                       static_cast<unsigned long>(offset));
      return false;
    }

    offset += count;

    if (offset == info.length ||
        static_cast<uint32_t>(millis() - lastProgress) >= 2000) {
      nc_update_output("NC:UPDATE Downloaded %lu/%lu bytes",
                       static_cast<unsigned long>(offset),
                       static_cast<unsigned long>(info.length));

      lastProgress = millis();
    }
  }

  // Flush file data and metadata before closing.
  bool synced = file.sync();
  file.close();

  if (!synced) {
    nc_update_output("NC:UPDATE SD sync ERR");
    return false;
  }

  // Start a separate verification pass from the actual SD file.
  if (!file.open(NC_UPDATE_TMP, O_RDONLY)) {
    nc_update_output("NC:UPDATE Cannot reopen UPDATE.TMP");
    return false;
  }

  uint32_t actualSize = file.fileSize();

  if (actualSize != info.length) {
    file.close();

    nc_update_output("NC:UPDATE Size ERR expected=%lu actual=%lu",
                     static_cast<unsigned long>(info.length),
                     static_cast<unsigned long>(actualSize));
    return false;
  }

  nc_update_output("NC:UPDATE Verifying UPDATE.TMP from SD");

  uint32_t remaining = info.length;
  uint32_t crc = 0xFFFFFFFFUL;

  while (remaining > 0) {
    size_t count = remaining > sizeof(buffer)
                       ? sizeof(buffer)
                       : static_cast<size_t>(remaining);

    int bytesRead = file.read(buffer, count);

    if (bytesRead != static_cast<int>(count)) {
      file.close();
      nc_update_output("NC:UPDATE SD verification read ERR");
      return false;
    }

    crc = nc_update_crc32(crc, buffer, count);
    remaining -= count;
  }

  file.close();
  crc ^= 0xFFFFFFFFUL;

  nc_update_output("NC:UPDATE CRC32 expected=%08lX actual=%08lX",
                   static_cast<unsigned long>(info.crc32),
                   static_cast<unsigned long>(crc));

  if (crc != info.crc32) {
    nc_update_output("NC:UPDATE CRC32 ERR; UPDATE.TMP not promoted");
    return false;
  }

  nc_update_output("NC:UPDATE UPDATE.TMP size and CRC32 verified");

  // Only now remove the previous UPDATE.BIN.
  // It remains untouched if downloading or verification fails.
  if (sd.exists(NC_UPDATE_BIN)) {
    nc_update_output("NC:UPDATE Existing UPDATE.BIN found; removing");

    if (!sd.remove(NC_UPDATE_BIN)) {
      nc_update_output("NC:UPDATE Remove UPDATE.BIN ERR");
      nc_update_output("NC:UPDATE Verified UPDATE.TMP retained");
      return false;
    }

    nc_update_output("NC:UPDATE Existing UPDATE.BIN removed");
  }

  nc_update_output("NC:UPDATE Renaming UPDATE.TMP -> UPDATE.BIN");

  if (!sd.rename(NC_UPDATE_TMP, NC_UPDATE_BIN)) {
    nc_update_output("NC:UPDATE Rename UPDATE.TMP -> UPDATE.BIN ERR");
    nc_update_output("NC:UPDATE Verified UPDATE.TMP retained");
    return false;
  }

  nc_update_output("NC:UPDATE Rename UPDATE.TMP -> UPDATE.BIN OK");
  nc_update_output("NC:UPDATE Verified; UPDATE.BIN ready");

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
bool notecard_update_ready(NCUI &info)
{
  info.length = 0;
  info.crc32 = 0;

  if (!notecard_exists) {
    nc_update_output("NC:UPDATE Notecard unavailable");
    return false;
  }

  // ==============================================================
  // Get the dfu.status for user dfu file aka feather firmware file
  // ==============================================================
  J *req = notecard.newRequest("dfu.status");

  if (req == NULL) {
    nc_update_output("NC:UPDATE New(dfu.status) ERR");
    return false;
  }

  JAddStringToObject(req, "name", "user");

  // requestAndResponse() takes ownership of req.
  J *rsp = notecard.requestAndResponse(req);

  if (rsp == NULL) {
    notecard_exists = false;
    nc_update_output("NC:UPDATE dfu.status NO RESPONSE");
    return false;
  }

  if (JHasObjectItem(rsp, "err")) {
    nc_update_output("NC:UPDATE dfu.status ERR %.100s",
                     JGetString(rsp, "err"));

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
    nc_update_output("NC:UPDATE Not ready: %.24s",
                     mode ? mode : "unknown");

    notecard.deleteResponse(rsp);
    return false;
  }

  NCUI candidate = {};
  J *body = JGetObjectItem(rsp, "body");

  bool valid =
      nc_update_uint32(body, "length", candidate.length) &&
      nc_update_uint32(body, "crc32", candidate.crc32) &&
      candidate.length > 0;

  notecard.deleteResponse(rsp);

  if (!valid) {
    nc_update_output("NC:UPDATE Invalid length/CRC32 metadata");
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
    nc_update_output("NC:UPDATE New(dfu.get) ERR");
    return false;
  }
  JAddNumberToObject(req, "length", 0);

  rsp = notecard.requestAndResponse(req);

  if (rsp == NULL) {
    notecard_exists = false;
    nc_update_output("NC:UPDATE dfu.get probe NO RESPONSE");
    return false;
  }

  if (JHasObjectItem(rsp, "err")) {
    nc_update_output("NC:UPDATE Not readable: %.100s",
                     JGetString(rsp, "err"));

    notecard.deleteResponse(rsp);
    return false;
  }

  notecard.deleteResponse(rsp);

  info = candidate;

  nc_update_output("NC:UPDATE READY bytes=%lu CRC32=%08lX",
                   static_cast<unsigned long>(info.length),
                   static_cast<unsigned long>(info.crc32));

  return true;
}



