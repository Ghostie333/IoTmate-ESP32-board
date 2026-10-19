#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>
#include "config.h"

class RelayManager
{
public:
    static constexpr uint8_t RelayCount = NUM_OF_OUTLETS;

    RelayManager() = default;

    void begin();
    void setRelay(uint8_t relayNum, bool state);
    String getStatesAsJson() const;

private:
    const uint8_t _relayPins[RelayCount] = {
        RELAY_1_PIN,
        RELAY_2_PIN,
        RELAY_3_PIN,
        RELAY_4_PIN};

    bool _relaysState[RelayCount] = {};

    void updateHardwarePins();
};

#endif
