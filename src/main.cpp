#include <Arduino.h>

#include "Button.h"
#include "Display.h"
#include "Modem.h"
#include "MQTT.h"
#include "Temperature.h"
#include "AppState.h"
#include "Logger.h"

AppState state;
Temperature temp;

void setup()
{
    logger.begin(115200, LogLevel::Info);
    delay(500);

    if (!oled.begin())
    {
        logger.error("OLED initialization failed!");
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
        logger.info("Modem OK");
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
        logger.warning("MQTT unavailable");
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

        logger.infof(
            "Ambient temperature: %.1f F\n",
            state.ambientTemperatureF);
    }

state.ambientTemperatureF = ambientTemperatureF;

state.networkConnected = cellular.isNetworkConnected();
state.dataConnected = cellular.isDataConnected();
state.mqttConnected = mqtt.isConnected();

state.signalQuality = cellular.getSignalQuality();
state.operatorName = cellular.getOperatorName();
state.ipAddress = cellular.getIpAddress();

state.uptimeSeconds = millis() / 1000UL;



    oled.update(state);

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