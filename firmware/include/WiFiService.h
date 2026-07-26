#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "Logger.h"
#include "Settings.h"

class WiFiService
{
public:
    // Initializes Wi-Fi station mode and begins a connection attempt
    // using the credentials stored in Settings.
    bool begin(Settings& settings, Logger& logger);

    // Handles non-blocking reconnect attempts.
    void update();

    // Stops Wi-Fi and clears the current connection.
    void disconnect();

    // Starts a new connection attempt using the current settings.
    void reconnect();

    bool connected() const;
    bool configured() const;
    bool enabled() const;

    String ipAddress() const;
    String ssid() const;
    String hostname() const;

    int32_t rssi() const;

private:

    Logger* logger_ = nullptr;
    Settings* settings_ = nullptr;
    bool wasConnected_ = false;
    bool started_ = false;
    bool connectionAttempted_ = false;

    uint32_t lastReconnectAttemptMs_ = 0;

    void connect();
};