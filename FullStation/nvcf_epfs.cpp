/*
 * ======================================================================================================================
 *  nvcf_epfs.cpp - EProm File System Functions
 * ======================================================================================================================
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <Adafruit_EEPROM_I2C.h>

#include "include/main.h"
#include "include/output.h"
#include "include/nvcf_epfs.h"


/*
 * ======================================================================================================================
 *  Constants and Private Definitions
 * ======================================================================================================================
 */
#define EPFS_DIRECTORY_SIZE      32
#define EPFS_DIRECTORY_VERSION   1
#define EPFS_IO_CHUNK            16

#define EPFS_ENTRY_TXT           0
#define EPFS_ENTRY_TMP           1

/*
 * ======================================================================================================================
 *  EPFS_Directory - RAM copy of the EEPROM directory.
 *
 *  Slot values are 0, 1, or EPFS_SLOT_EMPTY.
 * ======================================================================================================================
 */
typedef struct {
  uint8_t txtSlot;
  uint16_t txtLength;
  uint8_t tmpSlot;
  uint16_t tmpLength;
} EPFS_Directory;

/*
 * ======================================================================================================================
 *  EPFS_State - Runtime state. Resides in RAM and is never written directly to EEPROM.
 * ======================================================================================================================
 */
typedef struct {
  EPFS_Directory directory;
  uint32_t generation;
  int openEntry;
  int accessMode;
  uint16_t fileLength;
  uint16_t filePosition;
  uint16_t cachePosition;
  uint8_t readCache[EPFS_CACHE_SIZE];
  uint8_t cacheLength;
  bool appendMode;
  bool lengthDirty;
  bool isOpen;
  bool initialized;
} EPFS_State;

static EPFS_State epfs_state = {};
static Adafruit_EEPROM_I2C epfs_eeprom;

int epfs_errno = EPFS_OK;


/*
 * ======================================================================================================================
 *  Function Declarations
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  epfs_Error() - Set epfs_errno and return -1.
 * ======================================================================================================================
 */
static int epfs_Error(int error) {
  epfs_errno = error;
  return -1;
}


/*
 * ======================================================================================================================
 *  epfs_Valid() - Check the single active file descriptor.
 * ======================================================================================================================
 */
static bool epfs_Valid(int fd) {
  if (!epfs_state.isOpen || fd != EPFS_FILE_DESCRIPTOR) {
    epfs_errno = EPFS_EBADF;
    return false;
  }

  return true;
}


/*
 * ======================================================================================================================
 *  epfs_Read() - Read bytes only within the EPFS region.
 * ======================================================================================================================
 */
static bool epfs_Read(uint16_t address, uint8_t *buffer, size_t count) {
  if (address < EPFS_BASE_ADDRESS ||
      address >= EPFS_BASE_ADDRESS + 3584 ||
      count > (size_t)(EPFS_BASE_ADDRESS + 3584 - address)) {
    epfs_errno = EPFS_EINVAL;
    return false;
  }

  if (!epfs_eeprom.read(address, buffer, (uint16_t)count)) {
    epfs_errno = EPFS_EIO;
    return false;
  }

  return true;
}


/*
 * ======================================================================================================================
 *  epfs_Write() - Write bytes only within the EPFS region.
 *  Adafruit_EEPROM_I2C handles page boundaries and write-cycle polling.
 * ======================================================================================================================
 */
static bool epfs_Write(uint16_t address, const uint8_t *buffer, size_t count) {
  if (address < EPFS_BASE_ADDRESS ||
      address >= EPFS_BASE_ADDRESS + 3584 ||
      count > (size_t)(EPFS_BASE_ADDRESS + 3584 - address)) {
    epfs_errno = EPFS_EINVAL;
    return false;
  }

  if (!epfs_eeprom.write(address, (uint8_t *)buffer, (uint16_t)count)) {
    epfs_errno = EPFS_EIO;
    return false;
  }

  return true;
}


/*
 * ======================================================================================================================
 *  epfs_CRC() - Calculate CRC-32 using polynomial 0xEDB88320.
 * ======================================================================================================================
 */
