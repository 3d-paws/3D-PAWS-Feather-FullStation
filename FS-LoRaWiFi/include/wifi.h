/*
 * ======================================================================================================================
 *  wifi.h - WiFi Definations
 * ======================================================================================================================
 */
#include <WiFi101.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define NTP_PACKET_SIZE 48
#define NTP_TIMEOUT 1500  // ms

#define METHOD_GET  0
#define METHOD_POST 1

// Extern variables
extern WiFiSSLClient wifi_ssl;
extern WiFiClient wifi;
extern bool WiFi_valid;
extern int WiFiStatus;
extern char WiFiAuthType[8];

// Function prototypes
unsigned long Wifi_GetTime();
void WiFi_UpdateTime();
bool WiFi_Feather();
bool WiFi_connect();
int WiFi_Send(char *msg, char *webserver, int webserver_port, char *webserver_path, int webserver_method, char *webserver_xapikey);
void WiFi_initialize();
