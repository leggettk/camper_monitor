#pragma once

#include <Arduino.h>

// -----------------------------------------------------------------------------
// Firmware
// -----------------------------------------------------------------------------

#define FW_NAME        "Camper Monitor"
#define FW_VERSION     "0.2.0"

constexpr uint8_t SCREEN_WIDTH  = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr char BUILD_DATE[] = __DATE__;
constexpr char BUILD_TIME[] = __TIME__;
constexpr uint8_t   OLED_ADDRESS = 0x3C;
