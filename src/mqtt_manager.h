#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
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
    enum class MqttState : uint8_t
    {
        Disconnected,
        Connected
    };

    WiFiClientSecure _wifiClient; // secured MQTT connection to the broker
    WiFiClient _backendClient;    // plaintext connection to the backend API (dev, http://)
    PubSubClient _mqttClient;
    RelayManager &_relayManager;

    String _topicStatus;
    String _topicState;
    String _topicCommand;

    MqttState _state = MqttState::Disconnected;
    unsigned long _lastHeartbeat = 0;
    unsigned long _lastConnectAttempt = 0;

    void handleCommand(const String &message);
    MqttCredentials fetchCredentials(const String &deviceId);
};

#endif // MQTT_MANAGER_H