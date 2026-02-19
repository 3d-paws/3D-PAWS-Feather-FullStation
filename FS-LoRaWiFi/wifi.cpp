/*
 * ======================================================================================================================
 *  WiFi.h - WiFi Functions
 * ======================================================================================================================
 */

#include "include/main.h"
#include "include/cf.h"
#include "include/output.h"
#include "include/time.h"
#include "include/support.h"
#include "include/wifi.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
WiFiSSLClient wifi_ssl;
WiFiClient wifi;
bool WiFi_valid = false;          // True if this is a Feather WiFi Board

/*
 * ======================================================================================================================
 * WiFi.status() returns:
 * WL_NO_SHIELD:      No Wi-Fi shield is present or detected. This indicates that the Wi-Fi hardware is not available.
 * WL_IDLE_STATUS:    The Wi-Fi shield is idle and ready to connect. It indicates that the shield is not currently 
 *                    connected to a network but is prepared to start a connection attempt.
 * WL_NO_SSID_AVAIL:  No SSID (network name) is available. This indicates that the Wi-Fi shield cannot find 
 *                    the specified SSID.
 * WL_SCAN_COMPLETED: A scan for available networks has completed. This status indicates that a network scan is finished, 
 *                    and the device can now proceed to connect to a network.
 * WL_CONNECTED:      The device is successfully connected to a Wi-Fi network. This status indicates that the connection 
 *                    to the specified network is active.
 * WL_CONNECT_FAILED: The attempt to connect to a Wi-Fi network failed. This status indicates that the connection 
 *                    attempt did not succeed.
 * WL_DISCONNECTED:   The device has disconnected from the Wi-Fi network. This status indicates that the connection 
 *                    was lost or terminated.
 * ======================================================================================================================
 */
int WiFiStatus = WL_NO_SHIELD;

WiFiUDP udp;
unsigned int UDP_LocalPort = 2390;

char WiFiAuthType[8];

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * WiFi_SendNTP() - Send a NTP request
 * ======================================================================================================================
 */
