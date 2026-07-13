#pragma once

#include <Arduino.h>

struct AppState
{
    float ambientTemperatureF = 0.0f;

    bool networkConnected = false;
    bool dataConnected = false;
    bool mqttConnected = false;

    int signalQuality = 99;

    String operatorName = "";
    String ipAddress = "";

    unsigned long uptimeSeconds = 0;
};