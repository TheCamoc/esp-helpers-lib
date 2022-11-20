#ifndef __WifiManagement__
#define __WifiManagement__

#ifdef ESP8266
#include<ESP8266Wifi.h>
#elif ESP32
#include<WiFi.h>
#endif

bool wifiConnect(const char* ssid, const char* password, int timeoutms);

void enableAP();

#endif