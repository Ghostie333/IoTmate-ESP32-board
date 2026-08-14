#include "relay_manager.h"
#include "config.h"
#include <ArduinoJson.h>

void RelayManager::begin()
{
    // Configure pin for Wi-Fi/factory reset
    pinMode(RESET_PIN, INPUT_PULLUP);

    // Initialize relay control pins as outputs
    for (uint8_t i = 0; i < NUM_RELAYS; i++)
    {
        pinMode(_relayPins[i], OUTPUT);
    }

    // Apply default hardware pin states
    updateHardwarePins();
}

void RelayManager::setRelay(uint8_t relayNum, bool state)
{
    // Validate relay number range (1..NUM_RELAYS)
    if (relayNum < 1 || relayNum > NUM_RELAYS)
        return;

    _relaysState[relayNum - 1] = state;
    updateHardwarePins();
}

bool RelayManager::getRelayState(uint8_t relayNum) const
{
    // Validate relay number range (1..NUM_RELAYS)
    if (relayNum < 1 || relayNum > NUM_RELAYS)
        return false;

    return _relaysState[relayNum - 1];
}

void RelayManager::updateHardwarePins()
{
    // Relay modules are active-low: LOW = relay ON, HIGH = relay OFF.
    // The logical state (_relaysState) is kept unchanged so the JSON sent to the
    // backend matches the physical relay position.
    for (uint8_t i = 0; i < NUM_RELAYS; i++)
    {
        digitalWrite(_relayPins[i], _relaysState[i] ? LOW : HIGH);
    }
}

String RelayManager::getStatesAsJson() const
{
    // Build JSON dynamically: {"outlet1":false,"outlet2":false,...} matching the backend format
    JsonDocument doc;
    for (uint8_t i = 0; i < NUM_RELAYS; i++)
    {
        String key = "outlet" + String(i + 1);
        doc[key] = _relaysState[i];
    }

    String jsonOutput;
    serializeJson(doc, jsonOutput);
    return jsonOutput;
}