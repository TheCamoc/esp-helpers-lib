# ESP Tools
## Current Features
- Add Website for configuring String options. Configuration will automatically be saved using LittleFS and persist restarts.

## Planned Features
- Wifi Management
- Setup of OTA Updates via the HTTP Server

## Example Usage
```c
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESPTools.h>

ESP8266WebServer server(80);

void setup()
{
  Serial.begin(115200);
  
  ESPTools.begin(&server); // Setup ESP Tools
  ESPTools.addConfigString("configOption"); // Add new config option, if doesn't have a saved value the default will be an empty string
  
  server.begin();
}

void loop()
{
    server.handleClient();
    Serial.println(ESPTools.config["configOption"]); // Access current value of option and output it
    delay(100);
}
```
