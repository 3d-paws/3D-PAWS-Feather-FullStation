# EEPROM Module

[← Top](../README.md)<br>

The station uses an external EEPROM module to preserve rainfall totals and the position in the SD card's Need to Send file across station reboots. The EEPROM also provides storage for `CONFIG.TXT` and `CONFIG.TMP` using two file-data slots and primary/backup directory blocks.

## Hardware

Use the [Adafruit 24LC32 I2C EEPROM Breakout — 32Kbit / 4 KB — STEMMA QT](https://www.adafruit.com/product/5146).

The module provides 4,096 bytes of nonvolatile storage and a 32-byte page-write buffer. Its default I2C address is `0x50`; address jumpers allow multiple EEPROM devices on the same bus. Data remains stored without power. [web:16]

## Wiring

Connect the module to the station's I2C bus using a STEMMA QT cable or the corresponding power, ground, SDA, and SCL connections. The breakout supports STEMMA QT / Qwiic connections. [web:16]

Use power and I2C logic levels compatible with the station controller. See the linked product documentation for electrical requirements.

## Memory Organization

The memory map below uses 32-byte blocks. Block numbers are zero-based, and address ranges include both endpoints. These blocks correspond to the device's 32-byte page size. [web:16]

- Block 0 contains the 28-byte `EEPROM_NVM` structure.
- The remaining space in block 0 and blocks 1–15 remain reserved.
- Blocks 16 and 17 contain the primary and backup directories.
- The remaining memory is divided into two 1,760-byte file-data slots.

```C
/*
 * ======================================================================================================================
 *  EEPROM Memory Layout - 24LC32, 4096 bytes, 32-byte blocks.
 *
 *  Area                    Blocks       Address Range       Size
 *  ----------------------  -----------  ------------------  ----------
 *  EEPROM_NVM              0 (partial)  0x0000-0x001B         28 bytes
 *  Reserved, block 0       0 (partial)  0x001C-0x001F          4 bytes
 *  Reserved                1-15         0x0020-0x01FF        480 bytes
 *  Directory primary       16           0x0200-0x021F         32 bytes
 *  Directory backup        17           0x0220-0x023F         32 bytes
 *  Slot 0 file data        18-72        0x0240-0x091F       1760 bytes
 *  Slot 1 file data        73-127       0x0920-0x0FFF       1760 bytes
 *
 *  The directory identifies which slot currently belongs to CONFIG.TXT and
 *  CONFIG.TMP. A rename changes only the directory, not the file data.
 * ======================================================================================================================
 */
```

Layout clarification: the original example's file-data block numbers did not match its address ranges and sizes. This map preserves the supplied addresses and two 1,760-byte slots, correcting the block ranges to `18–72` and `73–127`. Each slot occupies 55 blocks. Confirm that the firmware uses these same boundaries before treating this document as the implemented layout.

## Persistent Station State — Block 0

`EEPROM_NVM` stores rainfall totals for both rain gauges, the timestamp of the last rain-state modification, the Need to Send file position, and a checksum.

```C
/*
 * ======================================================================================================================
 *  EEPROM_NVM - Persistent Rainfall and SD Transmission State, 28 bytes.
 *  Stored in EEPROM block 0, starting at address 0x0000.
 *
 *  Field       Type             Address Range      Size       Description
 *  ----------  ---------------  -----------------  ---------  ------------------------------------------------
 *  rgt1        float            0x0000-0x0003      4 bytes    Rain gauge 1 total today
 *  rgp1        float            0x0004-0x0007      4 bytes    Rain gauge 1 total prior
 *  rgt2        float            0x0008-0x000B      4 bytes    Rain gauge 2 total today
 *  rgp2        float            0x000C-0x000F      4 bytes    Rain gauge 2 total prior
 *  rgts        uint32_t         0x0010-0x0013      4 bytes    Rain gauge timestamp of last modification
 *  n2sfp       unsigned long    0x0014-0x0017      4 bytes    SD "need to send" file position
 *  checksum    unsigned long    0x0018-0x001B      4 bytes    Stored checksum
 *  Reserved    --               0x001C-0x001F      4 bytes    Remaining space in block 0
 *
 *  Block 0 spans addresses 0x0000-0x001F (32 bytes).
 *  This layout assumes 4-byte float and unsigned long fields, with no padding.
 * ======================================================================================================================
 */
```