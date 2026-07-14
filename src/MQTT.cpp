#include <Arduino.h>
#include <PubSubClient.h>
#include <SSLClient.h>
#include <TinyGsmClient.h>

#include "Logger.h"
#include "Modem.h"
#include "MQTT.h"
#include "Pins.h"
#include "Secrets.h"
#include "TrustAnchors.h"

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
}

bool MQTT::begin()
{
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setKeepAlive(60);
    mqttClient.setSocketTimeout(30);
    mqttClient.setBufferSize(512);

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

    mqttClient.publish(
        STATUS_TOPIC,
        "online",
        true);

    logger.info("MQTT TLS connected");
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
        logger.error(
            "Ambient temperature publish failed");
    }

    return published;
}