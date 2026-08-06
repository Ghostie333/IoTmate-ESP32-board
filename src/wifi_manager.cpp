#include "wifi_manager.h"
#include <WiFiManager.h>
#include <WiFi.h>
#include <Arduino.h>

void CustomWiFiManager::setupWiFi()
{
    // Reset Wifi module
    WiFi.disconnect(true, true);
    delay(100);

    // Set AP+STA mode and turn off modem sleep
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);

    // Max wifi power
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    checkResetButtonOnBoot();

    _wm.setCaptivePortalEnable(true);
    _wm.setConnectTimeout(10);
    _wm.setSaveConnectTimeout(10);
    _wm.setConfigPortalTimeout(60);

    Serial.println("[WiFi] Launching autoconnect...");

    bool connected = _wm.autoConnect("IoTmate-Board");

    if (!connected)
    {
        Serial.println("[WiFi] Failed to connect to Wifi and AP time's out. Restarting...");
        delay(1000);
        ESP.restart();
    }

    Serial.print("[WiFi] Connected, IP: ");
    Serial.println(WiFi.localIP());
}

void CustomWiFiManager::checkResetButtonOnBoot()
{
    // Wait after turning on device
    delay(100);

    // If RESET_BUTTON is pressed
    if (digitalRead(RESET_PIN) == LOW)
    {
        Serial.println("\n[SYSTEM] RESET_BUTTON clicked");
        Serial.println("[SYSTEM] Cleaning Wi-Fi settings...");

        // Reset wifi settings
        _wm.resetSettings();

        // Wait for user to stop pressing button
        while (digitalRead(RESET_PIN) == LOW)
        {
            delay(50);
        }

        Serial.println("[SYSTEM] Settings cleared. Launching Captive Portal...");
    }
}
