#include <ESPTools.h>

static const char configIndex[] PROGMEM =
    R"(<!DOCTYPE html>
     <html lang='en'>
     <head>
        <title>ESPTools Config Page</title>
         <meta charset='utf-8'>
         <meta name='viewport' content='width=device-width,initial-scale=1'/>
         <style>
            body { background-color: #313131; font-family: Arial, Helvetica, Sans-Serif; color: #fff; }
        </style>
     </head>
     <body>
     <form method='POST' action='' enctype='multipart/form-data'>
         {{inputlist}}
         <input type='submit' value='Save'>
     </form>
     </body>
     </html>)";

static const char configStringInput[] PROGMEM = R"(
%s: <input type='text' name='%s' value='%s'><br><br>
)";

#ifdef ESP8266
void ESPToolsClass::begin(ESP8266WebServer *s)
#elif ESP32
void ESPToolsClass::begin(WebServer *s)
#endif
{
    setupFS();
    loadConfig();

    server = s;
    server->on("/config", HTTP_GET, [&]()
               { handleConfigGET(); });

    server->on("/config", HTTP_POST, [&]()
               { handleConfigPOST(); });

    server->on("/formatfs", HTTP_GET, [&]()
               { fs.format(); server->send(200, "text/plain", "Ok"); });
}

void ESPToolsClass::setupHTTPUpdates()
{
#ifdef ESP8266
    httpUpdater.setup(server);
#elif ESP32
    setupESP32Updates(server);
#endif
}

void ESPToolsClass::setupFS()
{
    if (!fs.begin())
    {
        // on fail try formatting then loading again
        fs.format();
        if (!fs.begin()) {
            log("Error loading LittleFS.");
        }
    }
}

int ESPToolsClass::loadConfig()
{
    // Check if LittleFS works and if file exists
    if (!fs.exists("/config.json"))
    {
        log("No Config file exists.");
        return -1;
    }

    // Read config file
    File configFile = fs.open("/config.json", "r");
    if (!configFile)
    {
        log("Failed to open Config File.");
        return -1;
    }
    DynamicJsonDocument jsonDoc = DynamicJsonDocument(1024);
    auto deserializeError = deserializeJson(jsonDoc, configFile);
    if (deserializeError)
    {
        log("Deserializing the Config failed.");
        return -1;
    }

    JsonObject obj = jsonDoc.as<JsonObject>();
    for (JsonPair p : obj)
    {
        config[p.key().c_str()] = p.value().as<String>();
    }

    configFile.close();
    log("Loading Config successful.");
    return 1;
}

int ESPToolsClass::saveConfig()
{
    File configFile = fs.open("/config.json", "w");
    if (!configFile)
    {
        log("Failed to open Config File.");
        return -1;
    }

    DynamicJsonDocument jsonDoc = DynamicJsonDocument(1024);
    for (std::pair<const String, String> pair : config)
    {
        jsonDoc[pair.first] = pair.second;
    }

    serializeJson(jsonDoc, configFile);
    configFile.close();
    log("Saving Config successful");
    return 1;
}

void ESPToolsClass::deleteConfig()
{
    fs.remove("/config.json");
}

void ESPToolsClass::addConfigString(String name)
{
    config[name];
}

void ESPToolsClass::handleConfigGET()
{
    char inputTemp[100];
    String inputList;
    for (std::pair<const String, String> pair : config)
    {
        sprintf(inputTemp, configStringInput, pair.first.c_str(), pair.first.c_str(), pair.second.c_str());
        inputList += inputTemp;
    }

    String body = configIndex;
    body.replace("{{inputlist}}", inputList);
    server->send(200, "text/html", body);
}

void ESPToolsClass::handleConfigPOST()
{
    for (std::pair<const String, String> pair : config)
    {
        config[pair.first] = server->arg(pair.first);
    }

    saveConfig();

    server->sendHeader("Location", "/config");
    server->send(302, "text/plain", "");
}

void ESPToolsClass::log(String message)
{
    Serial.println("[ESPTools] " + message);
}

void ESPToolsClass::wifiAutoConnect()
{
    addConfigString("ssid");
    addConfigString("password");
    addConfigString("hostname");
    
    if (!config["ssid"].equals("")) {
        if (wifiConnect(config["ssid"], config["password"], config["hostname"], 30000)) {
            log("WiFi connected to " + WiFi.SSID());
            log("IP address: " + WiFi.localIP().toString());
            return;
        }
    }

    log("Starting AP and Captive Portal");

    enableAP();
    DNSServer dnsServer;
    dnsServer.start(53, "*", IPAddress(172,0,0,1));

    server->onNotFound([this] () {
        server->sendHeader("Location", String("http://") + server->client().localIP().toString() + String("/config"));
        server->send(302, "text/plain", "");
        server->client().stop();
    });
    server->begin();

    String startSSID = config["ssid"];
    String startPassword = config["password"];

    while(true) {
        dnsServer.processNextRequest();
        server->handleClient();
        delay(100);
        if (!config["ssid"].equals(startSSID) || !config["password"].equals(startPassword)) {
            ESP.restart();
        }
    }
}

ESPToolsClass ESPTools;