void WiFi_SendNTP(IPAddress& address) {
  byte packetBuffer[NTP_PACKET_SIZE];
  
  // Initialize packetBuffer
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  
  // Set the first byte to 0b11100011, which is LI (Leap Indicator) = 3 (not in sync),
  // VN (Version Number) = 4, and Mode = 3 (Client)
  packetBuffer[0] = 0b11100011;
  
  // Send the NTP request
  udp.beginPacket(address, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

/*
 * ======================================================================================================================
 * Wifi_GetTime() - Make a NTP request
 * ======================================================================================================================
 */
unsigned long Wifi_GetTime() {
  IPAddress ntpServerIP;
    
  Output ("WiFi GetTime()");

  udp.begin(UDP_LocalPort);   // Start UDP
  WiFi.hostByName(cf_ntpserver, ntpServerIP);  // Do DNS Lookup to get NTP server's IP

  Output ("WiFi NTP Req");
  WiFi_SendNTP(ntpServerIP);

  unsigned long startMillis = millis();
  Output ("WiFi NTP Wait");
  while (!udp.parsePacket()) {
    if (millis() - startMillis >= NTP_TIMEOUT) {
      return 0; // Timeout
    }
  }

  Output ("WiFi UDP Read");
  byte packetBuffer[NTP_PACKET_SIZE];
  udp.read(packetBuffer, NTP_PACKET_SIZE);
  
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  
  const unsigned long seventyYears = 2208988800UL; // Epoch starts at 1970
  unsigned long epoch = secsSince1900 - seventyYears; // If Local Time Zone desired add  "+ timeZone * 3600"

  Output ("WiFi NTP OK");
  return (epoch);
}

/*
 * ======================================================================================================================
 * WiFi_UpdateTime() - Function to get network time and set rtc
 * ======================================================================================================================
 */
void WiFi_UpdateTime() {

  WiFiStatus = WiFi.status();

  if ((WiFiStatus == WL_CONNECTED) && RTC_exists) {
    unsigned long networktime = Wifi_GetTime();

    DateTime dt_networktime = DateTime(networktime);
    if ((dt_networktime.year() >= TM_VALID_YEAR_START) && (dt_networktime.year() <= TM_VALID_YEAR_END)) {
      rtc.adjust(dt_networktime);
      Output("WiFi RTC SET");
      rtc_timestamp();
      sprintf (msgbuf, "%sW", timestamp);
      Output (msgbuf);
      RTC_valid = true;
    }
    else {
      Output("WiFi RTC ERROR");
    }
  }
}

/*
 * ======================================================================================================================
 * WiFi_NSLookup()
 * ======================================================================================================================
 */
void WiFi_NSLookup(char *host) {
  IPAddress ip;  // Stores resolved IP

  Output("WiFi:NS");
  if (WiFi.status() == WL_CONNECTED) {
    int result = WiFi.hostByName(host, ip);
    if (result == 1) {
      sprintf(msgbuf, "WiFi IP: %s->%d.%d.%d.%d", host, ip[0], ip[1], ip[2], ip[3]);
    }
    else {
      sprintf(msgbuf, "WiFi IP: %s->0.0.0.0"); 
    }
    Output(msgbuf);
  }
}

/*
 * ======================================================================================================================
 * WiFi_Feather() - check for the presence of WiFi
 * ======================================================================================================================
 */
bool WiFi_Feather() {
  // check for the presence of the shield:
  WiFiStatus = WiFi.status();
  if (WiFiStatus == WL_NO_SHIELD) {
    WiFi_valid = false;
  }
  else {
    WiFi_valid = true;
  }
  return (WiFi_valid);
}

/* 
 *=======================================================================================================================
 * WiFi_connect - return true if called WiFi.begin
 *=======================================================================================================================
 */
bool WiFi_connect() {
  if (strcmp(cf_wifi_ssid,"") != 0) {
    sprintf (msgbuf, "WiFiCon SSID: [%s]", cf_wifi_ssid);
    Output(msgbuf);

    if ((strcmp(cf_wifi_pw,"") == 0) && (strcmp(cf_wifi_wepkey,"") == 0)) {
      // Connect to Open WiFi Network
      strcpy (WiFiAuthType, "Open");
      sprintf (msgbuf, "WiFi Type: %s", WiFiAuthType);
      Output(msgbuf);
      WiFiStatus = WiFi.begin(cf_wifi_ssid);
      return (true);
    }
    else if ((strcmp(cf_wifi_pw,"") != 0) && (strcmp(cf_wifi_wepkey,"") == 0)) {
      // connect to WPA-encrypted Wifi network
      strcpy (WiFiAuthType, "WPA/2");
      sprintf (msgbuf, "WiFi Type: %s", WiFiAuthType);
      Output(msgbuf);
      WiFiStatus = WiFi.begin(cf_wifi_ssid, cf_wifi_pw);
      return (true);
    }
    else if ((strcmp(cf_wifi_pw,"") != 0) && (strcmp(cf_wifi_wepkey,"") != 0)) {
       // Connect to WEP-secured network using default key index (typically 0).
      strcpy (WiFiAuthType, "WEP");
      sprintf (msgbuf, "WiFi Type: %s", WiFiAuthType);
      Output(msgbuf);
      WiFiStatus = WiFi.begin(cf_wifi_ssid, cf_wifi_wepkey);
      return (true);
    }
    else {
      // Error unknown Method
      strcpy (WiFiAuthType, "UNDEF");
      sprintf (msgbuf, "WiFi Type: %s", WiFiAuthType);
      Output(msgbuf);
      return(false);   
    } 
  }
  else {
    // Error no SID defined
    sprintf (msgbuf, "WiFi CONERR: NO SSID");
    Output(msgbuf);
    return(false);
  }
}

/* 
 *=======================================================================================================================
 * WiFi_connect_wait()
 *=======================================================================================================================
 */
void WiFi_connect_wait(int wait_seconds) {
  Output ("WiFiConnWait");
  if (WiFi_connect()) {
    // Wait until connected or timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < (wait_seconds*1000)) {
      BackGroundWork(); // Burns 1 Second
    }

    IPAddress ip = WiFi.localIP();
    char ipStr[16]; // Buffer to hold the IP address as a string
    sprintf(msgbuf, "WiFi IP: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    Output(msgbuf);
  
    long rssi = WiFi.RSSI();
    sprintf(msgbuf, "WiFi RSSI: %ld", rssi);
    Output(msgbuf); 
  }
}

/*
 * ======================================================================================================================
 * WiFi_Send_http() - returns 0, 1 (OK), -500
 * ======================================================================================================================
 */
 int WiFi_Send_http(char *msg, char *webserver, int webserver_port, char *webserver_path, int webserver_method, char *webserver_xapikey) {
  char response[64];
  char buf[96];
  int r, exit_timer=0;
  int posted = 0;

  if (WiFi.status() == WL_CONNECTED) {
    Output("WiFi:SEND->HTTP");
    if (!wifi.connect(webserver, webserver_port)) {
      Output("WiFi:HTTP FAILED");
    }
    else {        
      Output("WiFi:HTTP CONNECTED");
  
      // Make a HTTP request:
      if (webserver_method == METHOD_GET) {
        // Make a HTTP GET request:
        wifi.print("GET ");
        wifi.print(msg); // path
        wifi.println(" HTTP/1.1");
        wifi.print("Host: ");
        wifi.println(webserver);
        wifi.println(String("X-API-Key: ") + webserver_xapikey);
        wifi.println("Connection: close");
        wifi.println(); // blank line ends headers
      }
      else {
        // Construct HTTP POST request    
        wifi.println(String("POST ") + webserver_path + " HTTP/1.1");
        wifi.println(String("Host: ") + webserver);
        wifi.println("Content-Type: application/json");
        wifi.print("Content-Length: ");
        wifi.println(strlen(msg));
        wifi.println(String("X-API-Key: ") + webserver_xapikey);
        wifi.println("Connection: close");
        wifi.println(); // blank line after headers
        wifi.print(msg);
      }

      Output("WiFi:HTTP SENT");

      // Check every 1s for data. While waiting take Wind Readings every 1s.
      exit_timer = 0;     
      while(wifi.connected() && !wifi.available()) {     
        BackGroundWork(); // Burns 1 Second
        if (++exit_timer >= 60) { // after 1 minutes lets call it quits
          break;
        }
      }
      
      Output("WiFi:HTTP WAIT");
        
      // Read first line of HTTP Response, then get out of the loop
      r=0;
      while ((wifi.connected() || wifi.available() ) && r<63 && (posted == 0)) {
        response[r] = wifi.read();
        response[++r] = 0;  // Make string null terminated
        if (strstr(response, "200 OK") != NULL) { // Does response includes a "200 OK" substring?
          posted = 1;
          break;
        }
        if (strstr(response, "500 Internal") != NULL) { // Does response includes a This Error substring?
          posted = -500;
          break;
        }        
        if ((response[r-1] == 0x0A) || (response[r-1] == 0x0D)) { // LF or CR
          // if we got here then we never saw the 200 OK
          break;
        }
      }
  
      // Read rest of the response after first line
      // while (wifi.connected() || wifi.available()) { //connected or data available
      //   char c = wifi.read(); //gets byte from ethernet buffer
      //   Serial.print (c);
      // }

      sprintf (buf, "WiFi:%s", response);
      Output(buf);
      
      // Server disconnected from clinet. No data left to read. Disconnect client from the server
      wifi.stop();

      sprintf (buf, "WiFi:%sPosted=%d", (posted == 1) ? "" : "Not ", posted);
      Output(buf);
    }
  }
  return (posted);
}

/*
 * ======================================================================================================================
 * WiFi_Send_https() - returns 0, 1 (OK), -500
 * ======================================================================================================================
 */
int WiFi_Send_https(char *msg, char *webserver, int webserver_port, char *webserver_path, int webserver_method, char *webserver_xapikey) {
  char response[64];
  char buf[96];
  int r, exit_timer=0;
  int posted = 0;

  if (WiFi.status() == WL_CONNECTED) {
    WiFi_NSLookup(webserver);

    wifi_ssl.setTimeout(15000);
    wifi_ssl.stop();
    Output("WiFi:SEND->HTTPS");
    if (!wifi_ssl.connect(webserver, webserver_port)) {
      Output("WiFi:HTTPS FAILED");
      wifi_ssl.stop();
    }
    else {        
      Output("WiFi:HTTPS CONNECTED");
      
      // Make a HTTPS request:
      if (webserver_method == METHOD_GET) {
        // Make a HTTPS GET request
        wifi_ssl.print("GET ");
        wifi_ssl.print(msg); // path
        wifi_ssl.println(" HTTP/1.1");
        wifi_ssl.print("Host: ");
        wifi_ssl.println(webserver);
        wifi_ssl.println(String("X-API-Key: ") + webserver_xapikey);
        wifi_ssl.println("Connection: close");
        wifi_ssl.println(); // blank line ends headers
      }
      else {
        // Construct HTTPS POST request    
        wifi_ssl.println(String("POST ") + webserver_path + " HTTP/1.1");
        wifi_ssl.println(String("Host: ") + webserver);
        wifi_ssl.println("Content-Type: application/json");
        wifi_ssl.print("Content-Length: ");
        wifi_ssl.println(strlen(msg));
        wifi_ssl.println(String("X-API-Key: ") + webserver_xapikey);
        wifi_ssl.println("Connection: close");
        wifi_ssl.println(); // blank line after headers
        wifi_ssl.print(msg);
      }

      Output("WiFi:HTTPS SENT");

      // Check every 1s for data. While waiting take Wind Readings every 1s.
      exit_timer = 0;     
      while(wifi_ssl.connected() && !wifi_ssl.available()) {     
        BackGroundWork(); // Burns 1 Second
        if (++exit_timer >= 60) { // after 1 minutes lets call it quits
          break;
        }
      }
      
      Output("WiFi:HTTPS WAIT");
        
      // Read first line of HTTP Response, then get out of the loop
      r=0;
      while ((wifi_ssl.connected() || wifi_ssl.available() ) && r<63 && (posted == 0)) {
        response[r] = wifi_ssl.read();
        response[++r] = 0;  // Make string null terminated
        if (strstr(response, "200 OK") != NULL) { // Does response includes a "200 OK" substring?
          posted = 1;
          break;
        }
        if (strstr(response, "500 Internal") != NULL) { // Does response includes a This Error substring?
          posted = -500;
          break;
        }        
        if ((response[r-1] == 0x0A) || (response[r-1] == 0x0D)) { // LF or CR
          // if we got here then we never saw the 200 OK
          break;
        }
      }
  
      // Read rest of the response after first line
      // while (wifi_ssl.connected() || wifi_ssl.available()) { //connected or data available
      //   char c = wifi_ssl.read(); //gets byte from ethernet buffer
      //   Serial.print (c);
      // }

      sprintf (buf, "WiFi:%s", response);
      Output(buf);
      
      // Server disconnected from clinet. No data left to read. Disconnect client from the server
      wifi_ssl.stop();

      sprintf (buf, "WiFi:%sPosted", (posted == 1) ? "" : "Not ");
      Output(buf);
    }
  }
  return (posted);
}

/*
 * ======================================================================================================================
 * WiFi_Send()
 * ======================================================================================================================
 */
int WiFi_Send(char *msg, char *webserver, int webserver_port, char *webserver_path, int webserver_method, char *webserver_xapikey) {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi_connect_wait(30);
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (webserver_port == 80) {
      return (WiFi_Send_http(msg, webserver, webserver_port, webserver_path, webserver_method, webserver_xapikey));
    }
    else {
      return (WiFi_Send_https(msg, webserver, webserver_port, webserver_path, webserver_method, webserver_xapikey));
    }
  }
  else {
    return (0); // Not Posted
  }
}

/* 
 *=======================================================================================================================
 * WiFi_initialize()
 *=======================================================================================================================
 */
void WiFi_initialize() {
  Output ("WiFi:INIT");
  sprintf(msgbuf, "WiFi FW: %s", WiFi.firmwareVersion());
  Output(msgbuf); 

  //String fv = WiFi.firmwareVersion();
  //sprintf (msgbuf, "WiFi FW:[%s]", fv);
  //Output(msgbuf);

  // Validate Config File Variables for WiFi and Web Server
  if ( (strcmp(cf_wifi_ssid,"") == 0) ||
       (strcmp(cf_webserver,"") == 0) ||
       (cf_webserver_port <= 0) ||
       (strcmp(cf_urlpath,"") == 0) ||
       (strcmp(cf_apikey,"") == 0) ||
       (cf_instrument_id < 0) ||
       (strcmp(cf_ntpserver,"") == 0)) {
        
     Output("HALT:INVALID CFG");

     // Halting - Let WatchDog reboot us in 5 minutes
     while (true) {
       Blink (1,500);
     }
  }
  
  WiFi_connect_wait(30);

  WiFi_UpdateTime();
}