static uint32_t epfs_CRC(const uint8_t *data, size_t count) {
  uint32_t crc = 0xFFFFFFFFUL;

  while (count > 0) {
    crc ^= *data++;

    for (uint8_t bit = 0; bit < 8; ++bit) {
      if ((crc & 1) != 0) {
        crc = (crc >> 1) ^ 0xEDB88320UL;
      }
      else {
        crc >>= 1;
      }
    }

    --count;
  }

  return crc ^ 0xFFFFFFFFUL;
}


/*
 * ======================================================================================================================
 *  epfs_LoadDirectory() - Load the newest valid directory copy.
 *
 *  Directory image:
 *    0-2    Signature 'E', 'P', 'F'
 *    3      Version
 *    4-7    Generation
 *    8      CONFIG.TXT slot
 *    9      Reserved
 *    10-11  CONFIG.TXT length
 *    12     CONFIG.TMP slot
 *    13     Reserved
 *    14-15  CONFIG.TMP length
 *    16-27  Reserved
 *    28-31  CRC of bytes 0-27
 * ======================================================================================================================
 */
static bool epfs_LoadDirectory() {
  uint8_t primary[EPFS_DIRECTORY_SIZE];
  uint8_t backup[EPFS_DIRECTORY_SIZE];
  bool primaryValid = false;
  bool backupValid = false;
  uint32_t primaryGeneration = 0;
  uint32_t backupGeneration = 0;

  epfs_state.directory.txtSlot = EPFS_SLOT_EMPTY;
  epfs_state.directory.txtLength = 0;
  epfs_state.directory.tmpSlot = EPFS_SLOT_EMPTY;
  epfs_state.directory.tmpLength = 0;
  epfs_state.generation = 0;

  if (!epfs_Read(EPFS_DIRECTORY_PRIMARY, primary, sizeof(primary))) {
    return false;
  }

  if (!epfs_Read(EPFS_DIRECTORY_BACKUP, backup, sizeof(backup))) {
    return false;
  }

  if (primary[0] == 'E' &&
      primary[1] == 'P' &&
      primary[2] == 'F' &&
      primary[3] == EPFS_DIRECTORY_VERSION &&
      epfs_CRC(primary, 28) ==
        ((uint32_t)primary[28] |
         ((uint32_t)primary[29] << 8) |
         ((uint32_t)primary[30] << 16) |
         ((uint32_t)primary[31] << 24))) {
    primaryValid = true;
    primaryGeneration =
      (uint32_t)primary[4] |
      ((uint32_t)primary[5] << 8) |
      ((uint32_t)primary[6] << 16) |
      ((uint32_t)primary[7] << 24);
  }

  if (backup[0] == 'E' &&
      backup[1] == 'P' &&
      backup[2] == 'F' &&
      backup[3] == EPFS_DIRECTORY_VERSION &&
      epfs_CRC(backup, 28) ==
        ((uint32_t)backup[28] |
         ((uint32_t)backup[29] << 8) |
         ((uint32_t)backup[30] << 16) |
         ((uint32_t)backup[31] << 24))) {
    backupValid = true;
    backupGeneration =
      (uint32_t)backup[4] |
      ((uint32_t)backup[5] << 8) |
      ((uint32_t)backup[6] << 16) |
      ((uint32_t)backup[7] << 24);
  }

  if (!primaryValid && !backupValid) {
    return true;
  }

  const uint8_t *selected = primary;
  uint32_t selectedGeneration = primaryGeneration;

  if (!primaryValid) {
    selected = backup;
    selectedGeneration = backupGeneration;
  }
  else if (backupValid) {
    uint32_t difference = backupGeneration - primaryGeneration;

    if (difference != 0 && difference < 0x80000000UL) {
      selected = backup;
      selectedGeneration = backupGeneration;
    }
  }

  epfs_state.generation = selectedGeneration;
  epfs_state.directory.txtSlot = selected[8];
  epfs_state.directory.txtLength =
    (uint16_t)selected[10] | ((uint16_t)selected[11] << 8);
  epfs_state.directory.tmpSlot = selected[12];
  epfs_state.directory.tmpLength =
    (uint16_t)selected[14] | ((uint16_t)selected[15] << 8);

  if (epfs_state.directory.txtSlot >= EPFS_SLOT_COUNT) {
    epfs_state.directory.txtSlot = EPFS_SLOT_EMPTY;
    epfs_state.directory.txtLength = 0;
  }

  if (epfs_state.directory.tmpSlot >= EPFS_SLOT_COUNT) {
    epfs_state.directory.tmpSlot = EPFS_SLOT_EMPTY;
    epfs_state.directory.tmpLength = 0;
  }

  return true;
}


