#ifndef __ESPTools__
#define __ESPTools__

#include <ArduinoJSON.h>
#include <LittleFS.h>
#include <string>
#include <map>
#include <WifiManagement.h>
#include <UpdateManagement.h>
#include <DNSServer.h>
#include <HTMLTemplates.h>

#ifdef ESP8266
    #include <ESP8266WebServer.h>
    #include <ESP8266HTTPUpdateServer.h>
#elif ESP32
    #include <WebServer.h>
#endif

class ESPToolsClass
{
public:
    #ifdef ESP8266
    void begin(ESP8266WebServer *server);
    #elif ESP32
    void begin(WebServer *server);
    #endif
    
    int loadConfig();
    int saveConfig();
    void deleteConfig();
    void addConfigString(String name);
    void wifiAutoConnect();
    void setupHTTPUpdates();
    
    std::map<String, String> config;
private:
    #ifdef ESP8266
    ESP8266WebServer *server;
    ESP8266HTTPUpdateServer httpUpdater;
    fs::FS fs = LittleFS;
    #elif ESP32
    WebServer *server;
    fs::LittleFSFS fs = LittleFS;
    #endif
    void handleConfigJSONGet();
    void handleConfigGET();
    void handleConfigPOST();
    void log(String message);
    void setupFS();

   
};

extern ESPToolsClass ESPTools;
#endif