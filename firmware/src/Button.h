#pragma once

#include <Arduino.h>

class Button
{
public:
    void begin();
    void update();

    bool wasPressed();
    bool wasLongPressed();

private:
    bool stableState = HIGH;
    bool previousReading = HIGH;

    bool shortPressEvent = false;
    bool longPressEvent = false;
    bool longPressReported = false;

    unsigned long lastChangeTime = 0;
    unsigned long pressStartTime = 0;

    static constexpr unsigned long DEBOUNCE_MS = 40;
    static constexpr unsigned long LONG_PRESS_MS = 3000;
};

extern Button userButton;