#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Unique device identification
extern String DEVICE_ID;

// Backend server
#define BACKEND_SERVER "192.168.1.69"
#define BACKEND_PORT 7070

// MQTT Broker settings (Update with your Docker host IP)
#define MQTT_SERVER "192.168.1.69"
#define MQTT_PORT 1883
#define MQTT_USER "backend_app"
#define MQTT_PASS "BackendPassword"

// Hardware Configuration (ESP32-C3 Super Mini Pinout)
#define RESET_PIN 0 // GPIO0 - Reset wifi button

#define RELAY_1_PIN 1 // GPIO1 - Relay 1
#define RELAY_2_PIN 2 // GPIO2 - Relay 2
#define RELAY_3_PIN 3 // GPIO3 - Relay 3
#define RELAY_4_PIN 4 // GPIO4 - Relay 4

// Timers
#define HEARTBEAT_INTERVAL_MS 10000 // Send state every 10s

#endif