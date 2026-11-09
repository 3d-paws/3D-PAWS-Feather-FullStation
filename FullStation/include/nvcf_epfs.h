/*
 * ======================================================================================================================
 *  nvcf_epfs.h - EProm File System Definitions
 *
 *  Two-file configuration store for a 24LC32 I2C EEPROM.
 *
 *  CONFIG.TXT - Committed configuration file.
 *  CONFIG.TMP - Temporary file used to build a complete replacement.
 * ======================================================================================================================
 */
#ifndef EPFS_H
#define EPFS_H

#include <Arduino.h>
#include <stddef.h>
#include <sys/types.h>

/*
 * ======================================================================================================================
 *  EEPROM Memory Layout - 24LC32, 4096 bytes, 32-byte blocks.
 *
 *  Area                    Blocks       Address Range       Size
 *  ----------------------  -----------  ------------------  ----------
 *  Reserved                0-15         0x0000-0x01FF        512 bytes
 *  Directory primary       16           0x0200-0x021F         32 bytes
 *  Directory backup        17           0x0220-0x023F         32 bytes
 *  Slot 0 file data        18-71        0x0240-0x091F       1760 bytes
 *  Slot 1 file data        72-125       0x0920-0x0FFF       1760 bytes
 *
 *  The directory identifies which slot currently belongs to CONFIG.TXT and
 *  CONFIG.TMP. A rename changes only the directory, not the file data.
 * ======================================================================================================================
 */

#define EPFS_EEPROM_ADDRESS    0x50
#define EPFS_EEPROM_SIZE       4096
#define EPFS_BLOCK_SIZE        32
#define EPFS_RESERVED_BLOCKS   16

#define EPFS_BASE_ADDRESS      (EPFS_RESERVED_BLOCKS * EPFS_BLOCK_SIZE)
#define EPFS_DIRECTORY_PRIMARY (EPFS_BASE_ADDRESS)
#define EPFS_DIRECTORY_BACKUP  (EPFS_BASE_ADDRESS + EPFS_BLOCK_SIZE)
#define EPFS_SLOT_DATA_ADDRESS(slot) (0x0240 + ((slot) * 1760))

#define EPFS_FILE_CAPACITY     1760
#define EPFS_SLOT_COUNT        2
#define EPFS_FILE_DESCRIPTOR   100
#define EPFS_CACHE_SIZE        32

#define EPFS_CONFIG_NAME       "CONFIG.TXT"
#define EPFS_TEMP_NAME         "CONFIG.TMP"

#define EPFS_SLOT_EMPTY        0xFF

/*
 * ======================================================================================================================
 *  EPFS_ErrorCode - Private error codes. Independent of the standard errno system.
 * ======================================================================================================================
 */
enum EPFS_ErrorCode {
  EPFS_OK = 0,
  EPFS_EBADF,
  EPFS_EBUSY,
  EPFS_EINVAL,
  EPFS_EIO,
  EPFS_ENOENT,
  EPFS_EEXIST,
  EPFS_EFAULT,
  EPFS_ENOSPC,
  EPFS_EROFS,
  EPFS_ECRC,
  EPFS_ESTATE
};

extern int epfs_errno;

/*
 * ======================================================================================================================
 *  Public Function Prototypes
 * ======================================================================================================================
 */
extern bool    epfs_Begin();
extern bool    epfs_Format();

extern int     epfs_Open(const char *name, int flags);
extern ssize_t epfs_FileSize(int fd);
extern ssize_t epfs_FileRead(int fd, void *buffer, size_t count);
extern ssize_t epfs_FileWrite(int fd, const void *buffer, size_t count);
extern int     epfs_FileSeek(int fd, long offset, int whence);
extern int     epfs_Sync(int fd);
extern int     epfs_Close(int fd);

extern int     epfs_Rename(const char *from, const char *to);
extern int     epfs_Unlink(const char *name);
extern void    epfs_PrintInfo();


#endif