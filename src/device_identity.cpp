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
    // 1. Generate unique DEVICE_ID from Wi-Fi STA MAC address
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    char macStr[13];
    snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    DEVICE_ID = String(macStr);

    // 2. Try to read 32-byte custom secret from eFuse BLOCK3 (BLOCK_USR_DATA / EFUSE_BLK3)
    uint8_t efuseKey[32] = {0};

    // Read raw 256 bits from BLOCK3
    esp_err_t err = esp_efuse_read_block(EFUSE_BLK3, efuseKey, 0, 256);

    bool isEfuseValid = (err == ESP_OK);
    if (isEfuseValid)
    {
        // Verify that eFuse is actually programmed (not all 0x00 or all 0xFF)
        bool allZero = true;
        bool allFF = true;
        for (int i = 0; i < 32; i++)
        {
            if (efuseKey[i] != 0x00)
                allZero = false;
            if (efuseKey[i] != 0xFF)
                allFF = false;
        }
        if (allZero || allFF)
        {
            isEfuseValid = false;
        }
    }

    if (isEfuseValid)
    {
        // Convert 32 raw binary bytes into a 64-character lowercase hex string
        char hexSecret[65];
        for (int i = 0; i < 32; i++)
        {
            snprintf(&hexSecret[i * 2], 3, "%02x", efuseKey[i]);
        }
        hexSecret[64] = '\0';
        HARDWARE_SECRET = String(hexSecret);
        Serial.println("[SECURITY] Hardware secret successfully read from eFuse (BLOCK3).");
    }
    else
    {
        // FALLBACK: eFuse is unprogrammed or unreadable. Generate fallback secret using MAC + static salt.
        char secretStr[33];
        snprintf(secretStr, sizeof(secretStr), "%02x%02x%02x%02x%02x%02xa1b2c3d4e5f67890",
                 mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
        HARDWARE_SECRET = String(secretStr);
        Serial.println("[SECURITY] WARNING: eFuse BLOCK3 unprogrammed. Used MAC fallback secret.");
    }
}