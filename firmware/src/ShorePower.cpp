#include <Arduino.h>

#include "Pins.h"
#include "ShorePower.h"

ShorePower shorePower;

void ShorePower::begin()
{
    pinMode(PIN_USB_POWER_SENSE, INPUT);

    present = digitalRead(PIN_USB_POWER_SENSE) == HIGH;
    previousPresent = present;
}

void ShorePower::update()
{
    const bool rawPresent =
        digitalRead(PIN_USB_POWER_SENSE) == HIGH;

    const unsigned long now = millis();

    if (rawPresent != previousPresent)
    {
        previousPresent = rawPresent;
        lastRawChangeMs = now;
    }

    if (
        rawPresent != present &&
        now - lastRawChangeMs >= DEBOUNCE_MS)
    {
        present = rawPresent;
        changeEvent = true;
    }
}

bool ShorePower::isPresent() const
{
    return present;
}

bool ShorePower::changed()
{
    if (!changeEvent)
    {
        return false;
    }

    changeEvent = false;
    return true;
}