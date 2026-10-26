/*
 * ======================================================================================================================
 *  derived.cpp - I2C based derived obs
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/output.h"
#include "include/support.h"
#include "include/nvcf.h"
#include "include/sensor_bmp.h"
#include "include/sensor_sht.h"
#include "include/sensor_mcp.h"
#include "include/obs.h"
#include "include/derived.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
float wetbulb_temp = 0.0;
float heat_index = 0.0;

/*
 * ======================================================================================================================
 *  Wet Bulb Temperature - Derived from Temperature and Humidity Sensors
 * ======================================================================================================================
 */
bool WBT_exists = false;

/*
 * ======================================================================================================================
 *  Heat Index Temperature - Derived from Temperature and Humidity Sensors
 * ======================================================================================================================
 */
bool HI_exists = false;

/*
 * ======================================================================================================================
 *  Wet Bulb Globe Temperature - Derived from Temperature and Humidity Sensonrs
 * ======================================================================================================================
 */
bool WBGT_exists = false;

/* 
 *=======================================================================================================================
 * MSLP - Mean sea level pressure 
 *=======================================================================================================================
 */
bool MSLP_exists = false;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */ 


/* 
 *=======================================================================================================================
 * wbt_initialize() - Wet Bulb Temperature
 *=======================================================================================================================
 */
void wbt_initialize() {
  Output("WBT:INIT");
  if (MCP_1_exists && SHT_1_exists) {
    WBT_exists = true;
    Output ("WBT:OK");
  }
  else {
    Output ("WBT:NF");
  }
}

/* 
 *=======================================================================================================================
 * wbt_calculate() - Compute Web Bulb Temperature
 * 
 * By definition, wet-bulb temperature is the lowest temperature a portion of air can acquire by evaporative 
 * cooling only. When air is at its maximum (100 %) humidity, the wet-bulb temperature is equal to the normal 
 * air temperature (dry-bulb temperature). As the humidity decreases, the wet-bulb temperature becomes lower 
 * than the normal air temperature. Forecasters use wet-bulb temperature to predict rain, snow, or freezing rain.
 * 
 * SEE https://journals.ametsoc.org/view/journals/apme/50/11/jamc-d-11-0143.1.xml
 * SEE https://www.omnicalculator.com/physics/wet-bulb
 * 
 * Tw = T * atan[0.151977(RH + 8.3,3659)^1/2] + atan(T + RH%) - atan(RH - 1.676311)  + 0.00391838(RH)^3/2 * atan(0.023101 * RH%) - 4.686035
 * 
 * [ ] square bracket denote grouping for order of operations. 
 *     In Arduino code, square brackets are not used for mathematical operations. Instead, parentheses ( ).
 * sqrt(x) computes the square root of x, which is x to the 1/2.
 * pow(RH, 1.5) calculates RH to the 3/2, which is the relative humidity raised to the power of 1.5.
 *=======================================================================================================================
 */
double wbt_calculate(double T, double RH) {
  if ((T == -999.9) || (RH == -999.9)) {
    return (-999.9);
  }

  // Output("WBT:CALC");

  // Equation components
  double term1 = T * atan(0.151977 * sqrt(RH + 8.313659));
  double term2 = atan(T + RH);
  double term3 = atan(RH - 1.676311);
  double term4 = 0.00391838 * pow(RH, 1.5) * atan(0.023101 * RH);
  double constant = 4.686035;

  // Wet bulb temperature calculation
  double Tw = term1 + term2 - term3 + term4 - constant;

  Tw = (isnan(Tw) || (Tw < QC_MIN_T)  || (Tw >QC_MAX_T))  ? QC_ERR_T  : Tw;
  return (Tw);
}

/* 
 *=======================================================================================================================
 * hi_initialize() - Heat Index Temperature
 *=======================================================================================================================
 */
void hi_initialize() {
  Output("HI:INIT");
  if (SHT_1_exists) {
    HI_exists = true;
    Output ("HI:OK");
  }
  else {
    Output ("HI:NF");
  }
}

/* 
 *=======================================================================================================================
 * hi_calculate() - Compute Heat Index Temperature Returns Celsius
 * 
 * SEE https://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
 * 
 * The regression equation of Rothfusz is:
 * HI = -42.379 + 2.04901523*T + 10.14333127*RH - .22475541*T*RH - .00683783*T*T - .05481717*RH*RH + .00122874*T*T*RH + 
 *      .00085282*T*RH*RH - .00000199*T*T*RH*RH
 * 
 * The Rothfusz regression is not appropriate when conditions of temperature and humidity 
 * warrant a heat index value below about 80 degrees F. In those cases, a simpler formula 
 * is applied to calculate values consistent with Steadman's results:
 * HI = 0.5 * {T + 61.0 + [(T-68.0)*1.2] + (RH*0.094)} 
 *=======================================================================================================================
 */