/*
 * ======================================================================================================================
 *  epfs_SaveDirectory() - Write the directory to the alternate directory block.
 *  The prior directory remains valid if this write is interrupted.
 * ======================================================================================================================
 */
static bool epfs_SaveDirectory() {
  uint8_t image[EPFS_DIRECTORY_SIZE] = {};
  uint32_t generation = epfs_state.generation + 1;
  uint16_t address;

  if ((generation & 1) != 0) {
    address = EPFS_DIRECTORY_BACKUP;
  }
  else {
    address = EPFS_DIRECTORY_PRIMARY;
  }

  image[0] = 'E';
  image[1] = 'P';
  image[2] = 'F';
  image[3] = EPFS_DIRECTORY_VERSION;

  image[4] = (uint8_t)generation;
  image[5] = (uint8_t)(generation >> 8);
  image[6] = (uint8_t)(generation >> 16);
  image[7] = (uint8_t)(generation >> 24);

  image[8] = epfs_state.directory.txtSlot;
  image[10] = (uint8_t)epfs_state.directory.txtLength;
  image[11] = (uint8_t)(epfs_state.directory.txtLength >> 8);

  image[12] = epfs_state.directory.tmpSlot;
  image[14] = (uint8_t)epfs_state.directory.tmpLength;
  image[15] = (uint8_t)(epfs_state.directory.tmpLength >> 8);

  uint32_t crc = epfs_CRC(image, 28);

  image[28] = (uint8_t)crc;
  image[29] = (uint8_t)(crc >> 8);
  image[30] = (uint8_t)(crc >> 16);
  image[31] = (uint8_t)(crc >> 24);

  if (!epfs_Write(address, image, sizeof(image))) {
    return false;
  }

  epfs_state.generation = generation;
  return true;
}


/*
 * ======================================================================================================================
 *  epfs_Begin() - Initialize EPFS and load the directory.
 *  Call Wire.begin() before calling this function.
 *  No valid CONFIG.TXT is an acceptable initial state.
 * ======================================================================================================================
 */
bool epfs_Begin() {
  if (epfs_state.isOpen) {
    epfs_errno = EPFS_EBUSY;
    return false;
  }

  epfs_state = EPFS_State{};
  epfs_state.openEntry = -1;

  if (!epfs_eeprom.begin(EPFS_EEPROM_ADDRESS)) {
    epfs_errno = EPFS_EIO;
    return false;
  }

  if (!epfs_LoadDirectory()) {
    return false;
  }

  epfs_state.initialized = true;
  epfs_errno = EPFS_OK;

  return true;
}


/*
 * ======================================================================================================================
 *  epfs_Format() - Discard both configuration files and reset the directory.
 *  Does not modify reserved blocks 0-15. Do not call on every startup.
 * ======================================================================================================================
 */
bool epfs_Format() {
  if (epfs_state.isOpen) {
    epfs_errno = EPFS_EBUSY;
    return false;
  }

  if (!epfs_state.initialized) {
    epfs_errno = EPFS_ESTATE;
    return false;
  }

  uint8_t zeros[EPFS_BLOCK_SIZE] = {};

  /* Clear both directory copies. */
  if (!epfs_Write(EPFS_DIRECTORY_PRIMARY, zeros, sizeof(zeros)) ||
      !epfs_Write(EPFS_DIRECTORY_BACKUP, zeros, sizeof(zeros))) {
    return false;
  }

  /* Clear both file-data slots. */
  for (uint8_t slot = 0; slot < EPFS_SLOT_COUNT; ++slot) {
    uint16_t address = EPFS_SLOT_DATA_ADDRESS(slot);

    for (uint16_t offset = 0; offset < EPFS_FILE_CAPACITY; offset += sizeof(zeros)) {
      if (!epfs_Write(address + offset, zeros, sizeof(zeros))) {
        return false;
      }
    }
  }

  epfs_state.directory.txtSlot = EPFS_SLOT_EMPTY;
  epfs_state.directory.txtLength = 0;
  epfs_state.directory.tmpSlot = EPFS_SLOT_EMPTY;
  epfs_state.directory.tmpLength = 0;
  epfs_state.generation = 0;

  return true;
}

