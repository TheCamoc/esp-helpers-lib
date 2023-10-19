# ESP Helpers
## Compatibility
Compatible with ESP8266 and ESP32 using Arduino and PlatformIO

## Current Features
- A Website for configuring String options. Configuration will automatically be saved using LittleFS and persist restarts.
- Wifi Management
- Setup of HTTP Updates
- MQTT Connection Management

## Planned Features
- Setup of Automatic OTA Updates
- Option to specify Firmware version on the config Website

## Example Usage
```c
#include <Arduino.h>
#include <WiFiClient.h>

#ifdef ESP8266
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#elif ESP32
#include <WebServer.h>
WebServer server(80);
#endif

#include <ESPHelpers.h>

ESP8266WebServer server(80);
unsigned long last_mqtt_message = 0;

void setup()
{
  Serial.begin(115200);
  
  // Setup, this will add the paths for the config Website to the server
  // as well as adding a path to restart the ESP, and format the Filesystem
  // /config -> Config Website
  // /formatfs -> Format Filesystem
  // /restart -> Restart the ESP
  ESPHelpers.begin(&server, "secret_ap_password");
  
  // Add a new option to the config Website, will be empty by default
  ESPHelpers.addConfigString("configOption");
  
  // Automatically try to connect to WiFi using presaved credentials 
  // or create AP with captive portal to let user input credentials
  // a password for the AP can optionally be specified in the begin function (see above)
  // the password has to be min 8, max 64 characters
  ESPHelpers.wifiAutoConnect(); 
  
  // Enable MQTT management, adding a mqtt_server option to the config
  ESPHelpers.enableMQTT();

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

    // Loop needed for mqtt to work, will also automatically reconnect if needed
    ESPHelpers.mqttLoop();
    
    // Access current value of option and output it
    Serial.println(ESPHelpers.config["configOption"]);

    // Send MQTT Message every 10 seconds
    if (ESPHelpers.mqtt.connected() && (last_mqtt_message + 10 * 1000) <= millis()) {
        last_mqtt_message = millis();
        ESPHelpers.mqtt.publish("topic", ESPHelpers.config["configOption"].c_str());
    }

    delay(100);
}
```
