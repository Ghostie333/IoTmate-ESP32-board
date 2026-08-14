#include "wifi_manager.h"
#include <WiFiManager.h>
#include <WiFi.h>
#include <Arduino.h>
#include <esp_system.h>

void CustomWiFiManager::setupWiFi()
{
    // Set AP+STA mode and turn off modem sleep
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);

    // Max wifi power
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    checkResetButtonOnBoot();

    _wm.setCaptivePortalEnable(true);
    _wm.setConnectTimeout(10);
    _wm.setSaveConnectTimeout(10);
    // 0 = no automatic timeout. The portal never closes by itself; see
    // startConfigPortalLoop() which keeps it alive after a manual exit.
    _wm.setConfigPortalTimeout(0);

    // ------------------------------------------------------------------
    // 1) NO SAVED CREDENTIALS -> open the config portal AP immediately.
    // ------------------------------------------------------------------
    String savedSSID = _wm.getWiFiSSID();
    if (savedSSID.length() == 0)
    {
        Serial.println("[WiFi] No saved credentials found, opening config portal...");
        startConfigPortalLoop();
        return; // Does not return until a network is configured and connected.
    }

    Serial.println("[WiFi] Saved credentials found for: " + savedSSID);

    // ------------------------------------------------------------------
    // 2) CREDENTIALS ARE SAVED -> try to connect (a few attempts).
    // ------------------------------------------------------------------
    if (connectToSavedWiFi(savedSSID, _wm.getWiFiPass()))
    {
        Serial.print("[WiFi] Connected, IP: ");
        Serial.println(WiFi.localIP());
        return;
    }

    // ------------------------------------------------------------------
    // 3) COULD NOT CONNECT -> erase saved credentials and open the portal.
    // ------------------------------------------------------------------
    Serial.println("[WiFi] Could not connect to saved WiFi after multiple attempts.");
    Serial.println("[WiFi] Erasing saved credentials and opening config portal...");
    _wm.resetSettings(); // Erases the saved SSID/password permanently

    // Give the erasure a moment before we reconfigure the radio into AP mode
    delay(500);
    WiFi.mode(WIFI_AP_STA);

    startConfigPortalLoop();
}

bool CustomWiFiManager::connectToSavedWiFi(const String &ssid, const String &pass)
{
    for (uint8_t attempt = 1; attempt <= WIFI_CONNECT_ATTEMPTS; attempt++)
    {
        Serial.printf("[WiFi] Connecting to %s (attempt %u/%u)...\n",
                      ssid.c_str(), attempt, WIFI_CONNECT_ATTEMPTS);

        if (tryConnectOnce(ssid, pass, WIFI_CONNECT_TIMEOUT_MS))
        {
            return true;
        }

        // Stop the in-flight attempt before retrying (avoids "sta is connecting" 0x3007)
        WiFi.disconnect(false, false);
        delay(200);
    }

    return false;
}

bool CustomWiFiManager::tryConnectOnce(const String &ssid, const String &pass, unsigned long timeoutMs)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs)
    {
        delay(500);
    }

    return WiFi.status() == WL_CONNECTED;
}

void CustomWiFiManager::startConfigPortalLoop()
{
    // Print the AP BSSID/MAC so the exact radio that owns this AP can be
    // confirmed from a phone's WiFi details page (our board's MAC is
    // e8:f6:0a:36:65:f0 - any AP showing a different last byte is another ESP).
    // Loop instead of rebooting: if the portal is closed without a successful
    // connection (timeout/manual exit), simply restart it so the AP stays up.
    while (true)
    {
        WiFi.softAPdisconnect(true);
        bool connected = _wm.startConfigPortal(WIFI_PORTAL_SSID);

        if (connected)
        {
            Serial.print("[WiFi] Connected, IP: ");
            Serial.println(WiFi.localIP());
            return;
        }

        Serial.println("[WiFi] Config portal closed without connecting. Restarting portal...");
        delay(500);
        // Ensure the radio is in AP mode for the next portal attempt.
        WiFi.mode(WIFI_AP_STA);
    }
}

void CustomWiFiManager::checkResetButtonOnBoot()
{
    // GPIO0 has no external pull-up on this board; enable internal pull-up and
    // debounce so noise (AP/STA radio powering up) cannot fake a button press
    // and trigger _wm.resetSettings() at boot.
    pinMode(RESET_PIN, INPUT_PULLUP);

    // Wait for line to settle after powering on
    delay(100);

    // If RESET_PIN is held low for >= 80ms it is a real press
    bool held = (digitalRead(RESET_PIN) == LOW);
    if (held)
    {
        delay(80);
        held = (digitalRead(RESET_PIN) == LOW);
    }

    if (held)
    {
        Serial.println("\n[SYSTEM] RESET_BUTTON LONG PRESS detected");
        Serial.println("[SYSTEM] Cleaning Wi-Fi settings...");

        // Reset wifi settings
        _wm.resetSettings();

        // Wait for user to stop pressing the button
        while (digitalRead(RESET_PIN) == LOW)
        {
            delay(50);
        }

        Serial.println("[SYSTEM] Settings cleared. Launching Captive Portal...");
    }
}