/*
 * ======================================================================================================================
 *  epfs_FindEntry() - Return the directory entry for a file name.
 * ======================================================================================================================
 */
static int epfs_FindEntry(const char *name) {
  if (name == NULL) {
    return -1;
  }

  if (strcmp(name, EPFS_CONFIG_NAME) == 0) {
    return EPFS_ENTRY_TXT;
  }

  if (strcmp(name, EPFS_TEMP_NAME) == 0) {
    return EPFS_ENTRY_TMP;
  }

  return -1;
}


/*
 * ======================================================================================================================
 *  epfs_EntryAddress() - Return the data address for a directory entry.
 * ======================================================================================================================
 */
static uint16_t epfs_EntryAddress(int entry) {
  uint8_t slot = entry == EPFS_ENTRY_TXT
    ? epfs_state.directory.txtSlot
    : epfs_state.directory.tmpSlot;

  return EPFS_SLOT_DATA_ADDRESS(slot);
}


/*
 * ======================================================================================================================
 *  epfs_EntrySlot() - Return the slot field for a directory entry.
 * ======================================================================================================================
 */
static uint8_t *epfs_EntrySlot(int entry) {
  return entry == EPFS_ENTRY_TXT
    ? &epfs_state.directory.txtSlot
    : &epfs_state.directory.tmpSlot;
}


/*
 * ======================================================================================================================
 *  epfs_EntryLength() - Return the length field for a directory entry.
 * ======================================================================================================================
 */
static uint16_t *epfs_EntryLength(int entry) {
  return entry == EPFS_ENTRY_TXT
    ? &epfs_state.directory.txtLength
    : &epfs_state.directory.tmpLength;
}


/*
 * ======================================================================================================================
 *  epfs_Open() - Open CONFIG.TXT or CONFIG.TMP.
 *
 *  CONFIG.TXT:
 *    O_RDONLY - Read the committed configuration.
 *
 *  CONFIG.TMP:
 *    O_WRONLY | O_CREAT | O_TRUNC - Build a complete replacement.
 *
 *  Only one file may be open at a time.
 * ======================================================================================================================
 */
int epfs_Open(const char *name, int flags) {
  if (!epfs_state.initialized) {
    return epfs_Error(EPFS_ESTATE);
  }

  if (epfs_state.isOpen) {
    return epfs_Error(EPFS_EBUSY);
  }

  int entry = epfs_FindEntry(name);

  if (entry < 0) {
    return epfs_Error(EPFS_EINVAL);
  }

  if (entry == EPFS_ENTRY_TXT && flags != O_RDONLY) {
    return epfs_Error(EPFS_EROFS);
  }

  if (entry == EPFS_ENTRY_TMP &&
      flags != (O_WRONLY | O_CREAT | O_TRUNC)) {
    return epfs_Error(EPFS_EINVAL);
  }

  uint8_t *slot = epfs_EntrySlot(entry);
  uint16_t *length = epfs_EntryLength(entry);

  if (entry == EPFS_ENTRY_TXT && *slot == EPFS_SLOT_EMPTY) {
    return epfs_Error(EPFS_ENOENT);
  }

  if (entry == EPFS_ENTRY_TMP) {
    /*
     * Reuse the existing temporary slot if one is already assigned.
     * Otherwise, choose the slot not used by CONFIG.TXT.
     */
    if (*slot == EPFS_SLOT_EMPTY) {
      if (epfs_state.directory.txtSlot == 0) {
        *slot = 1;
      }
      else {
        *slot = 0;
      }
    }

    *length = 0;
  }

  epfs_state.openEntry = entry;
  epfs_state.accessMode = flags & O_ACCMODE;
  epfs_state.appendMode = (flags & O_APPEND) != 0;
  epfs_state.fileLength = *length;
  epfs_state.filePosition = 0;
  epfs_state.cachePosition = 0;
  epfs_state.cacheLength = 0;
  epfs_state.lengthDirty = false;
  epfs_state.isOpen = true;

  return EPFS_FILE_DESCRIPTOR;
}


