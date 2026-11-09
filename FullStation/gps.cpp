/*
 * ======================================================================================================================
 *  gps.cpp - GPS Functions
 * ======================================================================================================================
 */
#include "include/output.h"
#include "include/time.h"
#include "include/support.h"
#include "include/main.h"
#include "include/obs.h"
#include "include/gps.h"


/*
Adafruit Mini GPS PA1010D - UART and I2C - STEMMA QT
  https://www.adafruit.com/product/4415

https://www.quora.com/How-long-does-it-take-to-get-a-GPS-fix
The GPS needs to know where are satellites are.

For a cold start, where it has zero information, it needs to start by locating all satellites and getting a rough 
estimate of the satellite tracks. If it finds one satellite it can get this rough information - called almanac - 
from that satellite. But this information is repeatedly sent with a period of 12.5 minutes. So a very long wait. 
But newer GPS can normally listen to 12 or more satellites concurrently, so a more common time for a cold start 
may be 2–4 minutes.

For a warm start, the GPS receiver has a rough idea about the current time, position (within 100 km) and speed 
and have a valid almanac (not older than 180 days). Then it can focus on getting the more detailed orbital 
information for the satellites. This is called ephemeris. This information is valid for 4 hours and each 
satellite repeats it every 30 seconds. An older GPS may need about 45 seconds for this, but many newer GPS 
can cut down this by listening to many satellites concurrently and hopefully quickly get the relevant data 
for enough satellites to get a fix.

If the GPS has valid ephemeris data (i.e. max 4 hours old) and haven’t moved too far or changed speed too 
much, it can get a fix within seconds. The really old GPS receivers needed over 20 seconds but new GPS 
normally might need about 2 seconds.

The very slow speed waiting for the relevant almanac and ephemeris data can be solved by using Assisted 
GPS (A-GPS) where the receiver has access to side channel communication. A mobile phone may use Internet 
to retrieve the almanac very quickly and may even use the seen cellular towers or WiFi networks to get a 
rough location estimate, making it possible to use this rough location to retrieve th ephemeris data over 
the side channel. So with A-GPS it’s possible to even do warm or cold starts within a few seconds. But 
without side channel communication, even the best of the best receivers often needs 40 seconds or more 
if they have no recent information cached in the GPS module - they just have to wait for the relevant 
data to be transmitted by any of the located and tracked satellites.

The PA1010D, “Backup mode” is a low-power state entered by sending the GPS sleep command, typically 
$PMTK225,4∗2F, which puts the module into backup mode. In this mode, the module keeps only minimal 
state so it can resume faster than from a full cold start, and the datasheet notes the board has a 
dedicated WAKE pin for low-power/standby operation.

If using the Adafruit library over UART or I2C, the practical approach is to send that sleep command 
to the module; some users note that waking back up may be easier over UART than over I2C depending on 
the library setup.

The PA1010D’s highest normal operating current is about 36 mA at 3.3 V during acquisition; in regular 
tracking it’s about 28 mA, and Adafruit’s product page rounds it to about 30 mA during navigation.

For standby mode, send this command over the GPS serial interface
gpsSerial.println("$PMTK161,0*28");
Adafruit’s datasheet says standby mode stops navigation, reduces current, and can be exited by 
sending any byte to the serial port
gpsSerial.write('X');
If you want the module to wake by command rather than “any byte,” the documented way to return
from low-power to full power is 
gpsSerial.println("$PMTK225,0*2B");

The PA1010D GPS module typically draws less than 1.0 mA in standby mode, though specific measurements 
and datasheet specifications vary depending on the implementation and hardware configuration

The receiver stops searching for and tracking satellites. However, it retains internal data (like Ephemeris 
and last known position) to allow for a Hot Start once it is woken up, typically achieving a fix in about 1 second.

myI2CGPS.begin() is just initializing the I2C connection, that typically won’t be a wake command by itself; 
it’s more of a bus/module setup call than an explicit “resume GPS” action.

For backup mode, send
gpsSerial.println("$PMTK225,4*2F");
Datasheet says the WAKE-UP pin must be tied to ground before entering backup mode, and backup mode 
cannot be exited by software; it requires hardware wake behavior instead

In backup mode, the PA1010D is down in the tens of microamps. The datasheet says about 18 µA typical at 3 V

This is a deeper sleep state where the I/O block is also powered off. It similarly stops active tracking 
but uses an internal Real-Time Clock (RTC) to estimate satellite positions while it is down. This enables 
a faster "warm start" when the module is fully powered back on

HDOP Meaning
gps.hdop.hdop() returns the Horizontal Dilution of Precision value from the GPS receiver—a unitless number
(typically 0.5-20) that measures satellite geometry quality for horizontal position accuracy (lat/lon).

  Low HDOP (<1.5): Excellent—satellites well-spread across sky, high confidence in position (±1-3m typical).
  Good (1.5-2.5): Reliable for most uses (±3-5m).
  Poor (>3.0): Clustered satellites or obstructions, avoid trusting position (±10m+ error possible).
  >6: Unusable for precision work.

  Running feather with GPS off =  0.0364 A
  Running feather with GPS on  = ~0.0745 A

The RST pin on the Adafruit Mini GPS PA1010D is an active-low hardware reset input: pulling it to ground puts 
the GPS chip into reset; releasing it allows the receiver to start running again.
*/

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
I2CGPS myI2CGPS; // Hook object to the library
TinyGPSPlus gps; // Declare gps object

