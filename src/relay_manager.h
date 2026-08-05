#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>
#include "config.h"

class RelayManager
{
private:
    // Use NUM_OF_OUTLETS defined in config.h
    static const uint8_t NUM_RELAYS = NUM_OF_OUTLETS;

    // Mapping loop index to specific GPIO pins from config.h
    const uint8_t _relayPins[NUM_RELAYS] = {
        RELAY_1_PIN,
        RELAY_2_PIN,
        RELAY_3_PIN,
        RELAY_4_PIN};

    // Relay states (defaults to false)
    bool _relaysState[NUM_RELAYS] = {false, false, false, false};

    void updateHardwarePins();

public:
    RelayManager() = default;

    void begin();

    // Expects relay index from 1 to NUM_OF_OUTLETS (1..NUM_RELAYS)
    void setRelay(uint8_t relayNum, bool state);
    bool getRelayState(uint8_t relayNum) const;

    String getStatesAsJson() const;
};

#endif // RELAY_MANAGER_H