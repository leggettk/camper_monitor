#include "SMS.h"

#include "Logger.h"
#include "Modem.h"
#include "Secrets.h"

SMS sms;

bool SMS::send(const String &message)
{
    if (sending)
    {
        logger.warning("SMS skipped: another SMS is active");
        return false;
    }

    if (!cellular.isNetworkConnected())
    {
        logger.error("SMS failed: cellular network is offline");
        return false;
    }

    sending = true;

    logger.infof(
        "Sending SMS to %s",
        SMS_PHONE_NUMBER);

    const bool sent =
        gsm.sendSMS(
            String(SMS_PHONE_NUMBER),
            message);

    sending = false;

    if (sent)
    {
        logger.info("SMS sent successfully");
    }
    else
    {
        logger.error("SMS send failed");
    }

    return sent;
}

bool SMS::sendShorePowerLost(
    float batteryVoltage,
    bool batteryValid)
{
    String message =
        "CAMPER ALERT: Shore power lost.";

    if (batteryValid)
    {
        message += " Battery ";
        message += String(batteryVoltage, 2);
        message += " V.";
    }

    return send(message);
}

bool SMS::sendShorePowerRestored()
{
    return send(
        "CAMPER UPDATE: Shore power restored.");
}

bool SMS::sendHighTemperature(
    float temperatureF,
    float setpointF)
{
    String message =
        "CAMPER ALERT: High temperature ";

    message += String(temperatureF, 1);
    message += " F. Limit ";
    message += String(setpointF, 1);
    message += " F.";

    return send(message);
}

bool SMS::sendTemperatureNormal(
    float temperatureF)
{
    String message =
        "CAMPER UPDATE: Temperature returned to normal at ";

    message += String(temperatureF, 1);
    message += " F.";

    return send(message);
}