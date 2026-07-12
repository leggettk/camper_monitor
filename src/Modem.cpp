#include <Arduino.h>
#include <TinyGsmClient.h>

#include "Modem.h"
#include "Pins.h"
#include "Secrets.h"

HardwareSerial SerialAT(1);

TinyGsm gsm(SerialAT);
Modem cellular;

bool Modem::begin()
{
    Serial.println("Starting modem...");

    SerialAT.begin(
        115200,
        SERIAL_8N1,
        PIN_MODEM_RX,
        PIN_MODEM_TX);

    pinMode(PIN_MODEM_PWRKEY, OUTPUT);

    digitalWrite(PIN_MODEM_PWRKEY, HIGH);
    delay(100);

    digitalWrite(PIN_MODEM_PWRKEY, LOW);
    delay(1200);

    digitalWrite(PIN_MODEM_PWRKEY, HIGH);
    delay(3000);

    Serial.println("Initializing TinyGSM...");

    if (!gsm.init())
    {
        Serial.println("TinyGSM initialization failed");
        return false;
    }

    Serial.println("TinyGSM initialized!");
    Serial.println(gsm.getModemInfo());

    Serial.println("Waiting for network...");

    networkConnected = gsm.waitForNetwork(60000L);

    if (!networkConnected)
    {
        Serial.println("Network registration failed");
        return false;
    }

    operatorName = gsm.getOperator();
    signalQuality = gsm.getSignalQuality();

    Serial.println("Network connected!");

    Serial.print("Operator: ");
    Serial.println(operatorName);

    Serial.print("Signal: ");
    Serial.println(signalQuality);

    Serial.println("Connecting to packet data...");

    dataConnected = gsm.gprsConnect(
        APN,
        APN_USER,
        APN_PASS);

    if (!dataConnected)
    {
        Serial.println("Packet data connection FAILED");
        return false;
    }

    ipAddress = gsm.localIP().toString();

    Serial.println("Packet data connected!");

    Serial.print("IP Address: ");
    Serial.println(ipAddress);

    return true;
}
bool Modem::isNetworkConnected() const
{
    return networkConnected;
}

bool Modem::isDataConnected() const
{
    return dataConnected;
}

int Modem::getSignalQuality() const
{
    return signalQuality;
}

const String &Modem::getOperatorName() const
{
    return operatorName;
}

const String &Modem::getIpAddress() const
{
    return ipAddress;
}