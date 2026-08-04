#include "mqtt_manager.h"
#include "config.h"
#include "device_identity.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

MqttManager::MqttManager(RelayManager &relayMgr)
    : _mqttClient(_wifiClient), _relayManager(relayMgr) {}

void MqttManager::begin()
{
    _topicStatus = "devices/" + DEVICE_ID + "/status";
    _topicState = "devices/" + DEVICE_ID + "/state";
    _topicCommand = "devices/" + DEVICE_ID + "/command";

    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    _mqttClient.setBufferSize(512); // WAŻNE: Wymagane dla tokenów 64-bitowych (SHA256)

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

bool MqttManager::reconnect()
{
    if (WiFi.status() != WL_CONNECTED)
        return false;

    // 1. Wczytaj poświadczenia z NVS lub wyrejestruj/pobierz z backendu
    MqttCredentials creds = fetchCredentials(DEVICE_ID);

    if (creds.user.isEmpty() || creds.pass.isEmpty())
    {
        Serial.println("[MQTT] Brak poświadczeń. Ponowna próba za 5s...");
        delay(5000);
        return false;
    }

    Serial.printf("[MQTT] Próba połączenia do brokera jako: %s z tokenem: %s...\n",
                  creds.user.c_str(), creds.pass.c_str());

    const char *lwtPayload = "{\"online\":false}";

    // 2. Łączenie z brokerem
    bool connected = _mqttClient.connect(
        creds.user.c_str(), // Client ID (zazwyczaj MAC/deviceId)
        creds.user.c_str(), // Username
        creds.pass.c_str(), // Password (secretToken z API)
        _topicStatus.c_str(),
        1,
        true,
        lwtPayload);

    if (connected)
    {
        Serial.println("[MQTT] Połączono pomyślnie!");
        _mqttClient.publish(_topicStatus.c_str(), "{\"online\":true}", true);
        _mqttClient.subscribe(_topicCommand.c_str());
        publishState();
        return true;
    }
    else
    {
        int state = _mqttClient.state();
        Serial.printf("[MQTT] Błąd połączenia, rc=%d\n", state);

        // rc = 5 (MQTT_CONNECT_UNAUTHORIZED)
        if (state == 5)
        {
            Serial.println("[MQTT] Odrzucono poświadczenia (rc=5)! Czyszczenie NVS...");
            clearCredentials();
            delay(3000); // Dajmy czas na ustabilizowanie
        }
        else
        {
            delay(2000);
        }

        return false;
    }
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
    String backend_address = "http://" + String(BACKEND_SERVER) + ":" + String(BACKEND_PORT) + "/api/devices/register";

    Serial.print("[HTTP] Connecting to backend: ");
    Serial.println(backend_address);

    http.begin(backend_address);
    http.addHeader("Content-Type", "application/json");

    // Construct JSON payload with device ID and hardware secret
    String jsonBody = "{\"deviceId\":\"" + deviceId + "\", \"deviceSecret\":\"" + HARDWARE_SECRET + "\", \"name\":\"IoTmate Relay Node\"}";

    Serial.print("[HTTP] Sending JSON: ");
    Serial.println(jsonBody);

    int httpCode = http.POST(jsonBody);
    MqttCredentials creds = {"", ""};

    if (httpCode == HTTP_CODE_OK) // 200 OK
    {
        String response = http.getString();
        Serial.print("[HTTP] Response from Backend: ");
        Serial.println(response);

        StaticJsonDocument<512> doc;
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
        clearCredentials();
        delay(5000);
    }
    else if (httpCode == 409) // 409 Conflict
    {
        Serial.println("[PROVISION] ERROR 409: Device conflict or invalid credentials. Clearing local credentials.");
        clearCredentials();
        delay(5000);
    }
    else
    {
        Serial.printf("[HTTP] Request failed, HTTP code: %d\n", httpCode);
        delay(5000);
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