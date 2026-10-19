#include "relay_manager.h"
#include "config.h"
#include <ArduinoJson.h>

void RelayManager::begin()
{
    pinMode(RESET_PIN, INPUT_PULLUP);

    for (uint8_t i = 0; i < RelayCount; i++)
    {
        pinMode(_relayPins[i], OUTPUT);
    }

    updateHardwarePins();
}

void RelayManager::setRelay(uint8_t relayNum, bool state)
{
    if (relayNum < 1 || relayNum > RelayCount)
        return;

    _relaysState[relayNum - 1] = state;
    updateHardwarePins();
}

void RelayManager::updateHardwarePins()
{
    for (uint8_t i = 0; i < RelayCount; i++)
    {
        digitalWrite(_relayPins[i], _relaysState[i] ? LOW : HIGH);
    }
}

String RelayManager::getStatesAsJson() const
{
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    for (uint8_t i = 0; i < RelayCount; i++)
    {
        array.add(_relaysState[i]);
    }

    String jsonOutput;
    serializeJson(doc, jsonOutput);
    return jsonOutput;
}
