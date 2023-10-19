#include <MQTTManagement.h>

void mqttReconnect(PubSubClient client, String domain, int port = 1883)
{
    Serial.print("[ESPHelpers] Attempting MQTT connection... ");
    String clientId = "ESPClient-";
    clientId += String(random(0xffff), HEX);

    client.setServer(domain.c_str(), port);
    if (client.connect(clientId.c_str()))
    {
        Serial.println("connected");
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" trying again");
        delay(100);
    }
}