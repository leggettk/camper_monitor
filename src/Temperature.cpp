#include "Temperature.h"
#include "Pins.h"

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(PIN_TEMP_AMBIENT);
DallasTemperature sensors(&oneWire);

bool Temperature::begin()
{
    sensors.begin();

    return sensors.getDeviceCount() > 0;
}

float Temperature::getFahrenheit()
{
    sensors.requestTemperatures();

    return sensors.getTempFByIndex(0);
}