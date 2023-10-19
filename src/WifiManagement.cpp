#include <WifiManagement.h>

bool wifiConnect(String ssid, String password, String hostname, int timeoutms)
{
    WiFi.mode(WIFI_STA);
    trySetHostname(hostname);
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setAutoReconnect(true);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        retries++;
        if (retries >= (timeoutms / 100))
        {
            return false;
        }
    }

    return true;
}

void enableAP(String apPassword)
{
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(IPAddress(172, 0, 0, 1), IPAddress(172, 0, 0, 1), IPAddress(255, 255, 255, 0));
    String apName = String("ESP Setup ") + WiFi.macAddress().substring(9);
    WiFi.softAP(apName.c_str(), apPassword.c_str());
}

void trySetHostname(String hostname)
{
    if (!hostname.equals(""))
    {
        WiFi.setHostname(hostname.c_str());
        WiFi.hostname(hostname);
    }
}