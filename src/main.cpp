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

    static unsigned long lastTemperatureRead = 0;
    static unsigned long lastScreenChange = 0;
    static unsigned long lastPublish = 0;

    static float ambientTemperatureF = 0.0f;
    static uint8_t screenNumber = 0;

    const unsigned long now = millis();

    // Read the sensor every five seconds.
    if (now - lastTemperatureRead >= 5000)
    {
        lastTemperatureRead = now;

        ambientTemperatureF = temp.getFahrenheit();

        Serial.printf(
            "Ambient: %.1f F\n",
            ambientTemperatureF);
    }

    // Rotate the OLED page every five seconds.
    if (now - lastScreenChange >= 5000)
    {
        lastScreenChange = now;
        screenNumber = (screenNumber + 1) % 3;

        switch (screenNumber)
        {
            case 0:
                oled.status(
                    "Camper Monitor",
                    "Ambient",
                    String(ambientTemperatureF, 1) + " F");
                break;

            case 1:
                oled.status(
                    cellular.isNetworkConnected()
                        ? "LTE Connected"
                        : "LTE Offline",
                    cellular.getOperatorName(),
                    "Signal: " +
                        String(cellular.getSignalQuality()));
                break;

            case 2:
                oled.status(
                    cellular.isDataConnected()
                        ? "Packet Data"
                        : "Data Offline",
                    "IP: " + cellular.getIpAddress(),
                    "MQTT: Offline");
                break;
        }
    }

    // This will remain unsuccessful until MQTT TLS is added.
    if (now - lastPublish >= 30000)
    {
        lastPublish = now;

        if (mqtt.publishAmbient(ambientTemperatureF))
        {
            Serial.println("Published temperature");
        }
        else
        {
            Serial.println("Temperature publish failed");
        }
    }
}