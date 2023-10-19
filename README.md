# ESP Helpers
## Compatibility
Compatible with ESP8266 and ESP32 using Arduino and PlatformIO

## Current Features
- A Website for configuring String options. Configuration will automatically be saved using LittleFS and persist restarts.
- Wifi Management
- Setup of HTTP Updates

## Planned Features
- Setup of Automatic OTA Updates
- Option to specify Firmware version on the config Website

## Example Usage
```c
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESPHelpers.h>

ESP8266WebServer server(80);

void setup()
{
  Serial.begin(115200);
  
  // Setup, this will add the paths for the config Website to the server
  // as well as adding a path to restart the ESP, and format the Filesystem
  // /config -> Config Website
  // /formatfs -> Format Filesystem
  // /restart -> Restart the ESP
  ESPHelpers.begin(&server);
  
  // Add a new option to the config Website, will be empty by default
  ESPHelpers.addConfigString("configOption");
  
  // Automatically try to connect to WiFi using presaved credentials 
  // or create AP with captive portal to let user input credentials
  // a password for the AP can optionally be specified
  // the password has to be min 8, max 64 characters
  ESPHelpers.wifiAutoConnect("secretpassword"); 
  
  // Setup HTTP firmware updates, accessible on path /update
  ESPHelpers.setupHTTPUpdates();
  
  server.begin();
}

void loop()
{
    // Important for the config functionality to work
    server.handleClient();

    // Check if WiFi has been disconnected for more than 4 minutes
    // if yes, reopen AP and Captive Portal to let users change Credentials
    ESPHelpers.wifiCheck();
    
    // Access current value of option and output it
    Serial.println(ESPTools.config["configOption"]);
    
    delay(100);
}
```
