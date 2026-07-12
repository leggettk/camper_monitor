#pragma once

#include <Arduino.h>
#include <TinyGsmClient.h>

class Modem
{
public:
    bool begin();

    bool isNetworkConnected() const;
    bool isDataConnected() const;

    int getSignalQuality() const;
    const String &getOperatorName() const;
    const String &getIpAddress() const;

private:
    bool networkConnected = false;
    bool dataConnected = false;

    int signalQuality = 99;

    String operatorName = "Unknown";
    String ipAddress = "No IP";
};

extern Modem cellular;
extern TinyGsm gsm;