#include <Arduino.h>

#include "Button.h"
#include "Display.h"
#include "Modem.h"
#include "MQTT.h"
#include "Temperature.h"
#include "AppState.h"
#include "Logger.h"
#include "Battery.h"
#include "ShorePower.h"
#include "Config.h"

AppState state;
Temperature temp;
//constexpr unsigned long TEMPERATURE_INTERVAL_MS = 5000;
constexpr unsigned long BATTERY_INTERVAL_MS = 60000;
//constexpr unsigned long MQTT_PUBLISH_INTERVAL_MS = 60000;

void updateTemperature(unsigned long now)
{
    static unsigned long lastUpdate = 0;

    if (now - lastUpdate < TEMPERATURE_INTERVAL_MS)
    {
        return;
    }

    lastUpdate = now;

    const float reading = temp.getFahrenheit();

    state.temperatureValid =
        reading > -100.0f &&
        reading < 200.0f;

    if (state.temperatureValid)
    {
        state.ambientTemperatureF = reading;

        logger.infof(
            "Ambient temperature: %.1f F",
            state.ambientTemperatureF);
    }
    else
    {
        logger.warning("Invalid temperature reading");
    }
}

void updateBattery(unsigned long now)
{
    static unsigned long lastUpdate = 0;

    if (now - lastUpdate < BATTERY_INTERVAL_MS)
    {
        return;
    }

    lastUpdate = now;

    battery.update();

    state.batteryVoltage =
        battery.voltage();

    state.batteryVoltageValid =
        battery.isValid();

    if (state.batteryVoltageValid)
    {
        logger.infof(
            "Camper battery: %.2f V",
            state.batteryVoltage);
    }
}

void updateCommunicationsState(unsigned long now)
{
    state.networkConnected =
        cellular.isNetworkConnected();

    state.dataConnected =
        cellular.isDataConnected();

    state.mqttConnected =
        mqtt.isConnected();

    state.signalQuality =
        cellular.getSignalQuality();

    state.operatorName =
        cellular.getOperatorName();

    state.ipAddress =
        cellular.getIpAddress();

    state.uptimeSeconds =
        now / 1000UL;

    state.freeHeap =
        ESP.getFreeHeap();
}

void handleButton()
{
    userButton.update();

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

        logger.info("Display button pressed");
    }

    if (userButton.wasLongPressed())
    {
        logger.info("Long press detected");

        oled.status(
            "Camper Monitor",
            "MQTT reconnect",
            "Coming soon");
    }
}

void publishTelemetry(unsigned long now)
{
    static unsigned long lastPublish = 0;

    if (
        !state.temperatureValid ||
        now - lastPublish <
            MQTT_PUBLISH_INTERVAL_MS)
    {
        return;
    }

    lastPublish = now;

    if (state.mqttConnected)
    {
        mqtt.publishAmbient(
            state.ambientTemperatureF);
    }
}

void updateShorePower()
{
    shorePower.update();

    state.shorePowerPresent =
        shorePower.isPresent();

    if (shorePower.changed())
    {
        logger.warning(
            state.shorePowerPresent
                ? "Shore power restored"
                : "Shore power lost");

        mqtt.publishShorePower(
            state.shorePowerPresent);
    }
}

void publishHeartbeat(unsigned long now)
{
    static unsigned long lastHeartbeat = 0;

    if (
        now - lastHeartbeat <
        HEARTBEAT_INTERVAL_MS)
    {
        return;
    }

    lastHeartbeat = now;

    if (state.mqttConnected)
    {
        mqtt.publishHeartbeat(state);
    }
    else
    {
        logger.warning(
            "Heartbeat skipped: MQTT offline");
    }
}

void setup()
{
    logger.begin(115200, LogLevel::Info);
    delay(500);

    battery.begin();
    shorePower.begin();

    if (!oled.begin())
    {
        logger.error("OLED initialization failed!");
    }

    userButton.begin();

    if (!temp.begin())
    {
        logger.error("Temperature sensor not found!");
    }

    oled.bootScreen();
    delay(1500);

    if (cellular.begin())
    {
        logger.info("Modem OK");
    }
    else
    {
        logger.error("Modem FAILED");
    }

    if (mqtt.begin())
    {
        logger.info("MQTT OK");
    }
    else
    {
        logger.warning("MQTT unavailable");
    }
}

void loop()
{
    const unsigned long now = millis();

    mqtt.loop();

    handleButton();
    updateTemperature(now);
    updateBattery(now);
    updateCommunicationsState(now);
    updateShorePower();

    oled.update(state);

    publishTelemetry(now);
    publishHeartbeat(now);
}