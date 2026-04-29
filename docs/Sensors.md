# Sensor Information
[←Top](../README.md)<BR>

Sensors
| Sensor Model                  | Type                    | Description                                                                                              | Company URL                             |
|-------------------------------|-------------------------|----------------------------------------------------------------------------------------------------------|------------------------------------------|
| Adafruit HTU21DF              | Temp/Humidity           | Digital I2C sensor, ±2% RH, ±0.3°C, compact, low power.                                     | [Adafruit HTU21DF](https://www.adafruit.com/product/1899)   |
| Adafruit MCP9808              | Temperature             | High-precision digital temp sensor, ±0.25°C, 2.7-5.5V, I2C.                                 | [Adafruit MCP9808](https://www.adafruit.com/product/1782)   |
| Adafruit BME280               | Temp/Humidity/Pressure  | Measures temp, humidity, pressure, I2C/SPI, -40 to +85°C, 0-100% RH, 1.8-5V.                | [Adafruit BME280](https://www.adafruit.com/product/2652)    |
| Adafruit BMP280               | Temp/Pressure           | Measures pressure (300–1100 hPa), temperature, I2C/SPI, small, low-power.                   | [Adafruit BMP280](https://www.adafruit.com/product/2651)    |
| Adafruit BMP3XX               | Temp/Pressure           | Gen3 barometric, high-precision, ±0.5m alt., I2C/SPI.                                       | [BMP388/BMP3XX](https://shop.pimoroni.com/en-us/products/adafruit-bmp388-precision-barometric-pressure-and-altimeter) |
| Adafruit_BMP5xx               | Temp/Pressure           | BMP5xx series (BMP581) is a high-precision barometric pressure sensors from Bosch Sensortec | [Adafruit BMP5xx](hhttps://www.adafruit.com/product/6407) |
| Adafruit SHT31                | Temp/Humidity           | High accuracy, ±2% RH, ±0.3°C, I2C, fast response, 2.4–5.5V.                                | [Adafruit SHT31](https://www.adafruit.com/product/2857)     |
| Adafruit_SHT4x                | Temp/Humidity           | Adafruit Sensirion SHT45 Precision Temp & Humidity with PTFE                                | [Adafruit SHT45](https://www.adafruit.com/product/5665)     |
| Adafruit SI1145               | Light/UV/Proximity      | I2C, measures visible, IR, and digital UV index, 3-5V.                                      | [Adafruit SI1145](https://www.adafruit.com/product/6174)    |
| Adafruit PM25AQI              | PM2.5 Air Quality       | Measures particulate (0.3–10um), I2C (fixed 0x12), 0-500μg/m³ range, 5V.                    | [PM25AQI](https://www.adafruit.com/product/4632)            |
| Adafruit HDC302x              | Temp/Humidity           | High-accuracy I2C digital sensor (variant of HDC3021, HDC3022).                             | [Adafruit HDC302x](https://www.adafruit.com)                |
| Adafruit LPS35HW              | Pressure                | Robust water-resistant MEMS pressure sensor, I2C/SPI, oil/water resistant.                  | [LPS35HW](https://www.adafruit.com)                        |
| DFRobot_B_LUX_V30B            | Ambient Light           | Measures up to 200klx, wide range, digital I2C, 3.3-5V.                                     | [DFRobot Wiki](https://wiki.dfrobot.com/Ambient_Light_Sensor_0_200klx_SKU_SEN0390) |
| Honeywell HIH8000             | Temp/Humidity           | Reliable, digital I2C, ±3% RH, ±0.5°C, industrial grade.                                    | [Honeywell HIH8000](https://sensing.honeywell.com)          |
| Tinovi LeafSens               | Leaf Wetness/Temp       | Non-contact capacitive leaf wetness + temperature, digital.                                 | [Tinovi LeafSens](https://www.tinovi.com/leafsens)          |
| Tinovi i2cArduino             | Soil Moisture/Temp      | Capacitive soil moisture + temperature, I2C.                                                | [Tinovi i2cArduino](https://www.tinovi.com/i2carduino)      |
| AS5600                        | Magnetic Encoder        | Rotational angle/magnetic position sensor, I2C.                                             | [AS5600 Sensor](https://www.ams.com/as5600)                 |
| AS5600L                       | Magnetic Encoder        | Same as AS5600, but designed for integration in new designs (ex: Particle Muon).            | [AS5600L Sensor](https://www.ams.com/as5600l)               |
| SS451A (Rain Gauge)           | Hall Effect Switch      | Digital magnetic field sensor, rugged, used as rain gauge switch.                           | [SS451A Datasheet](https://sensing.honeywell.com)           |
| SS451A (Anemometer)           | Hall Effect Switch      | Used as wind anemometer sensor (rotation detection).                                        | [SS451A Datasheet](https://sensing.honeywell.com)           |
| TMP112A                       | Temperature             | Precision digital temp sensor, on-board Particle Muon.                                      | [TMP112A Sensor](https://www.ti.com/product/TMP112A)        |
| MB7360, MB7369, MB7380, MB7389| 5m Distance             | Maxbotix Ultrasonic Distance Sensor                                                         | [Maxbotix](https://maxbotix.com/products)        |
| MB7363, MB7366, MB7383, MB7386| 10m Distance            | Maxbotix Ultrasonic Distance Sensor                                                         | [Maxbotix](https://maxbotix.com/products)        |

---
### Tags Names sent to Particle
| Tag Name | Description |
|----------|-------------|
| bcs      | Battery Charger Status            |
| bpc      | Battery Percent Charge            |
| cfr      | Charger Fault Register            |
| rg       | Rain Gauge            |
| rgs      | Rain Gauge Seconds          |
| rgt      | Rain Gauge Total            |
| rgp      | Rain Gauge Total Prior          |
| ws       | Wind Speed            |
| wd       | Wind Direction            |
| wg       | Wind Gust            |
| wgd      | Wind Gust Direction            |
| pm1s10   | PM25AQI Standard PM1.0 (µg/m³)           |
| pm1s25   | PM25AQI Standard PM2.5 (µg/m³)           |
| pm1s100  | PM25AQI Standard PM10.0 (µg/m³)           |
| pm1e10   | PM25AQI Environmental PM1.0 (µg/m³)           |
| pm1e25   | PM25AQI Environmental PM2.5 (µg/m³)           |
| pm1e100  | PM25AQI Environmental PM10.0 (µg/m³)           |
| bp1      | BME280,BMP280,BMP3XX Pressure           |
| bt1      | BME280,BMP280,BMP3XX Temperature           |
| bh1      | BME280,BMP280,BMP3XX Humidity           |
| bp2      | BME280,BMP280,BMP3XX Pressure           |
| bt2      | BME280,BMP280,BMP3XX Temperature           |
| bh2      | BME280,BMP280,BMP3XX Humidity           |
| hh1      | HIH8000 Humidity           |
| ht1      | HIH8000 Temperature           |
| hh2      | HIH8000 Humidity           |
| ht2      | HIH8000 Temperature           |
| st1      | SHT31 Temperature           |
| sh1      | SHT31 Humidity          |
| st2      | SHT31 Temperature           |
| sh2      | SHT31 Humidity           |
| hdt1     | HDC302x Temperature           |
| hdh1     | HDC302x Humidity           |
| hdt2     | HDC302x Temperature           |
| hdh2     | HDC302x Humidity           |
| lpt1     | LPS35HW Temperature           |
| lpp1     | LPS35HW Pressure           |
| lpt2     | LPS35HW Temperature           |
| lpp2     | LPS35HW Pressure           |
| sv1      | SI1145 VIS           |
| si1      | SI1145 IR           |
| su1      | SI1145 UV          |
| mt1      | MCP9808 Temperature           |
| mt2      | MCP9808 Temperature           |
| gt1      | MCP9808 Globe Temperature            |
| gt2      | MCP9808 Globe Temperature            |
| blx      | DFRobot_B_LUX_V30B            |
| sg       | Option 1 Maxbotix Distance Guage snow or stream            |
| op1r     | Option 1 analog pin raw reading           |
| rg2      | Option 1 2nd rain gauge            |
| rgt2     | Option 1 2nd rain total            |
| rgp2     | Option 1 2nd rain total prior            |
| op2r     | Option 2 analog pin raw reading            |
| vbv      | Option 2 Voltaic Battery Voltage |
| vpc      | Option 2 Voltaic Percent Charge |
| hi       | SHT31 Heat Index Temperature            |
| wbt      | MCP9808 & SHT31 Wet Bulb Temperature|
| wbgt     | MCP9808 & SHT31 Wet Bulb Globe Temperature            |
| mslp     | Mean Sea Level Pressure |
| tlww     | Tinovi LeafSens Wetness           |
| tlwt     | Tinovi LeafSens Temp            |
| tsmt     | Tinovi Soil Moisture Temperature |
| tsme25   | Tinovi Soil Moisture e25 |
| tsmec    | Tinovi Soil Moisture ec |
| tsmvwc   | Tinovi Soil Moisture vwc |
| tsmt-[1-8] | MUX Tinovi Soil Moisture Temperature |
| tsme25-[1-8] | MUX Tinovi Soil Moisture e25 |
| tsmec-[1-8] | MUX Tinovi Soil Moisture ec |
| tsmvwc-[1-8] | MUX Tinovi Soil Moisture vwc |


### Wind
#### Collecting Wind Data
- **Wind_SampleSpeed()** – Returns the wind speed based on interrupt counts and the duration between calls.  
- **Wind_SampleDirection()** – Reads wind direction via I²C from the AS5600(L) sensor.  
- **Wind_TakeReading()** – Called every second. It collects wind direction and wind speed, then stores the samples in a circular buffer of 60 buckets.  

#### Creating the 1 Minute Wind Observations
- **Wind_DirectionVector()** – Uses the 60-sample buffer of wind directions and wind speeds (where wind speed > 0) to compute and return a wind vector.  
- **Wind_SpeedAverage()** – Returns the average wind speed from the 60-sample buffer.  
- **Wind_GustUpdate()** – Finds the highest three consecutive wind speed samples in the buffer, averages them to calculate the wind gust, and updates `wind.gust` and `wind.gust_direction`. `Wind_GustUpdate()` must be called before `Wind_Gust()` or `Wind_GustDirection()`.  
- **Wind_Gust()** – Returns `wind.gust`.  
- **Wind_GustDirection()** – Returns `wind.gust_direction`.  

---
### PM25AQI - I2C - Air Quality Sensor
#### Concentration Units (standard)
- pm10_standard: Standard PM1.0 (µg/m³)  
- pm25_standard: Standard PM2.5 (µg/m³)  
- pm100_standard: Standard PM10.0 (µg/m³)
#### Concentration Units (environmental)
- pm10_env: Environmental PM1.0 (µg/m³)  
- pm25_env: Environmental PM2.5 (µg/m³)  
- pm100_env: Environmental PM10.0 (µg/m³)  
#### Particle Variables - Not Reporting
- 03um: ≥ 0.3 µm per 0.1 L of air  
- 05um: ≥ 0.5 µm per 0.1 L of air  
- 10um: ≥ 1.0 µm per 0.1 L of air  
- 25um: ≥ 2.5 µm per 0.1 L of air  
- 50um: ≥ 5.0 µm per 0.1 L of air  
- 100um: ≥ 10.0 µm per 0.1 L of air  

#### Notes
- pms = Particulate Matter Standard  
- pme = Particulate Matter Environmental  

#### Variable Tags
- pm1s10, pm1s25, pm1s100  
- pm1e10, pm1e25, pm1e100  

---
### MaxBotix Distance Sensors
#### 5-Meter Sensors  
(MB7360, MB7369, MB7380, MB7389)  
Scale factor: (Vcc / 5120) per 1 mm  

- Particle (12-bit, 0–4095): Range 0–5119 mm, resolution 1.25 mm per unit  
- Feather (10-bit, 0–1023): Range 0–5119 mm, resolution 5 mm per unit  

#### 10-Meter Sensors  
(MB7363, MB7366, MB7383, MB7386)  
Scale factor: (Vcc / 10240) per 1 mm  

- Particle (12-bit, 0–4095): Range 0–10239 mm, resolution 2.5 mm per unit  
- Feather (10-bit, 0–1023): Range 0–10239 mm, resolution 10 mm per unit  

### I2C Mux and Multiple Tinovi Soil Moisture/Temp Sensors
- Tested with: [Adafruit PCA9548 8-Channel STEMMA QT / Qwiic I2C Multiplexer - TCA9548A](https://www.adafruit.com/product/5626) \
This I2C Multiplexer is set to I2C address 0x70. It is TCA9548A compatible.
- This [Grove - 8 Channel I2C Multiplexer/I2C Hub (TCA9548A)](https://www.seeedstudio.com/Grove-8-Channel-I2C-Hub-TCA9548A-p-4398.html) should also work.
<pre>
Below is output from the serial console at boot.
MUX:INIT
MUX OK
MUX:SCAN
  CH-0 TSM NF
  CH-1 TSM NF
  CH-2 TSM NF
  CH-3 TSM NF
  CH-4 TSM NF
  CH-5 TSM NF
  CH-6 TSM NF
  CH-7.0 TSM OK

The .0 on the "CH-7.0" above is representing the first occurance of a discovered sensor on this channel.
In the future we may be supporting other sensors on the mux. 

 MUX sensors are reported bith the below tag names. 
 [1-8] is the MUX channel the sensor was found on.
   tsmt-[1-8]    Tinovi Soil Moisture temperature
   tsme25-[1-8]  Tinovi Soil Moisture e25
   tsmec-[1-8]   Tinovi Soil Moisture ec
   tsmvwc-[1-8]  Tinovi Soil Moisture vwc
</pre>


### Derived Sensor Observations
#### Heat Index Temperature
- Heat Index (hi) is reported if sensor SHT_1 exists.
- heat_index = hi_calculate(sht1_temp, sht1_humid);
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 600px; border: 1px solid black; padding: 10px;">

```C
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
```
 </div>

#### Wet Bulb Temperature
- Wet Bulb Temperature (wbt) is reported if sensors - MCP_1 and SHT_1 exists.
- wetbulb_temp = wbt_calculate(sht1_temp, sht1_humid);

<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 500px; border: 1px solid black; padding: 10px;">

```C
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
```
</div>


#### Wet Bulb Globe Temperature
- Wet Bulb Globe Temperature (wbgt) is reported if sensor SHT_1_exists.
- Two Methods of computing based on MCP_3 sensor existing Black Globle sensor.
- if MCP_3 use wbgt = wbgt_using_wbt(sht1_temp, mcp3_temp, wetbulb_temp);
- Otherwise wbgt = wbgt_using_hi(heat_index); - requires SHT_1_exists 
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 500px; border: 1px solid black; padding: 10px;">

```C
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
```
```C
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
```
</div>

#### Mean Sea Level Pressure
- Reported as "mslp" if station elevation is set "ELEV.TXT" and station is configured with a SHT and BMP sensors. 

<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 500px; border: 1px solid black; padding: 10px;">

```C
/* 
 *=======================================================================================================================
 * mslp_caculate() - mean sea level pressure caculate
 *  Inputs: 
 *    Surface air temperature Ts (deg C)  sht1_temp       Ts
 *    Relative humidity RH (%%)           sht1_humid      RH
 *    Station pressure ps (hPa)           bmx_1_pressure  ps
 *    Station height (m)                  cf_elevation    station_height
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
```
</div>

### Reading Voltaic Battery Voltage Breakout on the UCB-C
Newer Voltaic Battery V25/V50/V75 firmware (post-2023) moved this cell voltage signal to SBU pins (A8/B8) on USB-C for better USB compliance. Older units used D+. Which conflicted with data transfer.  The expected voltage range on Voltaic V25's monitor pins (D+ on older firmware, SBU1 (A8) and SBU2 (B8) on newer) is 1.6V to 2.1V

In the configuration file CONFIG.TXT, configure "op2=2".

This will report as "vbv" (Voltaic Battery Voltage)

[Adafruit USB Type C Breakout Board - Downstream Connection](https://www.adafruit.com/product/4090)
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 200px; border: 1px solid black; padding: 10px;">

```C
/* 
 *=======================================================================================================================
 * VoltaicVoltage() - Breakout the UCB-C 
 *=======================================================================================================================
 */
float VoltaicVoltage(int pin) {
  int numReadings = 5;
  int totalValue = 0;
  for (int i = 0; i < numReadings; i++) {
    totalValue += analogRead(pin);
    delay(10);  // Short delay between readings
  }
  float voltage = (3.3 * (totalValue / numReadings)) / 4095.0; 
  return(voltage);
}
```
</div>

