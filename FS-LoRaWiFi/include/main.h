/*
 * ======================================================================================================================
 *  main.h - Main Code Definations
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Loop Timers
 * ======================================================================================================================
 */


 /*
 * ======================================================================================================================
 *  Relay Power Control Pin
 * ======================================================================================================================
 */
#define REBOOT_PIN        A0  // Connect to WatchDog Trigger
#define HEARTBEAT_PIN     A1  // Connect to WatchDog Heartbeat

#define MAX_MSGBUF_SIZE   1024


// Extern variables
extern char versioninfo[];
extern bool JustPoweredOn;              // Used to clear SystemStatusBits set during power on device discovery
extern bool TurnLedOff;                 // Set true in rain gauge interrupt

extern char msgbuf[MAX_MSGBUF_SIZE];    // Used to hold messages
extern char *msgp;                      // Pointer to message text
extern char Buffer32Bytes[32];          // General storage

extern unsigned long Time_of_obs;       // unix time of observation
extern unsigned long Time_of_next_obs;  // time of next observation

extern unsigned long nextinfo;          // Time of Next INFO transmit 

// Function prototypes
int time_to_next_obs();
void HeartBeat();
void BackGroundWork();

