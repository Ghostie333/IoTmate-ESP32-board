#include "mqtt_manager.h"
#include "config.h"

MqttManager::MqttManager(RelayManager &relayMgr)
    : _mqttClient(_wifiClient), _relayManager(relayMgr) {}

void MqttManager::begin()
{
    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

    // Register incoming message callback using lambda
    _mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length)
                            {
            String message = "";
            for (unsigned int i = 0; i < length; i++) {
                message += (char)payload[i];
            }
            Serial.printf("[MQTT] Received on [%s]: %s\n", topic, message.c_str());
            this->handleCommand(message); });
}

void MqttManager::loop()
{
    if (!_mqttClient.connected())
    {
        reconnect();
    }
    _mqttClient.loop();

    // Heartbeat Timer
    unsigned long now = millis();
    if (now - _lastHeartbeat > HEARTBEAT_INTERVAL_MS)
    {
        _lastHeartbeat = now;
        publishState();
    }
}

void MqttManager::reconnect()
{
    while (!_mqttClient.connected())
    {
        Serial.print("[MQTT] Attempting connection to broker...");

        // Define Last Will and Testament (LWT)
        const char *lwtPayload = "{\"online\":false}";

        // Connect to MQTT Broker with LWT
        if (_mqttClient.connect(DEVICE_ID, MQTT_USER, MQTT_PASS, TOPIC_STATUS, 1, true, lwtPayload))
        {
            Serial.println(" connected!");

            // Publish retained ONLINE status upon successful connection
            _mqttClient.publish(TOPIC_STATUS, "{\"online\":true}", true);

            // Subscribe to incoming control commands
            _mqttClient.subscribe(TOPIC_COMMAND);

            // Publish initial hardware state (On-Change pattern)
            publishState();
        }
        else
        {
            Serial.printf(" failed, rc=%d. Retry in 5 seconds...\n", _mqttClient.state());
            delay(5000);
        }
    }
}

void MqttManager::publishState()
{
    String stateJson = _relayManager.getStatesAsJson();
    // Publish telemetry state with Retain = true
    _mqttClient.publish(TOPIC_STATE, stateJson.c_str(), true);
    Serial.printf("[MQTT] Published State: %s\n", stateJson.c_str());
}

void MqttManager::handleCommand(const String &message)
{
    if (message == "ON_1")
        _relayManager.setRelay(1, true);
    else if (message == "OFF_1")
        _relayManager.setRelay(1, false);
    else if (message == "ON_2")
        _relayManager.setRelay(2, true);
    else if (message == "OFF_2")
        _relayManager.setRelay(2, false);

    // Immediate state notification upon change
    publishState();
}