bool gps_exists=false;
bool gps_valid=false;
bool gps_on=false;


char gps_timestamp[32];

double gps_lat=0.0;
double gps_lon=0.0;
double gps_altm=0.0;
double gps_altf=0.0;
int    gps_sat=0;
float  gps_hdop=9999.9;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * gps_reset() - Toggle GPS reset Pin
 * =======================================================================================================================
 */
void gps_reset() {
  Output ("GPS:RESET");
  digitalWrite(GPS_RST_PIN, LOW);   // assert reset
  delay(20);                        // hold for 20ms
  digitalWrite(GPS_RST_PIN, HIGH);  // release reset
  delay(500);
}

/*
 * ======================================================================================================================
 * gps_wake() - 36 mA at 3.3 V during acquisition; in regular tracking it’s about 28 mA
 * 
 * Caveat: Once the PA1010D is in standby or backup, I2C failes to respond to command and wake pin. Reset needed.
 * 
 * Pulling the RST pin low performs a full hardware reset on the PA1010D, clearing firmware state, 
 * PMTK settings (returns to full power 1Hz mode), and I2C buffer. 
 * 
 * Hardware reset is used because it reliably restores I2C communication on this hardware. Retained navigation state and 
 * configuration behavior depend on the receiver and backup-power conditions.
 * =======================================================================================================================
 */
bool gps_wake() {
  if (gps_exists) {
    // We don't care about state of gps_on, just do the reset and make sure it talks to us
    gps_reset();
    if (I2C_Device_Exist(GPS_ADDRESS)) {
      gps_on = true;
      Output ("GPS:ON");
      if (myI2CGPS.begin()) {    
        Output("GPS:WAKE OK");
        return(true); 
      }
      else {
        // We reset the gps but it did not respond to begin(), so we can not assume it is on
        Output("GPS:WAKE ERR");
      }
    }
    else {
      gps_on = false;
      Output ("GPS:OFF");
    }
  }
  return(false);
}

/*
 * ======================================================================================================================
 * gps_sleep() - GPS_STANDBY_MODE or GPS_BACKUP_MODE
 * 
 * Caveat: I have seen the gps heartbeat led stuck on when put to sleep. Okay to ignore.
 * =======================================================================================================================
 */
