#include <ESPHelpers.h>

#ifdef ESP8266
void ESPHelpersClass::begin(ESP8266WebServer *s, String apPassword)
#elif ESP32
void ESPHelpersClass::begin(WebServer *s, String apPassword)
#endif
{
    wifiAPPassword = apPassword;
    if (apPassword.length() > 0 && apPassword.length() < 8)
    {
        log("WiFi AP Password too short for WPA2, defaulting to no password");
        wifiAPPassword = "";
    }
    else if (apPassword.length() > 64)
    {
        log("WiFi AP Password too long, defaulting to no password");
        wifiAPPassword = "";
    }

    setupFS();
    loadConfig();

    server = s;
    server->on("/config", HTTP_GET, [&]()
               { handleConfigGET(); });

    server->on("/configjson", HTTP_GET, [&]()
               { handleConfigJSONGet(); });

    server->on("/config", HTTP_POST, [&]()
               { handleConfigPOST(); });

    server->on("/formatfs", HTTP_GET, [&]()
               { fs.format(); server->send(200, "text/plain", "Ok"); });

    server->on("/restart", [&]()
               {
        server->send(200, "text/plain", "Ok");
        delay(500);
        ESP.restart(); });
}

void ESPHelpersClass::setupHTTPUpdates()
{
#ifdef ESP8266
    httpUpdater.setup(server);
#elif ESP32
    setupESP32Updates(server);
#endif
}

void ESPHelpersClass::setupFS()
{
    if (fs.begin())
    {
        return;
    }

    // on fail try formatting then loading again
    fs.format();
    if (!fs.begin())
    {
        log("Error loading LittleFS.");
    }
}

int ESPHelpersClass::loadConfig()
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

int ESPHelpersClass::saveConfig()
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

void ESPHelpersClass::deleteConfig()
{
    fs.remove("/config.json");
}

void ESPHelpersClass::addConfigString(String name)
{
    config[name];
}

void ESPHelpersClass::handleConfigJSONGet()
{
    DynamicJsonDocument jsonDoc = DynamicJsonDocument(1024);
    for (std::pair<const String, String> pair : config)
    {
        jsonDoc[pair.first] = pair.second;
    }

    String response;
    serializeJson(jsonDoc, response);
    server->send(200, "application/json", response);
}

void ESPHelpersClass::handleConfigGET()
{
    server->send(200, "text/html", FPSTR(CONFIG_HTML));
}

void ESPHelpersClass::handleConfigPOST()
{
    for (std::pair<const String, String> pair : config)
    {
        config[pair.first] = server->arg(pair.first);
        if (pair.first == "mqtt_server")
        {
            mqttServerChanged = true;
        }
    }

    saveConfig();

    server->sendHeader("Location", "/config");
    server->send(302, "text/plain", "");
}

void ESPHelpersClass::log(String message)
{
    Serial.println("[ESPHelpers] " + message);
}

void ESPHelpersClass::wifiAutoConnect()
{
    addConfigString("ssid");
    addConfigString("password");
    addConfigString("hostname");

    if (!config["ssid"].equals(""))
    {
        if (wifiConnect(config["ssid"], config["password"], config["hostname"], 30000))
        {
            log("WiFi connected to " + WiFi.SSID());
            log("IP address: " + WiFi.localIP().toString());
            return;
        }
    }

    log("Starting AP and Captive Portal");

    enableAP(wifiAPPassword);
    // Captive Portal Stuff
    DNSServer dnsServer;
    dnsServer.start(53, "*", IPAddress(172, 0, 0, 1));

    server->onNotFound([this]()
                       {
        server->sendHeader("Location", String("http://") + server->client().localIP().toString() + String("/config"));
        server->send(302, "text/plain", "");
        server->client().stop(); });
    server->begin();

    String startSSID = config["ssid"];
    String startPassword = config["password"];

    // Try connecting just in case connection loss is temporary
    WiFi.begin(startSSID.c_str(), startPassword.c_str());
    while (true)
    {
        dnsServer.processNextRequest();
        server->handleClient();
        delay(100);
        if (!config["ssid"].equals(startSSID) || !config["password"].equals(startPassword))
        {
            WiFi.begin(config["ssid"].c_str(), config["password"].c_str());
            startSSID = config["ssid"];
            startPassword = config["password"];
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            if (wifiConnect(startSSID, startPassword, config["hostname"], 30000))
            {
                return;
            }
            else
            {
                enableAP(wifiAPPassword);
            }
        }
    }
}

void ESPHelpersClass::wifiCheck()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        wifiLastConnected = millis();
    }
    else
    {
        if ((wifiLastConnected + 240 * 1000) <= millis())
        {
            wifiAutoConnect();
        }
    }
}

void ESPHelpersClass::enableMQTT()
{
    mqtt.setClient(wifiClient);
    addConfigString("mqtt_server");
}

void ESPHelpersClass::mqttLoop()
{
    if (!mqtt.connected() && config["mqtt_server"] != "" && WiFi.status() == WL_CONNECTED)
    {
        mqttReconnect();
    } 
    else if (mqttServerChanged)
    {
        mqttServerChanged = false;
        log("Reconnecting MQTT because of server change");
        mqtt.disconnect();
        mqttReconnect();
    }
    mqtt.loop();
}

void ESPHelpersClass::mqttReconnect(int port)
{
    Serial.print("[ESPHelpers] Attempting MQTT connection... ");
    String clientId = "ESPClient-";
    clientId += String(random(0xffff), HEX);

    mqtt.setServer(config["mqtt_server"].c_str(), port);
    if (mqtt.connect(clientId.c_str()))
    {
        Serial.println("connected");
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.print(mqtt.state());
        Serial.println(" trying again");
        delay(100);
    }
}

ESPHelpersClass ESPHelpers;