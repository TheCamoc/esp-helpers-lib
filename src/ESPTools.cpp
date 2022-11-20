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

void ESPToolsClass::setupFS()
{
    if (!fs.begin())
    {
        log("Error loading LittleFS.");
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
        p.key();
        p.value();
        config[p.key().c_str()] = p.value().as<const char*>();
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
    for (std::pair<const std::string, std::string> pair : config)
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

void ESPToolsClass::addConfigString(std::string name)
{
    config[name];
}

void ESPToolsClass::handleConfigGET()
{
    char inputTemp[100];
    String inputList;
    for (std::pair<const std::string, std::string> pair : config)
    {
        sprintf(inputTemp, configStringInput, pair.first.c_str(), pair.first.c_str(), pair.second.c_str());
        inputList += inputTemp;
    }
    //sprintf(resultHTML, configIndex, inputList.c_str());
    String body = configIndex;
    body.replace("{{inputlist}}", inputList);
    server->send(200, "text/html", body);
}

void ESPToolsClass::handleConfigPOST()
{
    for (std::pair<const std::string, std::string> pair : config)
    {
        config[pair.first] = server->arg(pair.first.c_str()).c_str();
    }

    saveConfig();

    server->sendHeader("Location", "/config");
    server->send(302, "text/plain", "");
}

void ESPToolsClass::log(std::string message)
{
    Serial.print("[ESPTools] ");
    Serial.println(message.c_str());
}

void ESPToolsClass::wifiAutoConnect()
{
    addConfigString("ssid");
    addConfigString("password");

    if (config["ssid"] != "") {
        if (wifiConnect(config["ssid"].c_str(), config["password"].c_str(), 30000)) {
            log("WiFi Connected");
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

    std::string startSSID = config["ssid"];
    std::string startPassword = config["password"];

    while(true) {
        dnsServer.processNextRequest();
        server->handleClient();
        delay(100);
        if (config["ssid"].compare(startSSID) != 0 || config["password"].compare(startPassword) != 0) {
            ESP.restart();
        }
    }
}

ESPToolsClass ESPTools;