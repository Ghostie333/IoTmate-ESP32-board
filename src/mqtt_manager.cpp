#include "mqtt_manager.h"
#include "config.h"
#include "device_identity.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "certificates.h"

MqttManager::MqttManager(RelayManager &relayMgr)
    : _mqttClient(_wifiClient), _relayManager(relayMgr) {}

void MqttManager::begin()
{
    _topicStatus = "devices/" + DEVICE_ID + "/status";
    _topicState = "devices/" + DEVICE_ID + "/state";
    _topicCommand = "devices/" + DEVICE_ID + "/command";

    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    _mqttClient.setBufferSize(512);
    _mqttClient.setKeepAlive(MQTT_KEEPALIVE_SEC);
    _mqttClient.setSocketTimeout(MQTT_SOCKET_TIMEOUT_SEC);

#if defined(MQTT_TLS) && !defined(MQTT_DEV_TLS)
    _wifiClient.setCACert(ROOT_CA); // production: verify the server certificate
#elif defined(MQTT_TLS)
    _wifiClient.setInsecure(); // dev only: self-signed cert, skip verification
#else
    _wifiClient.setInsecure(); // plaintext 1883, no TLS involved
#endif

    _mqttClient.setCallback(
        [this](char *topic, byte *payload, unsigned int length)
        {
            String message = "";
            for (unsigned int i = 0; i < length; i++)
            {
                message += (char)payload[i];
            }
            Serial.printf("[MQTT] Received on [%s]: %s\n", topic, message.c_str());
            this->handleCommand(message);
        });
}

void MqttManager::loop()
{
    const unsigned long now = millis();

    if (_state == MqttState::Connected)
    {
        if (!_mqttClient.connected())
        {
            _mqttClient.loop();
            _state = MqttState::Disconnected;
            return;
        }

        _mqttClient.loop();

        // Heartbeat timer
        if (now - _lastHeartbeat >= HEARTBEAT_INTERVAL_MS)
        {
            _lastHeartbeat = now;
            publishState();
        }
        return;
    }

    // Disconnected: schedule connection attempts at a fixed interval (non-blocking retry)
    if (now - _lastConnectAttempt >= MQTT_RETRY_DELAY_MS)
    {
        _lastConnectAttempt = now;
        if (reconnect())
        {
            _state = MqttState::Connected;
        }
    }
}

bool MqttManager::reconnect()
{
    if (WiFi.status() != WL_CONNECTED)
        return false;

    // 1. Load credentials from NVS or provision/register them via the backend
    MqttCredentials creds = fetchCredentials(DEVICE_ID);

    if (creds.user.isEmpty() || creds.pass.isEmpty())
    {
        Serial.println("[MQTT] No credentials to connect.");
        return false;
    }

    const char *lwtPayload = "{\"online\":false}";

    // 2. Connect to the broker
    bool connected = _mqttClient.connect(
        creds.user.c_str(), // Client ID (usually MAC/deviceId)
        creds.user.c_str(), // Username
        creds.pass.c_str(), // Password (secretToken from API)
        _topicStatus.c_str(),
        1,
        true,
        lwtPayload);

    if (connected)
    {
        Serial.println("[MQTT] Connected successfully!");
        _mqttClient.publish(_topicStatus.c_str(), "{\"online\":true}", true);
        _mqttClient.subscribe(_topicCommand.c_str());
        publishState();
        return true;
    }

    int state = _mqttClient.state();
    Serial.printf("[MQTT] Connection error, rc=%d\n", state);

    // rc = 5 (MQTT_CONNECT_UNAUTHORIZED)
    if (state == 5)
    {
        Serial.println("[MQTT] Credentials rejected (rc=5)! Clearing NVS and re-provisioning...");
        clearCredentials();
    }

    return false;
}

