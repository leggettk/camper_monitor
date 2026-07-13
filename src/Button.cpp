#include "Button.h"
#include "Pins.h"

Button userButton;

void Button::begin()
{
    pinMode(PIN_USER_BUTTON, INPUT_PULLUP);

    stableState = digitalRead(PIN_USER_BUTTON);
    previousReading = stableState;
}

void Button::update()
{
    const bool reading = digitalRead(PIN_USER_BUTTON);
    const unsigned long now = millis();

    if (reading != previousReading)
    {
        previousReading = reading;
        lastChangeTime = now;
    }

    if ((now - lastChangeTime) < DEBOUNCE_MS)
    {
        return;
    }

    if (reading != stableState)
    {
        stableState = reading;

        if (stableState == LOW)
        {
            pressStartTime = now;
            longPressReported = false;
        }
        else
        {
            if (!longPressReported)
            {
                shortPressEvent = true;
            }
        }
    }

    if (
        stableState == LOW &&
        !longPressReported &&
        (now - pressStartTime >= LONG_PRESS_MS))
    {
        longPressReported = true;
        longPressEvent = true;
    }
}

bool Button::wasPressed()
{
    if (!shortPressEvent)
    {
        return false;
    }

    shortPressEvent = false;
    return true;
}

bool Button::wasLongPressed()
{
    if (!longPressEvent)
    {
        return false;
    }

    longPressEvent = false;
    return true;
}