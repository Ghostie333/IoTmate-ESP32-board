#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFiManager.h>
#include "config.h"

class CustomWiFiManager
{
private:
    WiFiManager _wm;

    // Sprawdza stan przycisku podczas uruchamiania
    void checkResetButtonOnBoot();

public:
    CustomWiFiManager() = default;

    void setupWiFi();
};

#endif // WIFI_MANAGER_H