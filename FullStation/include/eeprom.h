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
bool EEPROM_ChecksumValid();
void EEPROM_ClearRainTotals(uint32_t current_time);
void EEPROM_ClearRain2Totals();
bool EEPROM_TimeToRollOver();
void EEPROM_Validate();
void EEPROM_UpdateRainTotals(float rgt1, float rgt2);
void EEPROM_SaveUnreportedRain();
void EEPROM_Update();
void EEPROM_Dump();
void EEPROM_initialize();