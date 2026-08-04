#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>
#include "config.h"

class RelayManager
{
private:
    static const uint8_t NUM_RELAYS = 4;

    // Tablica mapująca indeks pętli na konkretne piny z config.h
    const uint8_t _relayPins[NUM_RELAYS] = {
        RELAY_1_PIN,
        RELAY_2_PIN,
        RELAY_3_PIN,
        RELAY_4_PIN};

    // Stany dla 4 przekaźników (domyślnie false)
    bool _relaysState[NUM_RELAYS] = {false, false, false, false};

    void updateHardwarePins();

public:
    RelayManager() = default;

    void begin();

    // Oczekuje numeru przekaźnika od 1 do 4 (1..NUM_RELAYS)
    void setRelay(uint8_t relayNum, bool state);
    bool getRelayState(uint8_t relayNum) const;

    String getStatesAsJson() const;
};

#endif // RELAY_MANAGER_H