#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SSLClient.h>
#include <TinyGsmClient.h>

#include "Logger.h"
#include "Modem.h"
#include "MQTT.h"
#include "Pins.h"
#include "Secrets.h"
#include "TrustAnchors.h"
#include "Version.h"

// Working SIM7000 TCP transport.
TinyGsmClient cellularClient(gsm);

// TLS layer with server-certificate verification.
SSLClient tlsClient(
    cellularClient,
    TAs,
    static_cast<size_t>(TAs_NUM),
    PIN_TLS_ENTROPY);

// MQTT protocol layer.
PubSubClient mqttClient(tlsClient);

MQTT mqtt;

namespace
{
constexpr char STATUS_TOPIC[] =
    "rvmonitor/camper01/status";

constexpr char AMBIENT_TOPIC[] =
    "rvmonitor/camper01/temperature/ambient";

constexpr char AMBIENT_DISCOVERY_TOPIC[] =
    "homeassistant/sensor/camper01_ambient/config";

constexpr char SHORE_POWER_TOPIC[] =
    "rvmonitor/camper01/power/shore";

constexpr char HEARTBEAT_TOPIC[] =
    "rvmonitor/camper01/heartbeat";
}

bool MQTT::begin()
{
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setKeepAlive(60);
    mqttClient.setSocketTimeout(30);
    mqttClient.setBufferSize(1024);

    return connect();
}

bool MQTT::connect()
{
    if (!cellular.isDataConnected())
    {
        logger.warning("MQTT skipped: packet data is offline");
        return false;
    }
    tlsClient.stop();

    logger.infof(
        "Connecting to MQTT TLS broker %s:%u",
        MQTT_HOST,
        static_cast<unsigned int>(MQTT_PORT));

    const bool connected = mqttClient.connect(
        MQTT_CLIENT_ID,
        MQTT_USER,
        MQTT_PASSWORD,
        STATUS_TOPIC,
        1,
        true,
        "offline");

    if (!connected)
    {
        logger.errorf(
            "MQTT connection failed, state=%d",
            mqttClient.state());

        tlsClient.stop();
        return false;
    }
    if (!discoveryPublished)
    {
            if (publishDiscovery())
            {
                discoveryPublished = true;
            }
    }
    mqttClient.publish(
        STATUS_TOPIC,
        "online",
        true);

    logger.info("MQTT TLS connected");

    if (!publishDiscovery())
    {
        logger.warning(
        "Ambient discovery publish failed");
    }
    return true;
}

void MQTT::loop()
{
    if (mqttClient.connected())
    {
        mqttClient.loop();
        return;
    }

    const unsigned long now = millis();

    if (
        now - lastReconnectAttemptMs <
        RECONNECT_INTERVAL_MS)
    {
        return;
    }

    lastReconnectAttemptMs = now;

    tlsClient.stop();

    connect();
}

bool MQTT::isConnected() const
{
    return mqttClient.connected();
}

bool MQTT::publishAmbient(float temperatureF)
{
    if (!mqttClient.connected())
    {
        logger.warning(
            "Ambient publish skipped: MQTT offline");

        return false;
    }

    char payload[16];

    snprintf(
        payload,
        sizeof(payload),
        "%.1f",
        temperatureF);

    const bool published = mqttClient.publish(
        AMBIENT_TOPIC,
        payload,
        true);

    if (published)
    {
        logger.infof(
            "Published ambient temperature: %s F",
            payload);
    }
    else
    {
        logger.errorf(
            "Ambient publish failed, MQTT state=%d, connected=%s",
            mqttClient.state(),
            mqttClient.connected() ? "yes" : "no");
    }

    return published;
}
bool MQTT::publishDiscovery()
{
    if (!mqttClient.connected())
    {
        return false;
    }

    JsonDocument document;

    document["name"] = "Ambient Temperature";
    document["unique_id"] = "camper01_ambient_temperature";
    document["default_entity_id"] =
        "sensor.camper_ambient_temperature";

    document["state_topic"] = AMBIENT_TOPIC;
    document["availability_topic"] = STATUS_TOPIC;

    document["payload_available"] = "online";
    document["payload_not_available"] = "offline";

    document["device_class"] = "temperature";
    document["state_class"] = "measurement";
    document["unit_of_measurement"] = "\xC2\xB0"
                                      "F";

    JsonObject device =
        document["device"].to<JsonObject>();

    JsonArray identifiers =
        device["identifiers"].to<JsonArray>();

    identifiers.add("camper_monitor_01");

    device["name"] = "Camper Monitor";
    device["manufacturer"] = "DIY";
    device["model"] = "LILYGO T-SIM7000G";
    device["sw_version"] = FW_VERSION;

    char payload[768];

    const size_t payloadLength =
        serializeJson(
            document,
            payload,
            sizeof(payload));

    if (
        payloadLength == 0 ||
        payloadLength >= sizeof(payload))
    {
        logger.error(
            "Discovery JSON buffer is too small");

        return false;
    }

    const bool published =
        mqttClient.publish(
            AMBIENT_DISCOVERY_TOPIC,
            payload,
            true);

    if (published)
    {
        logger.info(
            "Published ambient MQTT discovery");
    }
    else
    {
        logger.error(
            "Failed to publish ambient MQTT discovery");
    }

    return published;
}
bool MQTT::publishShorePower(bool present)
{
    if (!mqttClient.connected())
    {
        logger.warning(
            "Shore power publish skipped: MQTT offline");

        return false;
    }

    const char *payload =
        present ? "ON" : "OFF";

    const bool published =
        mqttClient.publish(
            SHORE_POWER_TOPIC,
            payload,
            true);

    if (published)
    {
        logger.infof(
            "Published shore power: %s",
            payload);
    }
    else
    {
        logger.error(
            "Failed to publish shore power");
    }

    return published;
}
bool MQTT::publishHeartbeat(const AppState &state)
{
    if (!mqttClient.connected())
    {
        logger.warning(
            "Heartbeat publish skipped: MQTT offline");

        return false;
    }

    JsonDocument document;

    document["status"] = "online";
    document["uptime_seconds"] = state.uptimeSeconds;
    document["free_heap"] = state.freeHeap;
    document["signal_quality"] = state.signalQuality;

    document["network_connected"] =
        state.networkConnected;

    document["data_connected"] =
        state.dataConnected;

    document["mqtt_connected"] =
        state.mqttConnected;

    if (state.temperatureValid)
    {
        document["ambient_temperature_f"] =
            state.ambientTemperatureF;
    }

    if (state.batteryVoltageValid)
    {
        document["battery_voltage"] =
            state.batteryVoltage;
    }

    document["shore_power"] =
        state.shorePowerPresent;

    document["operator"] =
        state.operatorName;

    document["ip_address"] =
        state.ipAddress;

    document["firmware"] =
        FW_VERSION;

    char payload[512];

    const size_t length =
        serializeJson(
            document,
            payload,
            sizeof(payload));

    if (
        length == 0 ||
        length >= sizeof(payload))
    {
        logger.error(
            "Heartbeat JSON buffer is too small");

        return false;
    }

    const bool published =
        mqttClient.publish(
            HEARTBEAT_TOPIC,
            payload,
            true);

    if (published)
    {
        logger.infof(
            "Published heartbeat: uptime=%lus, heap=%lu",
            state.uptimeSeconds,
            state.freeHeap);
    }
    else
    {
        logger.errorf(
            "Heartbeat publish failed, MQTT state=%d",
            mqttClient.state());
    }

    return published;
}