/*
 * ======================================================================================================================
 *  eeprom.h - EEPROM Definations
 * ======================================================================================================================
 */
#include <Arduino.h>

#define EEPROM_I2C_ADDR 0x50

/*
 * ======================================================================================================================
 *  EEPROM NonVolitileMemory - Stores rain totals in persistant memory, structure must no exceed 32 bytes.
 *                             On a 24LC32, the largest safe single write is one page = 32 bytes, 
 *                             and you must not cross a page boundary in that write 
 * 
 * The 24LC32:
 * Has 4 KB total (4096 bytes), addressed 0–4095.
 * Uses 32‑byte pages for writes.
 * Page boundaries are at addresses that are multiples of 32: 0, 32, 64, 96, …, 4064
 * ======================================================================================================================
 */

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
 *  Unused      --               0x001C-0x001F      4 bytes    Remaining space in block 0
 *
 *  Block 0 spans addresses 0x0000-0x001F (32 bytes).
 *  This layout assumes 4-byte float and unsigned long fields, with no padding.
 * ======================================================================================================================
 */
typedef struct {
    float    rgt1;       // rain gauge total today
    float    rgp1;       // rain gauge total prior
    float    rgt2;       // rain gauge 2 total today
    float    rgp2;       // rain gauge 2 total prior
    uint32_t rgts;       // rain gauge timestamp of last modification
    unsigned long n2sfp; // sd need 2 send file position
    unsigned long checksum;
} EEPROM_NVM;  // Above is 28 bytes.

// Extern variables
extern EEPROM_NVM eeprom;
extern bool eeprom_valid;
extern bool eeprom_exists;

// Function prototype
unsigned long EEPROM_ChecksumCompute();
void EEPROM_ChecksumUpdate();
void EEPROM_ClearRainTotals(uint32_t current_time);
bool EEPROM_TimeToRollOver();
void EEPROM_Validate();
void EEPROM_UpdateRainTotals(float rgt1, float rgt2);
void EEPROM_SaveUnreportedRain();
void EEPROM_Update();
void EEPROM_Dump();
void EEPROM_initialize();