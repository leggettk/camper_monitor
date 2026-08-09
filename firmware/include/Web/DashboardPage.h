#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "AppState.h"
#include "WebPageBuilder.h"

class DashboardPage
{
public:
    void begin(
        WebServer& server,
        AppState& state);

    void handle();

private:
    WebServer* server_ = nullptr;
    AppState* state_ = nullptr;

    String formatTemperature() const;
    String formatBatteryVoltage() const;
    String formatUptime(
        uint32_t totalSeconds) const;
    String formatFreeHeap(
        uint32_t bytes) const;

    StatusLevel determineOverallStatus(
        String& title,
        String& message) const;
};