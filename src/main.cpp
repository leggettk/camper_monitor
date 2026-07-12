#include <Arduino.h>

#include "Display.h"
#include "Temperature.h"
#include "Modem.h"
#include "MQTT.h"

Temperature temp;

void setup()
{
    Serial.begin(115200);
    delay(500);

    if (!oled.begin())
    {
        Serial.println("OLED failed!");
    }

    if (!temp.begin())
    {
        Serial.println("Temperature sensor not found!");
    }

    oled.bootScreen();
    delay(1500);

    if (!cellular.begin())
    {
        Serial.println("Modem FAILED");
        return;
    }

    Serial.println("Modem OK");

    if (mqtt.begin())
    {
        Serial.println("MQTT OK");
    }
    else
    {
        Serial.println("MQTT FAILED");
    }
}

void loop()
{
    mqtt.loop();

    static unsigned long lastDisplay = 0;
    static unsigned long lastPublish = 0;

    const unsigned long now = millis();

    if (now - lastDisplay >= 5000)
    {
        lastDisplay = now;

        const float temperatureF = temp.getFahrenheit();

        Serial.printf("Ambient: %.1f F\n", temperatureF);

        oled.status(
            "Camper Monitor",
            "Ambient",
            String(temperatureF, 1) + " F");
    }

    if (now - lastPublish >= 30000)
    {
        lastPublish = now;

        const float temperatureF = temp.getFahrenheit();

        if (mqtt.publishAmbient(temperatureF))
        {
            Serial.println("Published temperature");
        }
        else
        {
            Serial.println("Temperature publish failed");
        }
    }
}