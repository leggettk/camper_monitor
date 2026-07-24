#pragma once
#include <Arduino.h>
//
// Camper Sentinel
//

constexpr char PROJECT_NAME[] = "Camper Sentinel";
constexpr char PROJECT_VERSION[] = "1.0.0";

//
// Timing (milliseconds)
//

constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS  = 30000;
constexpr uint32_t MQTT_RECONNECT_INTERVAL_MS  = 15000;
constexpr uint32_t LTE_RECONNECT_INTERVAL_MS   = 60000;

//
// Default timing (seconds)
//

constexpr uint32_t DEFAULT_HEARTBEAT_INTERVAL_SECONDS = 300;
constexpr uint32_t DEFAULT_TEMPERATURE_PUBLISH_SECONDS = 180;

//
// Temperature Alarm Defaults (°F)
//

constexpr float DEFAULT_TEMP_ALARM_HIGH_F  = 90.0f;
constexpr float DEFAULT_TEMP_ALARM_RESET_F = 85.0f;