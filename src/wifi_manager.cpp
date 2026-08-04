#include "wifi_manager.h"
#include <WiFiManager.h>
#include <WiFi.h>
#include <Arduino.h>

void CustomWiFiManager::setupWiFi()
{
    // 1. Zresetuj moduł Wi-Fi
    WiFi.disconnect(true, true);
    delay(100);

    // 2. Ustaw tryb AP+STA i wyłącz uśpienie modemu (KLUCZOWE DLA C3)
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);

    // 3. Opcjonalnie: ustaw maksymalną moc nadawania TX (w dBm)
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    WiFiManager wm;

    wm.setCaptivePortalEnable(true);
    wm.setConnectTimeout(10);
    wm.setSaveConnectTimeout(10);
    wm.setConfigPortalTimeout(60); // Daj sobie 3 minuty na testy

    Serial.println("[WiFi] Uruchamianie autoconnect...");

    bool connected = wm.autoConnect("IoTmate-Setup-AP");

    if (!connected)
    {
        Serial.println("[WiFi] Nie połączono z siecią i upłynął czas AP. Restart...");
        delay(1000);
        ESP.restart();
    }

    Serial.print("[WiFi] Połączono! IP: ");
    Serial.println(WiFi.localIP());
}