float hi_calculate(float T, float RH) {
  float HI;
  float HI_f;

  if ((T == -999.9) || (RH == -999.9)) {
    return (-999.9);
  }

  // Convert temperature from Celsius to Fahrenheit
  float T_f = T * 9.0 / 5.0 + 32.0;

  // Steadman's equation
  HI_f = 0.5 * (T_f + 61.0 + ((T_f - 68.0)*1.2) + (RH * 0.094));

  // Compute the average of the simple HI with the actual temperature [deg F]
  HI_f = (HI_f + T_f) / 2;

  if (HI_f >= 80.0) { 
    // Use Rothfusz's equation
    
    // Constants for the Heat Index formula
    float c1 = -42.379;
    float c2 = 2.04901523;
    float c3 = 10.14333127;
    float c4 = -0.22475541;
    float c5 = -0.00683783;
    float c6 = -0.05481717;
    float c7 = 0.00122874;
    float c8 = 0.00085282;
    float c9 = -0.00000199;
    
    // Heat Index calculation
    HI_f = c1 + (c2 * T_f) + (c3 * RH) + (c4 * T_f * RH) +
                (c5 * T_f * T_f) + (c6 * RH * RH) + 
                (c7 * T_f * T_f * RH) + (c8 * T_f * RH * RH) +
                (c9 * T_f * T_f * RH * RH);

    if ((RH < 13.0) && ((T_f > 80.0) && (T_f < 112.0)) ) {
      // If the RH is less than 13% and the temperature is between 80 and 112 degrees F, 
      // then the following adjustment is subtracted from HI: 
      // ADJUSTMENT = [(13-RH)/4]*SQRT{[17-ABS(T-95.)]/17}

      float Adjustment = ( (13 - RH) / 4 ) * sqrt( (17 - abs(T_f - 95.0) ) / 17 );

      HI_f = HI_f - Adjustment;

    }
    else if ((RH > 85.0) && ((T_f > 80.0) && (T_f < 87.0)) ) {
      // If the RH is greater than 85% and the temperature is between 80 and 87 degrees F, 
      // then the following adjustment is added to HI: 
      // ADJUSTMENT = [(RH-85)/10] * [(87-T)/5]

      float Adjustment = ( (RH - 85) / 10 ) * ( (87.0 - T_f) / 5 );

      HI_f = HI_f + Adjustment;
    }
  }

  // Convert Heat Index from Fahrenheit to Celsius
  HI = (HI_f - 32.0) * 5.0 / 9.0;

  // Quality Control Check
  HI = (isnan(HI) || (HI < QC_MIN_HI)  || (HI >QC_MAX_HI))  ? QC_ERR_HI  : HI;

  return (HI);
}

/* 
 *=======================================================================================================================
 * wbgt_initialize() - Wet Bulb Globe Temperature
 *=======================================================================================================================
 */
void wbgt_initialize() {
  Output("WBGT:INIT");
  if (SHT_1_exists) {
    WBGT_exists = true;
    if (MCP_3_exists) {
      Output ("WBGT:OK W/G");
    }
    else {
      Output ("WBGT:OK WO/G");
    }
  }
  else {
    Output ("WBGT:NF");
  }
}

/* 
 *=======================================================================================================================
 * wbgt_using_hi() - Compute Web Bulb Globe Temperature using Heat Index
 *=======================================================================================================================
 */
double wbgt_using_hi(double HIc) {

  if (HIc == -999.9) {
    return (-999.9);
  }

  double HIf = HIc * 9.0 / 5.0 + 32.0;

  // Below produces Wet Bulb Globe Temperature in Celsius
  double TWc = -0.0034 * pow(HIf, 2) + 0.96 * HIf - 34;

  TWc = (isnan(TWc) || (TWc < QC_MIN_T)  || (TWc >QC_MAX_T))  ? QC_ERR_T  : TWc;
  return (TWc);
}

/* 
 *=======================================================================================================================
 * wbgt_using_wbt() - Compute Web Bulb Globe Temperature using web bulb temperature
 *=======================================================================================================================
 */
double wbgt_using_wbt(double Ta, double Tg, double Tw) {
  // Ta = mcp1 temp
  // Tg = mcp3 temp
  // Tw = wbt_calculate(Ta, RH)

  double wbgt = (0.7 * Tw) + (0.2 * Tg) + (0.1 * Ta);  // This will be Celsius

  wbgt = (isnan(wbgt) || (wbgt < QC_MIN_T)  || (wbgt >QC_MAX_T))  ? QC_ERR_T  : wbgt;

  return (wbgt);
}

/* 
 *=======================================================================================================================
 * mslp_initialize() - mean sea level pressure init MSLP_exists if all the input exist.
 *  Dependants: 
 *    Surface air temperature Ts (deg C)  sht1_temp       Ts
 *    Relative humidity RH (%%)           sht1_humid      RH
 *    Station pressure ps (hPa)           bmp_1_pressure  ps
 *    Station height (m)                  elevation       station_height
 * 
 * Testing Information
 *   https://www.airnav.com/airport/KEIK - Provides Elevation
 *   https://www.weather.gov/wrh/timeseries?site=KEIK - Provides current MSLP
 *=======================================================================================================================
 */
