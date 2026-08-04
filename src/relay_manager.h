#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>

class RelayManager
{
public:
    void begin();
    void setRelay(uint8_t relayNum, bool state);
    bool getRelayState(uint8_t relayNum) const;
    String getStatesAsJson() const;

private:
    bool _relay1State = false;
    bool _relay2State = false;
    void updateHardwarePins();
};

#endif