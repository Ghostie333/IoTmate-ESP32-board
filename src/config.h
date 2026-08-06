#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "device_type.h"
// Unique device identification
extern String DEVICE_ID;

// Device information for backend
constexpr DeviceType DEVICE_TYPE = DeviceType::PowerStrip;
#define NUM_OF_OUTLETS 4

// Backend server and MQTT
// MQTT_TLS: use TLS for the MQTT broker (port 8883). Without it, plaintext 1883.
// BACKEND_TLS: use HTTPS for the backend (port 443). Without it, plaintext HTTP 7070.
// MQTT_DEV_TLS: dev only - accept self-signed broker certs (setInsecure) instead of verifying via ROOT_CA.
#ifdef MQTT_TLS
#define MQTT_SERVER "192.168.1.69"
#define MQTT_PORT 8883
#else
#define MQTT_SERVER "192.168.1.69"
#define MQTT_PORT 1883
#endif

#ifdef BACKEND_TLS
#define BACKEND_SERVER "192.168.1.69"
#define BACKEND_PORT 443
#define BACKEND_SCHEME "https"
#else
#define BACKEND_SERVER "192.168.1.69"
#define BACKEND_PORT 7070
#define BACKEND_SCHEME "http"
#endif

// Hardware Configuration (ESP32-C3 Super Mini Pinout)
#define RESET_PIN 0 // GPIO0 - Reset wifi button

#define RELAY_1_PIN 1 // GPIO1 - Relay 1
#define RELAY_2_PIN 2 // GPIO2 - Relay 2
#define RELAY_3_PIN 3 // GPIO3 - Relay 3
#define RELAY_4_PIN 4 // GPIO4 - Relay 4

// Timers
#define HEARTBEAT_INTERVAL_MS 10000 // Send state every 10s
#define MQTT_RETRY_DELAY_MS 5000    // Delay between connection attempts
#define MQTT_KEEPALIVE_SEC 15       // MQTT keepalive interval
#define MQTT_SOCKET_TIMEOUT_SEC 10  // Max blocking time for socket operations
#define HTTP_TIMEOUT_MS 3000        // Max blocking time for HTTP requests

#endif