void gps_sleep(int mode) {
  if (mode == GPS_STANDBY_MODE) {
    // Put in standby mode - 1.0 mA
    myI2CGPS.sendMTKpacket("$PMTK161,0*28\r\n");
    Output ("GPS MODE:STANDBY");
  }
  else if (mode == GPS_BACKUP_MODE) {
    // Put in backup mode - 18 µA
    myI2CGPS.sendMTKpacket("$PMTK225,4*2F\r\n");
    Output ("GPS MODE:BACKUP");
  }
  else {
    // Put in periodic mode
    /*
     * PA1010D $PMTK225,2,4000,15000,24000,90000*16 Command Parameters
     * -----------------------------------------------------------
     * Parameter     | Value (ms) | Meaning
     * --------------|------------|---------
     * Type          | 2          | Periodic Standby Mode (stops acquisition during sleep)
     * Run_time      | 4000       | 4s full navigation/tracking (~28mA)
     * Sleep_time    | 15000      | 15s standby (~200µA)
     * 2nd_Run_time  | 24000      | 24s extended nav (if no fix in first cycle)
     * 2nd_Sleep_time| 90000      | 90s extended sleep
     */
    myI2CGPS.sendMTKpacket("$PMTK225,2,4000,15000,24000,90000*16\r\n");
    Output ("GPS MODE:PERIODIC");
  }
  gps_on = false;
  Output ("GPS:OFF");
  delay(20);
}

/* 
 *=======================================================================================================================
 * gps_displayInfo() - 
 *=======================================================================================================================
 */
void gps_displayInfo()
{
  if (gps_valid) {
    Output ("GPN INFO:");
    sprintf(msgbuf, " DATE:%d-%02d-%02d",
      gps.date.year(),
      gps.date.month(),
      gps.date.day());
    Output(msgbuf);
  
    sprintf(msgbuf, " TIME:%02d:%02d:%02d",
      gps.time.hour(),
      gps.time.minute(),
      gps.time.second());
    Output(msgbuf);

    sprintf(msgbuf, " LAT:%f", gps_lat);
    Output(msgbuf);
    sprintf(msgbuf, " LON:%f", gps_lon);
    Output(msgbuf);
    sprintf(msgbuf, " ALT:%fm",  gps_altm);
    Output(msgbuf);
    sprintf(msgbuf, " ALT:%fft", gps_altf);
    Output(msgbuf);
    sprintf(msgbuf, " SAT:%d", gps_sat);
    Output(msgbuf);
    sprintf(msgbuf, " HDOP:%f", gps_hdop);
    Output(msgbuf);
    sprintf(msgbuf, " ON:%d", (gps_on)?1:0);
    Output(msgbuf);
  }
  else{
    Output("GPS:!VALID");
  }
}

/* 
 *=======================================================================================================================
 * gps_keepoff() - Have seen the gps on when it should not be, check and shut off if i2c 0x10 responds
 *                 When it is truely off it will not respond to the I2C_Device_Exist() call.
 *=======================================================================================================================
 */
void gps_keepoff() {
  if (gps_exists && !gps_on && I2C_Device_Exist(GPS_ADDRESS)) {
    Output("GPS ON: KEEPOFF()");
    if (gps_wake()) {
       gps_aquire();  // This will validate the gps and rtc then put the gps to sleep
    }
    else {
      // if we failed to begin / initialize the gps !!! Do we set gps_exists=false
    }
  }
}

/* 
 *=======================================================================================================================
 * gps_aquire() - return true if aquired
 * 
 * If we do not aquire gps time, we leave the gps powered on.
 * If we stop calling gps.encode(myI2CGPS.read()); then gps data becomes stale.
 *=======================================================================================================================
 */
