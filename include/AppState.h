#pragma once

#include <Arduino.h>

struct AppState
{
    float ambientTemperatureF = 0.0f;
    bool temperatureValid = false;

    bool networkConnected = false;
    bool dataConnected = false;
    bool mqttConnected = false;

    int signalQuality = 99;

    String operatorName = "";
    String ipAddress = "";

    unsigned long uptimeSeconds = 0;
    uint32_t freeHeap;
    
    float batteryVoltage = 0.0f;
    bool batteryVoltageValid = false;
};