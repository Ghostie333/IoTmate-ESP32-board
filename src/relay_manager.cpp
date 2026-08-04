#include "relay_manager.h"
#include "config.h"

void RelayManager::begin()
{
    // Pin for Wifi reset
    pinMode(RESET_PIN, INPUT_PULLUP);

    // Relays pins
    for (uint8_t i = 0; i < NUM_RELAYS; i++)
    {
        pinMode(_relayPins[i], OUTPUT);
    }

    updateHardwarePins();
}

void RelayManager::setRelay(uint8_t relayNum, bool state)
{
    // Accept 1..NUM_RELAYS numbers
    if (relayNum < 1 || relayNum > NUM_RELAYS)
        return;

    _relaysState[relayNum - 1] = state;
    updateHardwarePins();
}

bool RelayManager::getRelayState(uint8_t relayNum) const
{
    if (relayNum < 1 || relayNum > NUM_RELAYS)
        return false;

    return _relaysState[relayNum - 1];
}

void RelayManager::updateHardwarePins()
{
    for (uint8_t i = 0; i < NUM_RELAYS; i++)
    {
        digitalWrite(_relayPins[i], _relaysState[i] ? HIGH : LOW);
    }
}

String RelayManager::getStatesAsJson() const
{
    // Building JSON: {"outlet1":false,"outlet2":false,"outlet3":false,"outlet4":false}
    String json = "{";
    for (uint8_t i = 0; i < NUM_RELAYS; i++)
    {
        json += "\"outlet" + String(i + 1) + "\":" + (_relaysState[i] ? "true" : "false");
        if (i < NUM_RELAYS - 1)
        {
            json += ",";
        }
    }
    json += "}";
    return json;
}