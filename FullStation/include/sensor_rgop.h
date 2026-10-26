/*
 * ======================================================================================================================
 *  sensor_rgop.h - Rain and OptionPin Definations
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Option Pin Defination Setup
 * ======================================================================================================================
 */
#define OP1_PIN  A4
#define OP2_PIN  A5

/*
 * ======================================================================================================================
 *  Pin OP1 State Setup
 * ======================================================================================================================
 */
#define OP1_STATE_NULL       0
#define OP1_STATE_RAW        1
#define OP1_STATE_RAIN       2
#define OP1_STATE_DIST_5M    5
#define OP1_STATE_DIST_10M   10

/*
 * ======================================================================================================================
 *  Pin OP2 State Setup
 * ======================================================================================================================
 */
#define OP2_STATE_NULL       0
#define OP2_STATE_RAW        1
#define OP2_STATE_VOLTAIC    2


#define RAINGAUGE1_IRQ_PIN  A3
#define RAINGAUGE2_IRQ_PIN  OP1_PIN
#define DISTANCE_GAUGE_PIN  OP1_PIN
#define VOLTAIC_VOLTAGE_PIN OP2_PIN
#define DG_BUCKETS          60


// Extern variables
extern volatile bool TurnLedOff;                 // Set true in rain gauge interrupt

extern volatile unsigned int raingauge1_interrupt_count;
extern uint64_t raingauge1_interrupt_stime;
extern volatile uint64_t raingauge1_interrupt_ltime;
extern uint64_t raingauge1_interrupt_toi;

extern volatile unsigned int raingauge2_interrupt_count;
extern uint64_t raingauge2_interrupt_stime;
extern volatile uint64_t raingauge2_interrupt_ltime;
extern uint64_t raingauge2_interrupt_toi;

extern unsigned int dg_resolution_adjust;

// Function prototype
void raingauge1_interrupt_handler();
float raingauge1_sample();
void raingauge2_interrupt_handler();
float raingauge2_sample();
bool RainEnabled();

float Pin_ReadAvg(int pin);

float VoltaicVoltage(int pin);
float VoltaicPercent(float half_cell_voltage);
void DS_TakeReading();
float DS_Median();

void OPT_AQS_Initialize();

void raingauge_obs_do(int &sidx);
void op_obs_do(int &sidx);