bool gps_aquire() {
  // Do not trust GPS data from a previous acquisition.
  gps_valid = false;

  if (!gps_exists) {
    return false;
  }
  Output ("GPS_AQUIRE");

  // Assume the GPS is on and gps_wake() was called before this function.

  const unsigned long timeoutMs = 30000UL;
  const unsigned long feedbackMs = 1000UL;
  const unsigned long freshnessMs = 2000UL;

  // Clear date/time update flags from a previous acquisition.
  // Reading value() clears the update flag without erasing the value.
  (void)gps.date.value();
  (void)gps.time.value();

  const unsigned long acquisitionStart = millis();
  unsigned long lastFeedback = acquisitionStart;

  while (millis() - acquisitionStart < timeoutMs) {
    const unsigned long currentMillis = millis();

    if (SerialConsoleEnabled &&
        (currentMillis - lastFeedback >= feedbackMs)) {
      Serial.print(".");
      lastFeedback = currentMillis;
    }

    if (!myI2CGPS.available()) {
      continue;
    }

    // Evaluate only after a complete, checksum-valid sentence.
    if (!gps.encode(myI2CGPS.read())) {
      continue;
    }

    // Capture these flags before reading date/time values.
    const bool dateUpdated = gps.date.isUpdated();
    const bool timeUpdated = gps.time.isUpdated();

    // Clear the flags after each completed sentence so updates
    // cannot accumulate across separate sentences.
    (void)gps.date.value();
    (void)gps.time.value();

    // Require a sentence that updated both date and time.
    if (!dateUpdated || !timeUpdated) {
      continue;
    }

    // All fields used to qualify the fix must be valid.
    if (!gps.date.isValid() ||
        !gps.time.isValid() ||
        !gps.location.isValid() ||
        !gps.altitude.isValid() ||
        !gps.satellites.isValid() ||
        !gps.hdop.isValid()) {
      continue;
    }

    // All fields used to qualify the fix must be recent.
    if (gps.date.age() >= freshnessMs ||
        gps.time.age() >= freshnessMs ||
        gps.location.age() >= freshnessMs ||
        gps.altitude.age() >= freshnessMs ||
        gps.satellites.age() >= freshnessMs ||
        gps.hdop.age() >= freshnessMs) {
      continue;
    }

    // Date/time sanity checks and position-quality requirements.
    if ((gps.date.year() >= TM_VALID_YEAR_START) &&
        (gps.date.year() <= TM_VALID_YEAR_END) &&
        (gps.date.month() >= 1) &&
        (gps.date.month() <= 12) &&
        (gps.date.day() >= 1) &&
        (gps.date.day() <= 31) &&
        (gps.time.hour() <= 23) &&
        (gps.time.minute() <= 59) &&
        (gps.time.second() <= 59) &&
        (gps.satellites.value() >= 4) &&
        (gps.hdop.hdop() < 2.5)) {

      gps_valid = true;
      break;
    }
  }

  // Clean up after the progress dots.
  if (SerialConsoleEnabled) {
    Serial.println();
  }

  if (!gps_valid) {
    Output("GPS:NOT AQUIRED");

    // Preserve existing behavior: leave the GPS running on failure.
    return false;
  }

  // Update RTC_Clock as soon as possible after GPS acquisition.
  rtc.adjust(DateTime(
    gps.date.year(),
    gps.date.month(),
    gps.date.day(),
    gps.time.hour(),
    gps.time.minute(),
    gps.time.second()
  ));

  Output("GPS:AQUIRED");
  Output("GPS->RTC Set");

  // Test RTC after setting.
  now = rtc.now();

  if ((now.year() >= TM_VALID_YEAR_START) &&
      (now.year() <= TM_VALID_YEAR_END) &&
      (now.month() >= 1) &&
      (now.month() <= 12) &&
      (now.day() >= 1) &&
      (now.day() <= 31)) {

    // Put GPS to sleep after validating the RTC.
    gps_sleep(GPS_STANDBY_MODE);

    RTC_valid = true;
    Output("RTC:VALID");

    // Save the GPS information.
    gps_lat  = gps.location.lat();
    gps_lon  = gps.location.lng();
    gps_altm = gps.altitude.meters();
    gps_altf = gps.altitude.feet();
    gps_sat  = gps.satellites.value();
    gps_hdop = gps.hdop.hdop();

    gps_displayInfo();
    return true;
  }
  else {
    RTC_valid = false;  // After Setting the RTC with GPS time, our tests show we now made it invalid.
  }

  Output("RTC:CHECK FAILED");

  // Preserve existing behavior: leave the GPS running on RTC failure.
  return false;
}

/* 
 *=======================================================================================================================
 * gps_initialize() - 
 *=======================================================================================================================
 */
void gps_initialize() {
  Output("GPS:INIT");
  pinMode(GPS_RST_PIN, OUTPUT);
  digitalWrite(GPS_RST_PIN, HIGH);  // running state for the gps
  delay (1000);
  gps_reset();

  if (I2C_Device_Exist(GPS_ADDRESS)) {
    if (myI2CGPS.begin()) {
      gps_exists = true;
      Output("GPS:OK");
      gps_on = true;
      Output("GPS:ON");
      
      gps_aquire();
    }
    else {
      // We reset the gps but it did not respond to begin(), so we can not assume it is on
      Output("GPS:INIT ERR");
    }
  }
  else {
    Output ("GPS:NF");
  }
}