/*
 * ======================================================================================================================
 *  epfs_FileSize() - Return the open file's size in bytes, or -1 on error.
 * ======================================================================================================================
 */
ssize_t epfs_FileSize(int fd) {
  if (!epfs_Valid(fd)) {
    return -1;
  }

  return (ssize_t)epfs_state.fileLength;
}


/*
 * ======================================================================================================================
 *  epfs_FileRead() - Read from the current file position.
 *  Uses a read cache for repeated single-character reads.
 * ======================================================================================================================
 */
ssize_t epfs_FileRead(int fd, void *buffer, size_t count) {
  if (!epfs_Valid(fd)) {
    return -1;
  }

  if (epfs_state.accessMode == O_WRONLY) {
    return epfs_Error(EPFS_EBADF);
  }

  if (count > 0 && buffer == NULL) {
    return epfs_Error(EPFS_EFAULT);
  }

  uint8_t *destination = (uint8_t *)buffer;
  size_t done = 0;

  while (done < count &&
         epfs_state.filePosition < epfs_state.fileLength) {
    uint16_t cacheEnd =
      epfs_state.cachePosition + epfs_state.cacheLength;

    if (epfs_state.cacheLength == 0 ||
        epfs_state.filePosition < epfs_state.cachePosition ||
        epfs_state.filePosition >= cacheEnd) {
      size_t available =
        epfs_state.fileLength - epfs_state.filePosition;

      if (available > sizeof(epfs_state.readCache)) {
        available = sizeof(epfs_state.readCache);
      }

/* 
      // Debug
      uint16_t readAddress =
        epfs_EntryAddress(epfs_state.openEntry) + epfs_state.filePosition;

      snprintf(msgbuf, sizeof(msgbuf),
               "EPFS Read: addr=0x%04X count=%u",
               readAddress,
               (unsigned)available);
      Output(msgbuf);
*/

      epfs_state.cacheLength = 0;

      if (!epfs_Read(epfs_EntryAddress(epfs_state.openEntry) +
                     epfs_state.filePosition,
                     epfs_state.readCache,
                     available)) {
/* 
        // Debug
        snprintf(msgbuf, sizeof(msgbuf),
                 "EPFS Read FAILED: addr=0x%04X count=%u errno=%d",
                 readAddress,
                 (unsigned)available,
                 epfs_errno);
        Output(msgbuf);
*/

        if (done > 0) {
          return (ssize_t)done;
        }

        return -1;
      }

      epfs_state.cachePosition = epfs_state.filePosition;
      epfs_state.cacheLength = (uint8_t)available;
    }

    size_t cacheOffset =
      epfs_state.filePosition - epfs_state.cachePosition;

    size_t amount = epfs_state.cacheLength - cacheOffset;

    if (amount > count - done) {
      amount = count - done;
    }

    memcpy(destination + done,
           epfs_state.readCache + cacheOffset,
           amount);

    epfs_state.filePosition += amount;
    done += amount;
  }

  return (ssize_t)done;
}


/*
 * ======================================================================================================================
 *  epfs_FileWrite() - Write at the current position or EOF when opened with O_APPEND.
 *  Data writes occur immediately. The directory length is committed by Sync() or Close().
 * ======================================================================================================================
 */
ssize_t epfs_FileWrite(int fd, const void *buffer, size_t count) {
  if (!epfs_Valid(fd)) {
    return -1;
  }

  if (epfs_state.accessMode == O_RDONLY) {
    return epfs_Error(EPFS_EBADF);
  }

  if (count > 0 && buffer == NULL) {
    return epfs_Error(EPFS_EFAULT);
  }

  if (count == 0) {
    return 0;
  }

  if (epfs_state.appendMode) {
    epfs_state.filePosition = epfs_state.fileLength;
  }

  if (epfs_state.filePosition >= EPFS_FILE_CAPACITY) {
    return epfs_Error(EPFS_ENOSPC);
  }

  size_t room = EPFS_FILE_CAPACITY - epfs_state.filePosition;

  if (count > room) {
    count = room;
  }

  epfs_state.cacheLength = 0;

  if (!epfs_Write(epfs_EntryAddress(epfs_state.openEntry) +
                  epfs_state.filePosition,
                  (const uint8_t *)buffer,
                  count)) {
    return -1;
  }

  epfs_state.filePosition += count;

  if (epfs_state.filePosition > epfs_state.fileLength) {
    epfs_state.fileLength = epfs_state.filePosition;
    epfs_state.lengthDirty = true;
  }

  return (ssize_t)count;
}


