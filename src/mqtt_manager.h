#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"
#include "relay_manager.h"

struct MqttCredentials
{
    String user;
    String pass;
};

class MqttManager
{
public:
    explicit MqttManager(RelayManager &relayMgr);

    void begin();
    void loop();

private:
    enum class MqttState : uint8_t
    {
        Disconnected,
        Connected
    };

    WiFiClientSecure _wifiClient;
    WiFiClient _backendClient;
    PubSubClient _mqttClient;
    RelayManager &_relayManager;

    String _topicStatus;
    String _topicState;
    String _topicCommand;

    MqttState _state = MqttState::Disconnected;
    unsigned long _lastHeartbeat = 0;
    unsigned long _lastConnectAttempt = 0;
    unsigned long _retryDelayMs = MQTT_RETRY_DELAY_MS;
    bool _forceCredentialRefresh = false;

    bool reconnect();
    void publishState();
    void handleCommand(const String &message);
    MqttCredentials fetchCredentials(const String &deviceId, bool forceRefresh);
};

#endif
