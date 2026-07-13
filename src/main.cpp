#include <Arduino.h>

#include "Button.h"
#include "Display.h"
#include "Modem.h"
#include "MQTT.h"
#include "Temperature.h"

Temperature temp;

void setup()
{
    Serial.begin(115200);
    delay(500);

    if (!oled.begin())
    {
        Serial.println("OLED failed!");
    }

    userButton.begin();

    if (!temp.begin())
    {
        Serial.println("Temperature sensor not found!");
    }

    oled.bootScreen();
    delay(1500);

    if (cellular.begin())
    {
        Serial.println("Modem OK");
    }
    else
    {
        Serial.println("Modem FAILED");
    }

    if (mqtt.begin())
    {
        Serial.println("MQTT OK");
    }
    else
    {
        Serial.println("MQTT unavailable");
    }
}

void loop()
{
    mqtt.loop();
    userButton.update();

    static unsigned long lastTemperatureRead = 0;
    static unsigned long lastPublish = 0;

    static float ambientTemperatureF = 0.0f;

    const unsigned long now = millis();

    if (userButton.wasPressed())
    {
        if (oled.isAwake())
        {
            oled.nextPage();
        }
        else
        {
            oled.wake();
        }

        Serial.println("Display button pressed.");
    }

    if (userButton.wasLongPressed())
    {
        Serial.println("Long press detected.");
        Serial.println("MQTT reconnect will be added later.");

        oled.status(
            "Camper Monitor",
            "MQTT reconnect",
            "Not available yet");
    }

    if (now - lastTemperatureRead >= 5000)
    {
        lastTemperatureRead = now;

        ambientTemperatureF =
            temp.getFahrenheit();

        Serial.printf(
            "Ambient: %.1f F\n",
            ambientTemperatureF);
    }

    DisplayData displayData;

    displayData.ambientTemperatureF =
        ambientTemperatureF;

    displayData.networkConnected =
        cellular.isNetworkConnected();

    displayData.dataConnected =
        cellular.isDataConnected();

    displayData.mqttConnected =
        mqtt.isConnected();

    displayData.operatorName =
        cellular.getOperatorName();

    displayData.signalQuality =
        cellular.getSignalQuality();

    displayData.ipAddress =
        cellular.getIpAddress();

    displayData.uptimeSeconds =
        millis() / 1000UL;

    oled.update(displayData);

    if (now - lastPublish >= 30000)
    {
        lastPublish = now;

        if (mqtt.publishAmbient(
                ambientTemperatureF))
        {
            Serial.println(
                "Published temperature");
        }
        else
        {
            Serial.println(
                "Temperature publish failed");
        }
    }
}