/*
 * ======================================================================================================================
 *  epfs_FileSeek() - Move the file position.
 *  Seeking beyond EOF is allowed for writes; it creates a sparse gap.
 * ======================================================================================================================
 */
int epfs_FileSeek(int fd, long offset, int whence) {
  if (!epfs_Valid(fd)) {
    return -1;
  }

  long position;

  if (whence == SEEK_SET) {
    position = offset;
  }
  else if (whence == SEEK_CUR) {
    position = (long)epfs_state.filePosition + offset;
  }
  else if (whence == SEEK_END) {
    position = (long)epfs_state.fileLength + offset;
  }
  else {
    return epfs_Error(EPFS_EINVAL);
  }

  if (position < 0 || position > EPFS_FILE_CAPACITY) {
    return epfs_Error(EPFS_EINVAL);
  }

  epfs_state.filePosition = (uint16_t)position;
  epfs_state.cacheLength = 0;

  return 0;
}


/*
 * ======================================================================================================================
 *  epfs_Sync() - Commit pending file-length metadata to the directory.
 *  Data writes themselves have already completed.
 * ======================================================================================================================
 */
int epfs_Sync(int fd) {
  if (!epfs_Valid(fd)) {
    return -1;
  }

  if (epfs_state.lengthDirty) {
    *epfs_EntryLength(epfs_state.openEntry) = epfs_state.fileLength;

    if (!epfs_SaveDirectory()) {
      return -1;
    }

    epfs_state.lengthDirty = false;
  }

  return 0;
}


/*
 * ======================================================================================================================
 *  epfs_Close() - Commit pending metadata and release the descriptor.
 * ======================================================================================================================
 */
int epfs_Close(int fd) {
  if (!epfs_Valid(fd)) {
    return -1;
  }

  int result = epfs_Sync(fd);

  epfs_state.isOpen = false;
  epfs_state.cacheLength = 0;

  return result;
}


/*
 * ======================================================================================================================
 *  epfs_Rename() - Rename CONFIG.TMP to CONFIG.TXT.
 *
 *  Only the directory changes. The prior CONFIG.TXT remains valid until this
 *  directory update succeeds. CONFIG.TMP becomes an empty file.
 * ======================================================================================================================
 */
int epfs_Rename(const char *from, const char *to) {
  if (!epfs_state.initialized) {
    return epfs_Error(EPFS_ESTATE);
  }

  if (epfs_state.isOpen) {
    return epfs_Error(EPFS_EBUSY);
  }

  if (from == NULL || to == NULL) {
    return epfs_Error(EPFS_EINVAL);
  }

  if (strcmp(from, EPFS_TEMP_NAME) != 0 ||
      strcmp(to, EPFS_CONFIG_NAME) != 0) {
    return epfs_Error(EPFS_EINVAL);
  }

  if (epfs_state.directory.tmpSlot == EPFS_SLOT_EMPTY) {
    return epfs_Error(EPFS_ENOENT);
  }

  epfs_state.directory.txtSlot = epfs_state.directory.tmpSlot;
  epfs_state.directory.txtLength = epfs_state.directory.tmpLength;
  epfs_state.directory.tmpSlot = EPFS_SLOT_EMPTY;
  epfs_state.directory.tmpLength = 0;

  if (!epfs_SaveDirectory()) {
    return -1;
  }

  return 0;
}


/*
 * ======================================================================================================================
 *  epfs_Unlink() - Remove a file by marking its directory entry empty.
 *  The data slot is not erased; it is reused by the next temporary file.
 * ======================================================================================================================
 */
