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
#include "SMS.h"
#include "settings.h"
#include "WiFiService.h"

Settings settings;
AppState state;
Temperature temp;
WiFiService wifi;
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
    static bool initialized = false;

    shorePower.update();

    state.shorePowerPresent =
        shorePower.isPresent();

    // Prevent a false "power lost" SMS during startup.
    if (!initialized)
    {
        initialized = true;

        state.shorePowerAlarm =
            !state.shorePowerPresent;

        return;
    }

    if (!shorePower.changed())
    {
        return;
    }

    mqtt.publishShorePower(
        state.shorePowerPresent);

    if (!state.shorePowerPresent)
    {
        logger.warning("Shore power lost");

        state.shorePowerAlarm = true;

        state.lastSmsSuccessful =
            sms.sendShorePowerLost(
                state.batteryVoltage,
                state.batteryVoltageValid);

        if (state.lastSmsSuccessful)
        {
            state.lastSmsTimeSeconds =
                millis() / 1000UL;
        }
    }
    else
    {
        logger.info("Shore power restored");

        state.shorePowerAlarm = false;

        state.lastSmsSuccessful =
            sms.sendShorePowerRestored();

        if (state.lastSmsSuccessful)
        {
            state.lastSmsTimeSeconds =
                millis() / 1000UL;
        }
    }
}

void updateTemperatureAlarm(unsigned long now)
{
    static unsigned long highTemperatureSince = 0;

    if (!state.temperatureValid)
    {
        highTemperatureSince = 0;
        return;
    }

    if (!state.highTemperatureAlarm)
    {
        if (
            state.ambientTemperatureF >=
            TEMP_HIGH_ALARM_F)
        {
            if (highTemperatureSince == 0)
            {
                highTemperatureSince = now;

                logger.warning(
                    "High-temperature delay started");
            }

            if (
                now - highTemperatureSince >=
                TEMP_ALARM_DELAY_MS)
            {
                state.highTemperatureAlarm = true;
                highTemperatureSince = 0;

                logger.errorf(
                    "High-temperature alarm: %.1f F",
                    state.ambientTemperatureF);

                state.lastSmsSuccessful =
                    sms.sendHighTemperature(
                        state.ambientTemperatureF,
                        TEMP_HIGH_ALARM_F);

                if (state.lastSmsSuccessful)
                {
                    state.lastSmsTimeSeconds =
                        now / 1000UL;
                }
            }
        }
        else
        {
            highTemperatureSince = 0;
        }

        return;
    }

    // Alarm is already active. Reset only after cooling
    // below the lower reset threshold.
    if (
        state.ambientTemperatureF <=
        TEMP_HIGH_RESET_F)
    {
        state.highTemperatureAlarm = false;

        logger.infof(
            "Temperature alarm cleared: %.1f F",
            state.ambientTemperatureF);

        state.lastSmsSuccessful =
            sms.sendTemperatureNormal(
                state.ambientTemperatureF);

        if (state.lastSmsSuccessful)
        {
            state.lastSmsTimeSeconds =
                now / 1000UL;
        }
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

if (settings.begin())
{
    logger.info("Settings loaded");

    wifi.begin(settings);

  const DeviceSettings& config = settings.get();

    logger.info(
        String("Device: ") + config.deviceName
    );

    logger.info(
        String("Hostname: ") + config.hostname
    );

    logger.info(
        String("WiFi configured: ") +
        (config.wifiSSID.isEmpty() ? "No" : "Yes")
    );
}
else
{
    logger.error("Settings initialization failed");
}

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
    wifi.update();
    handleButton();
    updateTemperature(now);
    updateBattery(now);
    updateCommunicationsState(now);
    updateShorePower();
    updateTemperatureAlarm(now);

    oled.update(state);

    publishTelemetry(now);
    publishHeartbeat(now);
}