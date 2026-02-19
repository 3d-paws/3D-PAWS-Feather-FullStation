/*
 * ======================================================================================================================
 *  sdcard.h - SD Card Definations
 * ======================================================================================================================
 */
#include <SdFat.h>

#define SD_MASK_INTERRUPS  0                // Do not mask interrups around sd card operations 
#define SD_ChipSelect     10                // GPIO 10 is Pin 10 on Feather and D5 on Particle Boron Board

// Extern variables
extern SdFat SD;
extern File SD_fp;
extern char SD_obsdir[];
extern bool SD_exists;
extern char SD_n2s_file[];
extern uint32_t SD_n2s_max_filesz;
extern char SD_crt_file[];
extern char SD_OPTAQS_FILE[];
extern char SD_INFO_FILE[];

// Function prototypes
void SD_initialize();
void SD_LogObservation(char *observations);
bool SD_N2S_Delete();
void SD_NeedToSend_Add(char *observation);
void SD_NeedToSend_Status(char *status);
void SD_ClearRainTotals();
void SD_UpdateInfoFile(char *info);
void SD_N2S_Publish();
