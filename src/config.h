#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "device_type.h"

constexpr DeviceType DEVICE_TYPE = DeviceType::PowerStrip;
#define NUM_OF_OUTLETS 4

#define MQTT_SERVER "192.168.0.118"
#define MQTT_PORT 8883

#ifdef BACKEND_TLS
#define BACKEND_SERVER "192.168.0.118"
#define BACKEND_PORT 443
#define BACKEND_SCHEME "https"
#else
#define BACKEND_SERVER "192.168.0.118"
#define BACKEND_PORT 7070
#define BACKEND_SCHEME "http"
#endif

#define RESET_PIN 0

#define RELAY_1_PIN 1
#define RELAY_2_PIN 2
#define RELAY_3_PIN 3
#define RELAY_4_PIN 4

#define WIFI_CONNECT_ATTEMPTS 4
#define WIFI_CONNECT_TIMEOUT_MS 10000
#define WIFI_PORTAL_SSID "IoTmate-Board"
#define WIFI_PORTAL_PASSWORD_PREFIX "iotmate-"
#define WIFI_RECONNECT_INTERVAL_MS 30000

#define HEARTBEAT_INTERVAL_MS 2000
#define MQTT_RETRY_DELAY_MS 5000
#define MQTT_RETRY_MAX_DELAY_MS 60000
#define MQTT_KEEPALIVE_SEC 15
#define MQTT_SOCKET_TIMEOUT_SEC 10
#define HTTP_TIMEOUT_MS 3000

#endif
