#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Unique device identification
#define DEVICE_ID "XIAO_ESP32C3_POWERSTRIP_01"

// MQTT Broker settings (Update with your Docker host IP)
#define MQTT_SERVER "192.168.1.69"
#define MQTT_PORT 1883
#define MQTT_USER "backend_app"
#define MQTT_PASS "BackendPassword"

// MQTT Topics
#define TOPIC_STATUS "devices/" DEVICE_ID "/status"
#define TOPIC_STATE "devices/" DEVICE_ID "/state"
#define TOPIC_COMMAND "devices/" DEVICE_ID "/command"

// Hardware Configuration (XIAO ESP32-C3 Pinout)
#define RELAY_1_PIN 2 // D0 / GPIO2
#define RELAY_2_PIN 3 // D1 / GPIO3

// Timers
#define HEARTBEAT_INTERVAL_MS 30000 // Send state every 30s

#endif