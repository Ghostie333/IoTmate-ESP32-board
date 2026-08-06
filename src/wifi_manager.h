#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFiManager.h>
#include "config.h"

class CustomWiFiManager
{
private:
    WiFiManager _wm;

    // Checks the reset button state during boot
    void checkResetButtonOnBoot();

public:
    CustomWiFiManager() = default;

    void setupWiFi();
};

#endif // WIFI_MANAGER_H