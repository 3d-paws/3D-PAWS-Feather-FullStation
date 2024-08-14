/*
 * ======================================================================================================================
 *  GPS.h - GPS Functions
 * ======================================================================================================================
 */

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
 *  Globals
 * ======================================================================================================================
 */
int OBS_Send(char *); // Prototype this function to aviod compile function unknown issue

#define GPS_ADDRESS 0x10
I2CGPS myI2CGPS; //Hook object to the library
TinyGPSPlus gps; //Declare gps object

bool gps_exists=false;
bool gps_valid=false;
bool gps_need2pub=true;

char gps_timestamp[32];

double gps_lat=0.0;
double gps_lon=0.0;
double gps_altm=0.0;
double gps_altf=0.0;
int    gps_sat=0;

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
 * gps_initialize() - 
 *=======================================================================================================================
 */
void gps_initialize(bool verbose) {

  // This is set so we only run the begin() once. But can call gps_initialize() multiple times to try to obtain time.
  if (!gps_exists) {
    if (myI2CGPS.begin()) {    
      Output("GPS:FOUND");
      gps_exists=true;
      SystemStatusBits &= ~SSB_GPS; // Turn Off Bit 
    }
    else {
      if (verbose) Output("GPS:NF");
    }
  }

  
  if (gps_exists) {
    uint64_t wait;
    uint64_t period;
    uint16_t default_year = gps.date.year();   // This will be 2000 if it has not updated

    if ((default_year<TM_VALID_YEAR_START) || (default_year>TM_VALID_YEAR_END) ) {
      if (verbose) Output("GPS:TM NOT VALID"); 
      
      wait = millis() + 30000;
      period = millis() + 1000;
      while (wait > millis()) {    
        if (verbose && SerialConsoleEnabled && (millis() > period)) {
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

    if (gps_valid ) {
      Output ("GPS:VALID");
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
      if (verbose) Output("GPS:TM NO SYNC");
    }    
  }
}

/*
 * ======================================================================================================================
 * gps_publish()
 * ======================================================================================================================
 */
void gps_publish() {
  gps_need2pub = false;
  
  memset(obsbuf, 0, sizeof(obsbuf));

  time_t ts = rtc_unixtime();
  tm *dt = gmtime(&ts);
  float bv = vbat_get(); 

  if (WiFi_valid) {
    sprintf (obsbuf, "%s?key=%s&instrument_id=%d", cf_urlpath, cf_apikey, cf_instrument_id);
  }

  sprintf (obsbuf+strlen(obsbuf), "at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
    dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
    dt->tm_hour, dt->tm_min, dt->tm_sec);
  sprintf (obsbuf+strlen(obsbuf), "&ms=%u", millis());

  sprintf (obsbuf+strlen(obsbuf), "&bv=%d.%02d",
     (int)bv, (int)(bv*100)%100); 
  sprintf (obsbuf+strlen(obsbuf), "&hth=%d",  SystemStatusBits);

  sprintf (obsbuf+strlen(obsbuf), "&lat=%f",  gps_lat);
  sprintf (obsbuf+strlen(obsbuf), "&lon=%f",  gps_lon);
  sprintf (obsbuf+strlen(obsbuf), "&altm=%f", gps_altm);
  sprintf (obsbuf+strlen(obsbuf), "&altf=%f", gps_altf);
  sprintf (obsbuf+strlen(obsbuf), "&sat=%d",  gps_sat);
  
  Serial_writeln (obsbuf);

  if (OBS_Send(obsbuf) != 1) {  
    Output("GPS->PUB FAILED");
  }
  else {
    Output("GPS->PUB OK");
  }
}
