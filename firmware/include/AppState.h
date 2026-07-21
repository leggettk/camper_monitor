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

    String operatorName = "Unknown";
    String ipAddress = "No IP";

    unsigned long uptimeSeconds = 0;
    uint32_t freeHeap = 0;
    
    float batteryVoltage = 0.0f;
    bool batteryVoltageValid = false;
    bool shorePowerPresent = false;
    bool highTemperatureAlarm = false;
    bool shorePowerAlarm = false;

    bool lastSmsSuccessful = false;
    unsigned long lastSmsTimeSeconds = 0;
};