int epfs_Unlink(const char *name) {
  if (!epfs_state.initialized) {
    return epfs_Error(EPFS_ESTATE);
  }

  if (epfs_state.isOpen) {
    return epfs_Error(EPFS_EBUSY);
  }

  int entry = epfs_FindEntry(name);

  if (entry < 0) {
    return epfs_Error(EPFS_EINVAL);
  }

  if (*epfs_EntrySlot(entry) == EPFS_SLOT_EMPTY) {
    return epfs_Error(EPFS_ENOENT);
  }

  *epfs_EntrySlot(entry) = EPFS_SLOT_EMPTY;
  *epfs_EntryLength(entry) = 0;

  if (!epfs_SaveDirectory()) {
    return -1;
  }

  return 0;
}

/*
 * ======================================================================================================================
 * epfs_PrintInfo() - Print EPFS filesystem information and directory status.
 * ======================================================================================================================
 */
void epfs_PrintInfo() {
  if (!epfs_state.initialized) {
    Output("EPFS:Not Initialized");
    return;
  }

  Output("EPFS Filesystem Information");
  Output("EPFS:EEPROM Address: 0x50");
  Output("EPFS:Reserved Blocks: 0-15");
  Output("EPFS:Filesystem Base: 0x0200");
  Output("EPFS:File Capacity: 1760 bytes");
  Output("EPFS:Slot Count: 2");

  snprintf(msgbuf, sizeof(msgbuf), "EPFS:Directory Generation: %lu",
           (unsigned long)epfs_state.generation);
  Output(msgbuf);

  /* CONFIG.TXT */
  if (epfs_state.directory.txtSlot == EPFS_SLOT_EMPTY) {
    Output("EPFS:CONFIG.TXT: Not Present");
  }
  else {
    snprintf(msgbuf, sizeof(msgbuf),
             "EPFS:CONFIG.TXT: Slot %u, Length %u, Address 0x%04X",
             epfs_state.directory.txtSlot,
             epfs_state.directory.txtLength,
             EPFS_SLOT_DATA_ADDRESS(epfs_state.directory.txtSlot));
    Output(msgbuf);

    snprintf(msgbuf, sizeof(msgbuf), "EPFS:CONFIG.TXT Free: %u bytes",
             EPFS_FILE_CAPACITY - epfs_state.directory.txtLength);
    Output(msgbuf);
  }

  /* CONFIG.TMP */
  if (epfs_state.directory.tmpSlot == EPFS_SLOT_EMPTY) {
    Output("EPFS:CONFIG.TMP: Not Present");
  }
  else {
    snprintf(msgbuf, sizeof(msgbuf),
             "EPFS:CONFIG.TMP: Slot %u, Length %u, Address 0x%04X",
             epfs_state.directory.tmpSlot,
             epfs_state.directory.tmpLength,
             EPFS_SLOT_DATA_ADDRESS(epfs_state.directory.tmpSlot));
    Output(msgbuf);

    snprintf(msgbuf, sizeof(msgbuf), "EPFS:CONFIG.TMP Free: %u bytes",
             EPFS_FILE_CAPACITY - epfs_state.directory.tmpLength);
    Output(msgbuf);
  }

  /* Slot ownership */
  if (epfs_state.directory.txtSlot != EPFS_SLOT_EMPTY &&
      epfs_state.directory.tmpSlot != EPFS_SLOT_EMPTY) {
    Output("EPFS:Both Slots Assigned");
  }
  else if (epfs_state.directory.txtSlot != EPFS_SLOT_EMPTY) {
    Output("EPFS:One Slot Assigned To CONFIG.TXT");
  }
  else if (epfs_state.directory.tmpSlot != EPFS_SLOT_EMPTY) {
    Output("EPFS:One Slot Assigned To CONFIG.TMP");
  }
  else {
    Output("EPFS:No Slots Assigned");
  }

  /* Open file */
  if (epfs_state.isOpen) {
    const char *name = epfs_state.openEntry == EPFS_ENTRY_TXT
      ? EPFS_CONFIG_NAME
      : EPFS_TEMP_NAME;

    snprintf(msgbuf, sizeof(msgbuf),
             "EPFS:Open File: %s, Position %u, Length %u",
             name,
             epfs_state.filePosition,
             epfs_state.fileLength);
    Output(msgbuf);
  }
  else {
    Output("EPFS:No File Open");
  }
}
