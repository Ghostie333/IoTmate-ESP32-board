#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "relay_manager.h"

struct MqttCredentials
{
    String user;
    String pass;
};

class MqttManager
{
public:
    MqttManager(RelayManager &relayMgr);
    void begin();
    void loop();
    bool reconnect();
    void publishState();
    void clearCredentials();

private:
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    RelayManager &_relayManager;

    String _topicStatus;
    String _topicState;
    String _topicCommand;

    unsigned long _lastHeartbeat = 0;

    void handleCommand(const String &message);
    MqttCredentials fetchCredentials(const String &deviceId);
};

#endif // MQTT_MANAGER_H