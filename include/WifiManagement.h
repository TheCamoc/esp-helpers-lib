#ifndef __WifiManagement__
#define __WifiManagement__

#ifdef ESP8266
#include <ESP8266Wifi.h>
#elif ESP32
#include <WiFi.h>
#endif

bool wifiConnect(String ssid, String password, String hostname, int timeoutms);
void enableAP();
void trySetHostname(String hostname);

#endif