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
#include "WebServerService.h"

Settings settings;
AppState state;
Temperature temp;
WiFiService wifi;
WebServerService web;
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

    state.temperature.valid =
        reading > -100.0f &&
        reading < 200.0f;

    if (state.temperature.valid)
    {
        state.temperature.ambientF = reading;

        logger.infof(
            "Ambient temperature: %.1f F",
            state.temperature.ambientF);
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

    state.power.batteryVoltage =
        battery.voltage();

    state.power.batteryVoltageValid =
        battery.isValid();

    if (state.power.batteryVoltageValid)
    {
        logger.infof(
            "Camper battery: %.2f V",
            state.power.batteryVoltage);
    }
}

void updateCommunicationsState(unsigned long now)
{
    state.cellular.networkConnected =
        cellular.isNetworkConnected();

    state.cellular.dataConnected =
        cellular.isDataConnected();

    state.mqtt.connected =
        mqtt.isConnected();

    state.cellular.signalQuality =
        cellular.getSignalQuality();

    state.cellular.operatorName =
        cellular.getOperatorName();

    state.cellular.ipAddress =
        cellular.getIpAddress();

    state.system.uptimeSeconds =
        now / 1000UL;

    state.system.freeHeap =
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
        !state.temperature.valid ||
        now - lastPublish <
            MQTT_PUBLISH_INTERVAL_MS)
    {
        return;
    }

    lastPublish = now;

    if (state.mqtt.connected)
    {
        mqtt.publishAmbient(
            state.temperature.ambientF);
    }
}

void updateShorePower()
{
    static bool initialized = false;

    shorePower.update();

    state.power.shorePowerPresent =
        shorePower.isPresent();

    // Prevent a false "power lost" SMS during startup.
    if (!initialized)
    {
        initialized = true;

        state.alarms.shorePower =
            !state.power.shorePowerPresent;

        return;
    }

    if (!shorePower.changed())
    {
        return;
    }

    mqtt.publishShorePower(
        state.power.shorePowerPresent);

    if (!state.power.shorePowerPresent)
    {
        logger.warning("Shore power lost");

        state.alarms.shorePower = true;

        state.sms.lastSuccessful =
            sms.sendShorePowerLost(
                state.power.batteryVoltage,
                state.power.batteryVoltageValid);

        if (state.sms.lastSuccessful)
        {
            state.sms.lastSentTimeSeconds =
                millis() / 1000UL;
        }
    }
    else
    {
        logger.info("Shore power restored");

        state.alarms.shorePower = false;

        state.sms.lastSuccessful =
            sms.sendShorePowerRestored();

        if (state.sms.lastSuccessful)
        {
            state.sms.lastSentTimeSeconds =
                millis() / 1000UL;
        }
    }
}

void updateTemperatureAlarm(unsigned long now)
{
    static unsigned long highTemperatureSince = 0;

    if (!state.temperature.valid)
    {
        highTemperatureSince = 0;
        return;
    }

    if (!state.alarms.highTemperature)
    {
        if (
            state.temperature.ambientF >=
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
                state.alarms.highTemperature = true;
                highTemperatureSince = 0;

                logger.errorf(
                    "High-temperature alarm: %.1f F",
                    state.temperature.ambientF);

                state.sms.lastSuccessful =
                    sms.sendHighTemperature(
                        state.temperature.ambientF,
                        TEMP_HIGH_ALARM_F);

                if (state.sms.lastSuccessful)
                {
                    state.sms.lastSentTimeSeconds =
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
        state.temperature.ambientF <=
        TEMP_HIGH_RESET_F)
    {
        state.alarms.highTemperature = false;

        logger.infof(
            "Temperature alarm cleared: %.1f F",
            state.temperature.ambientF);

        state.sms.lastSuccessful =
            sms.sendTemperatureNormal(
                state.temperature.ambientF);

        if (state.sms.lastSuccessful)
        {
            state.sms.lastSentTimeSeconds =
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

    if (state.mqtt.connected)
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

    wifi.begin(settings, logger);
    web.begin(settings, logger);

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
    web.update();
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