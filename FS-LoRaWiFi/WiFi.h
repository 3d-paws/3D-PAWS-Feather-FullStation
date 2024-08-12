/*
 * ======================================================================================================================
 *  WiFi.h - WiFi Functions
 * ======================================================================================================================
 */

void BackGroundWork(); // Prototype this function to aviod compile function unknown issue.

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

#define NTP_PACKET_SIZE 48
#define NTP_TIMEOUT 1500  // ms
WiFiUDP udp;
unsigned int localPort = 2390;

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

  udp.begin(localPort);   // Start UDP
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
    rtc.adjust(dt_networktime);
    Output("WiFi RTC SET");
    
    rtc_timestamp();
    sprintf (msgbuf, "%sW", timestamp);
    Output (msgbuf);
    
    RTC_valid = true;
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
    Output ("WiFiConn SSID");
    sprintf (msgbuf, "WiFi SSID: [%s]", cf_wifi_ssid);
    Output(msgbuf);
    
    if ((strcmp(cf_wifi_pw,"") == 0) && (strcmp(cf_wifi_wepkey,"") == 0)) {
      // Connect to Open WiFi Network
      Output("WiFi Type: Open");
      WiFiStatus = WiFi.begin(cf_wifi_ssid);
      return (true);
    }
    else if ((strcmp(cf_wifi_pw,"") != 0) && (strcmp(cf_wifi_wepkey,"") == 0)) {
      // connect to WPA-encrypted Wifi network
      Output("WiFi Type: WPA/2");
      WiFiStatus = WiFi.begin(cf_wifi_ssid, cf_wifi_pw);
      return (true);
    }
    else if ((strcmp(cf_wifi_pw,"") != 0) && (strcmp(cf_wifi_wepkey,"") != 0)) {
       // Connect to WEP-secured network using default key index (typically 0).
      Output("WiFi Type: WEP");
      WiFiStatus = WiFi.begin(cf_wifi_ssid, cf_wifi_wepkey);
      return (true);
    }
    else {
      // Error unknown Method
      sprintf (msgbuf, "WiFi CONERR: NO PW/WKEY");
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
 * WiFi_Send_http()
 * ======================================================================================================================
 */
bool WiFi_Send_http(char *obs) {
  char response[64];
  char buf[96];
  int r, exit_timer=0;
  bool posted = false;

  if (WiFi.status() != WL_CONNECTED) {
    WiFi_connect_wait(30);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Output("OBS:SEND->HTTP");
    if (!wifi.connect(cf_webserver, cf_webserver_port)) {
      Output("OBS:HTTP FAILED");
    }
    else {        
      Output("OBS:HTTP CONNECTED");
      
      // Make a HTTP request:
      wifi.print("GET ");
      wifi.print(obs); // path
      wifi.println(" HTTP/1.1");
      wifi.print("Host: ");
      wifi.println(cf_webserver);
      wifi.println("Connection: close");
      wifi.println();

      Output("OBS:HTTP SENT");

      // Check every 1s for data. While waiting take Wind Readings every 1s.
      exit_timer = 0;     
      while(wifi.connected() && !wifi.available()) {     
        BackGroundWork(); // Burns 1 Second
        if (++exit_timer >= 60) { // after 1 minutes lets call it quits
          break;
        }
      }
      
      Output("OBS:HTTP WAIT");
        
      // Read first line of HTTP Response, then get out of the loop
      r=0;
      while ((wifi.connected() || wifi.available() ) && r<63 && !posted) {
        response[r] = wifi.read();
        response[++r] = 0;  // Make string null terminated
        if (strstr(response, "200 OK") != NULL) { // Does response includes a "200 OK" substring?
          posted = true;
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

      sprintf (buf, "OBS:%s", response);
      Output(buf);
      
      // Server disconnected from clinet. No data left to read. Disconnect client from the server
      wifi.stop();

      sprintf (buf, "OBS:%sPosted", (posted) ? "" : "Not ");
      Output(buf);
    }
  }
  return (posted);
}

/*
 * ======================================================================================================================
 * WiFi_Send_https()
 * ======================================================================================================================
 */
bool WiFi_Send_https(char *obs) {
  char response[64];
  char buf[96];
  int r, exit_timer=0;
  bool posted = false;

  if (WiFi.status() != WL_CONNECTED) {
    WiFi_connect_wait(30);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Output("OBS:SEND->HTTP");
    if (!wifi_ssl.connect(cf_webserver, cf_webserver_port)) {
      Output("OBS:HTTP FAILED");
    }
    else {        
      Output("OBS:HTTP CONNECTED");
      
      // Make a HTTP request:
      wifi_ssl.print("GET ");
      wifi_ssl.print(obs); // path
      wifi_ssl.println(" HTTP/1.1");
      wifi_ssl.print("Host: ");
      wifi_ssl.println(cf_webserver);
      wifi_ssl.println("Connection: close");
      wifi_ssl.println();

      Output("OBS:HTTP SENT");

      // Check every 1s for data. While waiting take Wind Readings every 1s.
      exit_timer = 0;     
      while(wifi_ssl.connected() && !wifi_ssl.available()) {     
        BackGroundWork(); // Burns 1 Second
        if (++exit_timer >= 60) { // after 1 minutes lets call it quits
          break;
        }
      }
      
      Output("OBS:HTTP WAIT");
        
      // Read first line of HTTP Response, then get out of the loop
      r=0;
      while ((wifi_ssl.connected() || wifi_ssl.available() ) && r<63 && !posted) {
        response[r] = wifi_ssl.read();
        response[++r] = 0;  // Make string null terminated
        if (strstr(response, "200 OK") != NULL) { // Does response includes a "200 OK" substring?
          posted = true;
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

      sprintf (buf, "OBS:%s", response);
      Output(buf);
      
      // Server disconnected from clinet. No data left to read. Disconnect client from the server
      wifi_ssl.stop();

      sprintf (buf, "OBS:%sPosted", (posted) ? "" : "Not ");
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
bool WiFi_Send(char *obs) {
  if (cf_webserver_port == 80) {
    return (WiFi_Send_http(obs));
  }
  else {
    return (WiFi_Send_https(obs));
  }
}

/* 
 *=======================================================================================================================
 * WiFi_initialize()
 *=======================================================================================================================
 */
void WiFi_initialize() {
  Output ("WiFi:INIT");

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
