#pragma once
#include "AppState.h"
class MQTT
{
public:
    bool begin();
    void loop();

    bool isConnected() const;
    bool publishAmbient(float temperatureF);
    bool publishBattery(float voltage);
    bool publishShorePower(bool present);
    bool publishHeartbeat(const AppState &state);

private:
    bool connect();
    bool publishDiscovery();
    bool discoveryPublished = false;
    unsigned long lastReconnectAttemptMs = 0;

    static constexpr unsigned long RECONNECT_INTERVAL_MS = 30000UL;
};

extern MQTT mqtt;