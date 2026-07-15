#pragma once

#include <Arduino.h>

class SMS
{
public:
    bool send(const String &message);

    bool sendShorePowerLost(
        float batteryVoltage,
        bool batteryValid);

    bool sendShorePowerRestored();

    bool sendHighTemperature(
        float temperatureF,
        float setpointF);

    bool sendTemperatureNormal(
        float temperatureF);

private:
    bool sending = false;
};

extern SMS sms;