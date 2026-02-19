/*
 * ======================================================================================================================
 *  obs.h - Observation Definations
 * ====================================================================================================================== 
 */
#include <time.h>  // defines time_t

#define OBSERVATION_INTERVAL      60   // Seconds
#define MAX_SENSORS         48
#define MAX_OBS_SIZE  1024


typedef enum {
  F_OBS, 
  I_OBS, 
  U_OBS
} OBS_TYPE;

typedef struct {
  char          id[6];       // Suport 4 character length observation names
  int           type;
  float         f_obs;
  int           i_obs;
  unsigned long u_obs;
  bool          inuse;
} SENSOR;

typedef struct {
  bool            inuse;                // Set to true when an observation is stored here         
  time_t          ts;                   // TimeStamp
  float           bv;                   // Lipo Battery Voltage
  unsigned long   hth;                  // System Status Bits
  SENSOR          sensor[MAX_SENSORS];
} OBSERVATION_STR;

// Extern variables
extern OBSERVATION_STR obs;
extern char obsbuf[MAX_OBS_SIZE];

// Function prototypes
int SendMsg(char *msg);
void OBS_Clear();
void OBS_N2S_Add();
void OBS_LOG_Add();
bool OBS_Build();
void OBS_N2S_Save();
void OBS_Take();
void OBS_Do();

