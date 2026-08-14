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

    // Connects to a saved WiFi using raw WiFi API (bypasses WiFiManager's own
    // saved-credential loop, so we control the retry/erase behavior ourselves).
    // Returns true when the STA link is established.
    bool connectToSavedWiFi(const String &ssid, const String &pass);

    // Tries one network connection attempt and waits up to a timeout.
    bool tryConnectOnce(const String &ssid, const String &pass, unsigned long timeoutMs);

    // Starts the config portal in AP mode and blocks in a loop: if it times
    // out without a successful connection, it simply restarts the portal
    // instead of rebooting the chip (the AP stays up).
    void startConfigPortalLoop();

public:
    CustomWiFiManager() = default;

    void setupWiFi();
};

#endif // WIFI_MANAGER_H