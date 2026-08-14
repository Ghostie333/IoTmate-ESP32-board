#include <Arduino.h>
#include "wifi_manager.h"
#include "relay_manager.h"
#include "mqtt_manager.h"
#include "device_identity.h"
#include <esp_system.h>

CustomWiFiManager wifiMgr;
RelayManager relayMgr;
MqttManager mqttMgr(relayMgr);

void setup()
{
  // Initialize Serial Debug Monitor
  Serial.begin(115200);
  delay(3000); // allow the USB CDC to enumerate before logging
  Serial.println("\n=== IoTmate XIAO ESP32-C3 Firmware Starting ===");

  // Show why the chip booted (0=power-on, 5=DEEPSLEEP_RESET, 2=BROWNOUT...).
  // Brownout/watchdog resets in the middle of the portal would reboot into the
  // same firmware though - they still cannot produce a different AP SSID.
  Serial.printf("[SYSTEM] Reset reason: %d\n", esp_reset_reason());

  // Generate global variables with unique IDs
  generateDeviceId();
  generateHardwareUniqueId();

  // DEVICE_ID == MAC
  Serial.print("Device ID (MAC): ");
  Serial.println(DEVICE_ID);

  Serial.print("Hardware Secret: ");
  Serial.println(HARDWARE_SECRET);

  // Initialize Relays and GPIOs
  relayMgr.begin();

  // Connect to Wi-Fi via Captive Portal
  wifiMgr.setupWiFi();

  // Initialize MQTT Client
  mqttMgr.begin();
}

void loop()
{
  // Process background MQTT tasks and handle reconnections
  mqttMgr.loop();
}