void MqttManager::publishState()
{
    String stateJson = _relayManager.getStatesAsJson();

    // Publish telemetry state with Retain = true
    _mqttClient.publish(_topicState.c_str(), stateJson.c_str(), true);
    Serial.printf("[MQTT] Published State: %s\n", stateJson.c_str());
}

void MqttManager::handleCommand(const String &message)
{
    bool requestedState = false;
    uint8_t relayNum = 0;

    if (message.startsWith("ON_"))
    {
        requestedState = true;
        relayNum = message.substring(3).toInt();
    }
    else if (message.startsWith("OFF_"))
    {
        requestedState = false;
        relayNum = message.substring(4).toInt();
    }
    else
    {
        return; // Invalid command format
    }

    _relayManager.setRelay(relayNum, requestedState);
    publishState();
}

MqttCredentials MqttManager::fetchCredentials(const String &deviceId)
{
    Preferences prefs;
    prefs.begin("mqtt_config", false);

    String savedUser = prefs.getString("user", "");
    String savedPass = prefs.getString("pass", "");

    // Return stored credentials if they exist
    if (savedUser.length() > 0 && savedPass.length() > 0)
    {
        Serial.println("[PROVISION] Loaded MQTT credentials from NVS.");
        prefs.end();
        return {savedUser, savedPass};
    }

    HTTPClient http;
    String backend_address = String(BACKEND_SCHEME) + "://" + String(BACKEND_SERVER) + ":" + String(BACKEND_PORT) + "/api/devices/register";

    Serial.print("[HTTP] Connecting to backend: ");
    Serial.println(backend_address);

    // Use the secure client so HTTPS verifies the server certificate via ROOT_CA
    http.begin(_wifiClient, backend_address);
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.addHeader("Content-Type", "application/json");

    // Construct JSON payload using modern ArduinoJson v7 JsonDocument
    JsonDocument reqDoc;
    reqDoc["deviceId"] = deviceId;
    reqDoc["deviceSecret"] = HARDWARE_SECRET;
    reqDoc["deviceType"] = static_cast<int>(DEVICE_TYPE);
    reqDoc["outletsCount"] = NUM_OF_OUTLETS;

    String jsonBody;
    serializeJson(reqDoc, jsonBody);

    Serial.print("[HTTP] Sending JSON: ");
    Serial.println(jsonBody);

    int httpCode = http.POST(jsonBody);
    MqttCredentials creds = {"", ""};

    if (httpCode == HTTP_CODE_OK) // 200 OK
    {
        String response = http.getString();
        Serial.print("[HTTP] Response from Backend: ");
        Serial.println(response);

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error)
        {
            String user = doc["deviceId"].as<String>();
            String pass = doc["secretToken"].as<String>();

            if (user.length() > 0 && pass.length() > 0)
            {
                creds.user = user;
                creds.pass = pass;

                // Save new valid credentials to NVS
                prefs.putString("user", creds.user);
                prefs.putString("pass", creds.pass);
                Serial.println("[PROVISION] Saved new MQTT credentials to NVS.");
            }
        }
    }
    else if (httpCode == 401) // 401 Unauthorized
    {
        Serial.println("[PROVISION] ERROR 401: Invalid deviceSecret. Clearing local credentials.");
        prefs.clear(); // prefs already open in this scope - avoid nested Preferences handle
    }
    else if (httpCode == 409) // 409 Conflict
    {
        Serial.println("[PROVISION] ERROR 409: Device conflict or invalid credentials. Clearing local credentials.");
        prefs.clear(); // prefs already open in this scope - avoid nested Preferences handle
    }
    else
    {
        Serial.printf("[HTTP] Request failed, HTTP code: %d\n", httpCode);
    }

    http.end();
    prefs.end();
    return creds;
}

void MqttManager::clearCredentials()
{
    Preferences prefs;
    prefs.begin("mqtt_config", false);
    prefs.clear();
    prefs.end();
    Serial.println("[NVS] MQTT credentials cleared successfully.");
}