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
        Serial.println("TinyGSM init failed");
        return false;
    }

    Serial.println("TinyGSM initialized!");
    Serial.println(gsm.getModemInfo());

    Serial.println("Waiting for network...");

    if (!gsm.waitForNetwork(60000L))
    {
        Serial.println("Network registration failed");
        return false;
    }

    Serial.println("Network connected!");

    Serial.print("Operator: ");
    Serial.println(gsm.getOperator());

    Serial.print("Signal: ");
    Serial.println(gsm.getSignalQuality());

    Serial.println("Connecting to packet data...");

    if (!gsm.gprsConnect(APN, APN_USER, APN_PASS))
    {
        Serial.println("Packet data connection FAILED");
        return false;
    }

    Serial.println("Packet data connected!");

    Serial.print("IP Address: ");
    Serial.println(gsm.localIP());

    return true;
}