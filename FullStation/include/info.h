/*
 * ======================================================================================================================
 *  info.h - Station Information Definations
 * ======================================================================================================================
 */

// Extern variables
#define INFO_TIME_INTERVAL  3600*6*1000         // milli seconds 6 hours
#define INFO_LW_WAIT_BTWN_TM    30              // Seconds
#define INFO_LW_WAIT_BUF_CLR    60              // Seconds

// Function prototypes
void INFO_Initialize();
void INFO_Do();