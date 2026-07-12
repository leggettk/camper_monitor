#include <Arduino.h>

#include "MQTT.h"

MQTT mqtt;

bool MQTT::begin()
{
    Serial.println("MQTT TLS not configured yet.");
    return false;
}

void MQTT::loop()
{
}

bool MQTT::publishAmbient(float temperatureF)
{
    (void)temperatureF;
    return false;
}