void mslp_initialize() {
  Output("MSLP:INIT");
  if ((scv.elevation != QC_ERR_ELEV) &&  BMP_1_exists && SHT_1_exists) {
    MSLP_exists = true;
    Output ("MSLP:OK");
  }
  else {
    Output ("MSLP:NF");
  }
}

/* 
 *=======================================================================================================================
 * mslp_caculate() - mean sea level pressure caculate
 *  Inputs: 
 *    Surface air temperature Ts (deg C)  sht1_temp       Ts
 *    Relative humidity RH (%%)           sht1_humid      RH
 *    Station pressure ps (hPa)           bmp_1_pressure  ps
 *    Station height (m)                  elevation       station_height
 *=======================================================================================================================
 */
double mslp_calculate(float Ts, float RH, float ps, int station_height) {
  double e, r, q;
  double L = 0.0065;   // Lapse rate (K/m)
  double T, Tv, P0;
  double Td;

  if ((Ts == QC_ERR_T) || (RH == QC_ERR_RH) || (ps == QC_ERR_P) || (station_height == QC_ERR_ELEV) ) {
    return (QC_ERR_P);
  }

  // Calculate dew point Td (deg C) from Ts and RH
  Td = Ts - ((100.0 - RH) / 5.0);

  // Step 1: Vapor pressure (hPa)
  e = 6.112 * exp((17.67 * Td) / (Td + 243.5));

  // Step 2: Mixing ratio r (kg/kg)
  r = 0.622 * e / (ps - e);

  // Step 3: Specific humidity q (kg/kg)
  q = r / (1.0 + r);

  // Mean temperature in Kelvin
  T = (Ts + 273.15) + (L * station_height / 2.0);

  // Virtual temperature (K)
  Tv = T * (1.0 + 0.61 * q);

  // Calculate mean sea level pressure (hPa) using the exponential formula
  P0 = ps * exp((9.80665 * station_height) / (287.05 * Tv));

  return (P0);
}

/* 
 *=======================================================================================================================
 * derived_info() - add to msg derived obs, Passing msg ptr, msg bubber sz, and pointer to comma
 *=======================================================================================================================
 */
void derived_info(char *rest, int size, const char *&comma) {
  char buf[64];
  const char *c = "";
  int used = strlen(rest);

  buf[0] = '\0';

  sprintf (buf, "\"derived\":\"");

  if (HI_exists) {
    sprintf (buf+strlen(buf), "%sHI", c);
    c=",";
  }
  if (WBT_exists) {
    sprintf (buf+strlen(buf), "%sWBT", c);
    c=",";
  }
  if (WBGT_exists) {
    if (MCP_3_exists) {
      sprintf (buf+strlen(buf), "%sWBGT W/G", c);
    }
    else {
      sprintf (buf+strlen(buf), "%sWBGT WO/G", c);
    }
    c=",";
  }
  if (MSLP_exists) {
    sprintf (buf+strlen(buf), "%sMSLP", c);
    c=",";
  }
  sprintf (buf+strlen(buf), "\"");

  // can we add with out over flowing?
  if ((used + strlen(buf)) < size) {
    sprintf(rest + used, "%s%s", comma, buf);
    comma = ",";
  }
  else {
    return;
  }
}

/* 
 *=======================================================================================================================
 * derived_obs_do() - Derived Sensors Observations
 *=======================================================================================================================
 */
void derived_obs_do(int &sidx) {
  // Heat Index Temperature
  if (HI_exists) {
    heat_index = hi_calculate(sht1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "hi");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) heat_index;
    obs.sensor[sidx++].inuse = true;    
  }
  
  // Wet Bulb Temperature
  if (WBT_exists) {
    strcpy (obs.sensor[sidx].id, "wbt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbt_calculate(sht1_temp, sht1_humid);
    obs.sensor[sidx++].inuse = true;    
  }

  // Wet Bulb Globe Temperature
  if (WBGT_exists) {
    float wbgt = 0.0;
    if (MCP_3_exists) {
      wbgt = wbgt_using_wbt(sht1_temp, mcp3_temp, wetbulb_temp); // TempAir, TempGlobe, TempWetBulb
    }
    else {
      wbgt = wbgt_using_hi(heat_index);
    }
    
    strcpy (obs.sensor[sidx].id, "wbgt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbgt;
    obs.sensor[sidx++].inuse = true;    
  }

  if (MSLP_exists) {
    float mslp = (float) mslp_calculate(sht1_temp, sht1_humid, bmp_1_pressure, scv.elevation);
    strcpy (obs.sensor[sidx].id, "mslp");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) mslp;
    obs.sensor[sidx++].inuse = true;  
  }
}

