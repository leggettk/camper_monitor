#pragma once

#include <Arduino.h>

// -----------------------------------------------------------------------------
// Firmware
// -----------------------------------------------------------------------------

#define FW_NAME        "Camper Monitor"
#define FW_VERSION     "0.4.0"

constexpr uint8_t SCREEN_WIDTH  = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr char BUILD_DATE[] = __DATE__;
constexpr char BUILD_TIME[] = __TIME__;
constexpr uint8_t   OLED_ADDRESS = 0x3C;
constexpr unsigned long TEMPERATURE_INTERVAL_MS = 5000UL;
constexpr unsigned long MQTT_PUBLISH_INTERVAL_MS = 180000UL; // 3 minutes
constexpr unsigned long HEARTBEAT_INTERVAL_MS = 300000UL;    // 5 minutes
// Temperature alarm
constexpr float TEMP_HIGH_ALARM_F = 85.0f;
constexpr float TEMP_HIGH_RESET_F = 80.0f;

// Temperature must remain high this long before alerting.
constexpr unsigned long TEMP_ALARM_DELAY_MS =
    1UL * 60UL * 1000UL;

// Optional reminder while alarm remains active.
// Set to 0 to disable reminders.
constexpr unsigned long SMS_REMINDER_INTERVAL_MS =
    0UL;