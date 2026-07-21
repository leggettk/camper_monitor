#include <Arduino.h>

#include "Battery.h"
#include "Pins.h"

Battery battery;

void Battery::begin()
{
    pinMode(PIN_BATTERY_VOLTAGE, INPUT);

    analogReadResolution(12);
    analogSetPinAttenuation(
        PIN_BATTERY_VOLTAGE,
        ADC_11db);
}

void Battery::update()
{
    constexpr uint8_t SAMPLE_COUNT = 32;

    uint32_t totalMillivolts = 0;

    for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
    {
        totalMillivolts +=
            analogReadMilliVolts(
                PIN_BATTERY_VOLTAGE);

        delay(2);
    }

    const float pinVoltage =
        (totalMillivolts / SAMPLE_COUNT) /
        1000.0f;

    const float dividerRatio =
        (R1_OHMS + R2_OHMS) / R2_OHMS;

    batteryVoltage =
        pinVoltage *
        dividerRatio *
        CALIBRATION_FACTOR;

    valid =
        batteryVoltage >= 0.0f &&
        batteryVoltage <= 18.0f;
}

float Battery::voltage() const
{
    return batteryVoltage;
}

bool Battery::isValid() const
{
    return valid;
}