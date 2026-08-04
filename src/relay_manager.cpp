#include "relay_manager.h"
#include "config.h"

void RelayManager::begin()
{
    pinMode(RELAY_1_PIN, OUTPUT);
    pinMode(RELAY_2_PIN, OUTPUT);
    updateHardwarePins();
}

void RelayManager::setRelay(uint8_t relayNum, bool state)
{
    if (relayNum == 1)
    {
        _relay1State = state;
    }
    else if (relayNum == 2)
    {
        _relay2State = state;
    }
    updateHardwarePins();
}

bool RelayManager::getRelayState(uint8_t relayNum) const
{
    if (relayNum == 1)
        return _relay1State;
    if (relayNum == 2)
        return _relay2State;
    return false;
}

void RelayManager::updateHardwarePins()
{
    digitalWrite(RELAY_1_PIN, _relay1State ? HIGH : LOW);
    digitalWrite(RELAY_2_PIN, _relay2State ? HIGH : LOW);
}

String RelayManager::getStatesAsJson() const
{
    // Generates JSON format expected by IoTmate backend
    return String("{\"outlet1\":") + (_relay1State ? "true" : "false") +
           ",\"outlet2\":" + (_relay2State ? "true" : "false") + "}";
}