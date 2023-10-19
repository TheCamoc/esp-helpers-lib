#ifndef __MQTTManagement__
#define __MQTTManagement__

#include <PubSubClient.h>

void mqttReconnect(PubSubClient client, String domain, int port = 1883);

#endif