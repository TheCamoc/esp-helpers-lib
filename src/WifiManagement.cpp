#include <WifiManagement.h>

bool wifiConnect(String ssid, String password, String hostname, int timeoutms)
{
    WiFi.mode(WIFI_STA);
    trySetHostname(hostname);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int retries = 0;
    while(WiFi.status() != WL_CONNECTED) {
        delay(100);
        retries++;
        if (retries >= (timeoutms / 100)) {
            return false;
        }
    }

    WiFi.setAutoReconnect(true);
    return true;
}

void enableAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(172,0,0,1), IPAddress(172,0,0,1), IPAddress(255,255,255,0));
    WiFi.softAP("ESP Setup");
}

void trySetHostname(String hostname) {
    if (!hostname.equals("")) {
        WiFi.setHostname(hostname.c_str());
        WiFi.hostname(hostname);
    }
}