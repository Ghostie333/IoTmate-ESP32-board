#include "device_identity.h"
#include <WiFi.h>
#include <esp_efuse.h>

String DEVICE_ID = "";
String HARDWARE_SECRET = "";

void generateDeviceId()
{
    uint8_t mac[6];
    WiFi.macAddress(mac);

    char idBuffer[13];
    snprintf(idBuffer, sizeof(idBuffer), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    DEVICE_ID = String(idBuffer);
}

void generateHardwareUniqueId()
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    char macStr[13];
    snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    DEVICE_ID = String(macStr);

    // Dynamic ID fallback when eFuse is unprogrammed
    char secretStr[33];
    snprintf(secretStr, sizeof(secretStr), "%02x%02x%02x%02x%02x%02xa1b2c3d4e5f67890",
             mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    HARDWARE_SECRET = String(secretStr);
}