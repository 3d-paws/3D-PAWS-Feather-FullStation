/*
 * ======================================================================================================================
 *  gps.cpp - GPS Functions
 * ======================================================================================================================
 */

#include "include/ssbits.h"
#include "include/feather.h"
#include "include/output.h"
#include "include/time.h"
#include "include/cf.h"
#include "include/wifi.h"
#include "include/main.h"
#include "include/obs.h"
#include "include/gps.h"


/*
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

char gps_timestamp[32];

double gps_lat=0.0;
double gps_lon=0.0;
double gps_altm=0.0;
double gps_altf=0.0;
int    gps_sat=0;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

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
  }
  else{
    Output("GPS:!VALID");
  }
}

/* 
 *=======================================================================================================================
 * gps_aquire() - 
 *=======================================================================================================================
 */
void gps_aquire() {
  if (gps_exists) {
    uint64_t wait;
    uint64_t period;
    uint16_t default_year = gps.date.year();   // This will be 2000 if it has not updated

    // Lets not trust gps_valid variable. Lets go with what the gps tells us for the year 
    if ((default_year<TM_VALID_YEAR_START) || (default_year>TM_VALID_YEAR_END) ) {
      Output("GPS:TM NOT VALID"); 
      
      // Lets wait around and see if we can get a valid year
      wait = millis() + 30000;
      period = millis() + 1000;
      while (wait > millis()) {    
        if (SerialConsoleEnabled && (millis() > period)) {
          Serial.print(".");  // Provide Serial Console some feedback as we loop
          period = millis() + 1000;
        }     
        if (myI2CGPS.available()) { //available() returns the number of new bytes available from the GPS module
          gps.encode(myI2CGPS.read()); //Feed the GPS parser

          // Look for a change in the year, once changed we have date, time and location
          if ((gps.date.year() >= TM_VALID_YEAR_START) && (gps.date.year() <= TM_VALID_YEAR_END) && gps.satellites.value()) {
            gps_valid = true;
            break;
          }
        }
      }
      if (SerialConsoleEnabled) Serial.println();  // Send a newline out to cleanup after all the periods we have been logging    
    }
    else {
      gps_valid = true;
    }

    if (gps_valid) {
      Output ("GPS:AQUIRED");
      //  Update RTC_Clock
      rtc.adjust(DateTime(
        gps.date.year(),
        gps.date.month(),
        gps.date.day(),
        gps.time.hour(),
        gps.time.minute(),
        gps.time.second()
      ));
      Output("GPS->RTC Set");
      now = rtc.now();
      if ((now.year() >= TM_VALID_YEAR_START) && (now.year() <= TM_VALID_YEAR_END)) {
        RTC_valid = true;
        Output("RTC:VALID");
      }
      else {
        RTC_valid = false;
        Output ("RTC:NOT VALID");
      }

      gps_lat  = gps.location.lat();
      gps_lon  = gps.location.lng();
      gps_altm = gps.altitude.meters();
      gps_altf = gps.altitude.feet();
      gps_sat  = gps.satellites.value();

      gps_displayInfo();
    }
    else {
      Output("GPS:NOT AQUIRED");
    }    
  }
}

/* 
 *=======================================================================================================================
 * gps_initialize() - 
 *=======================================================================================================================
 */
void gps_initialize() {
  if (!gps_exists) {
    if (myI2CGPS.begin()) {    
      Output("GPS:FOUND");
      gps_exists=true;
      // We still need gps_valid to be set by gps_aquire()
      gps_aquire();
    }
    else {
      Output("GPS:NF");
    }
  }
}
