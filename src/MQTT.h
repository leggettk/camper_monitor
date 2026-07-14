#pragma once

class MQTT
{
public:
    bool begin();
    void loop();

    bool isConnected() const;
    bool publishAmbient(float temperatureF);

private:
    bool connect();
    unsigned long lastReconnectAttemptMs = 0;

    static constexpr unsigned long RECONNECT_INTERVAL_MS = 30000;
};

extern MQTT mqtt;