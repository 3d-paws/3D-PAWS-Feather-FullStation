/*
 * ======================================================================================================================
 *  derived.h - I2C OBS Definations
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Heat Index Temperature - Derived from Temperature and Humidity Sensors
 * ======================================================================================================================
 */
extern bool HI_exists;

/*
 * ======================================================================================================================
 *  Wet Bulb Globe Temperature - Derived from Temperature and Humidity Sensonrs
 * ======================================================================================================================
 */
extern bool WBGT_exists;

/*
 * ======================================================================================================================
 *  Wet Bulb Temperature - Derived from Temperature and Humidity Sensors
 * ======================================================================================================================
 */
extern bool WBT_exists;

/* 
 *=======================================================================================================================
 * MSLP - Mean sea level pressure 
 *=======================================================================================================================
 */
extern bool MSLP_exists;

// Function prototype
void wbt_initialize();
double wbt_calculate(double T, double RH);
void hi_initialize();
float hi_calculate(float T, float RH);
void wbgt_initialize();
double wbgt_using_hi(double HIc);
double wbgt_using_wbt(double Ta, double Tg, double Tw);
void mslp_initialize();
double mslp_calculate(float Ts, float RH, float ps, int station_height);
void derived_info(char *rest, int size, const char *&comma);
void derived_obs_do(int &sidx);