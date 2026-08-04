#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "relay_manager.h"

class MqttManager
{
public:
    MqttManager(RelayManager &relayMgr);
    void begin();
    void loop();
    void publishState();

private:
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    RelayManager &_relayManager;
    unsigned long _lastHeartbeat = 0;

    void reconnect();
    void handleCommand(const String &message);
};

#endif