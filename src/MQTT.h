#pragma once

class MQTT
{
public:
    bool begin();
    void loop();
    bool publishAmbient(float temp);
};

extern MQTT mqtt;