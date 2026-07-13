#pragma once

class MQTT
{
public:
    bool begin();
    void loop();

    bool publishAmbient(float temperatureF);
    bool isConnected() const;
};

extern MQTT mqtt;