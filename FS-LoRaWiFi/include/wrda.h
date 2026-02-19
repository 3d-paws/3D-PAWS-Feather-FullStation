/*
 * ======================================================================================================================
 *  wrda.h - Wind Rain Distance Air Definations
 * ======================================================================================================================
 */

 /*
 * ======================================================================================================================
 *  Wind Related Setup
 * 
 *  NOTE: With interrupts tied to the anemometer rotation we are essentually sampling all the time.  
 *        We record the interrupt count, ms duration and wind direction every second.
 *        One revolution of the anemometer results in 2 interrupts. There are 2 magnets on the anemometer.
 * 
 *        Station observations are logged every minute
 *        Wind and Direction are sampled every second producing 60 samples 
 *        The one second wind speed sample are produced from the interrupt count and ms duration.
 *        Wind Observations a 
 *        Reported Observations
 *          Wind Speed = Average of the 60 samples.
 *          Wind Direction = Average of the 60 vectors from Direction and Speed.
 *          Wind Gust = Highest 3 consecutive samples from the 60 samples. The 3 samples are then averaged.
 *          Wind Gust Direction = Average of the 3 Vectors from the Wind Gust samples.
 * 
 * Distance Sensors
 * The 5-meter sensors (MB7360, MB7369, MB7380, and MB7389) use a scale factor of (Vcc/5120) per 1-mm.
 * Particle 12bit resolution (0-4095),  Sensor has a resolution of 0 - 5119mm,  Each unit of the 0-4095 resolution is 1.25mm
 * Feather has 10bit resolution (0-1023), Sensor has a resolution of 0 - 5119mm, Each unit of the 0-1023 resolution is 5mm
 * 
 * The 10-meter sensors (MB7363, MB7366, MB7383, and MB7386) use a scale factor of (Vcc/10240) per 1-mm.
 * Particle 12bit resolution (0-4095), Sensor has a resolution of 0 - 10239mm, Each unit of the 0-4095 resolution is 2.5mm
 * Feather has 10bit resolution (0-1023), Sensor has a resolution of 0 - 10239mm, Each unit of the 0-1023 resolution is 10mm
 * ======================================================================================================================
 */
#define ANEMOMETER_IRQ_PIN  A2
#define WIND_READINGS       60       // One minute of 1s Samples

typedef struct {
  int direction;
  float speed;
} WIND_BUCKETS_STR;

typedef struct {
  WIND_BUCKETS_STR bucket[WIND_READINGS];
  int bucket_idx;
  float gust;
  int gust_direction;
  int sample_count;
} WIND_STR;

/*
 * ======================================================================================================================
 *  Option Pin Defination Setup
 * ======================================================================================================================
 */
#define OP1_PIN  A4
#define OP2_PIN  A5

#define RAINGAUGE1_IRQ_PIN  A3
#define RAINGAUGE2_IRQ_PIN  OP1_PIN
#define DISTANCE_GAUGE_PIN  OP1_PIN
#define VOLTAIC_VOLTAGE_PIN OP2_PIN
#define DG_BUCKETS          60


// Extern variables
extern volatile unsigned int anemometer_interrupt_count;
extern unsigned long anemometer_interrupt_stime;

extern bool AS5600_exists;
extern int AS5600_ADR;

extern volatile unsigned int raingauge1_interrupt_count;
extern uint64_t raingauge1_interrupt_stime;
extern uint64_t raingauge1_interrupt_ltime;
extern uint64_t raingauge1_interrupt_toi;

extern volatile unsigned int raingauge2_interrupt_count;
extern uint64_t raingauge2_interrupt_stime;
extern uint64_t raingauge2_interrupt_ltime;
extern uint64_t raingauge2_interrupt_toi;

extern bool ws_refresh;

extern unsigned int dg_resolution_adjust;

// Function prototype
void anemometer_interrupt_handler();
void raingauge1_interrupt_handler();
void raingauge2_interrupt_handler();

float Wind_SampleSpeed();
int Wind_SampleDirection();
int Wind_DirectionVector();
float Wind_SpeedAverage();
float Wind_Gust();
int Wind_GustDirection();
void Wind_GustUpdate();
void Wind_ClearSampleCount();
void Wind_TakeReading();
void as5600_initialize();
float Pin_ReadAvg(int pin);
float VoltaicVoltage(int pin);
void DS_TakeReading();
float DS_Median();

void Wind_Distance_Air_Initialize();

void OPT_AQS_Initialize();
