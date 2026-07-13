#include "Display.h"

#include <Wire.h>

#include "Pins.h"
#include "Config.h"

Display oled;

bool Display::begin()
{
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);

    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            OLED_ADDRESS))
    {
        return false;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);
    display.display();

    awake = true;
    resetActivityTimer();

    return true;
}

void Display::bootScreen()
{
    wake();

    display.clearDisplay();

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("Camper");
    display.println("Monitor");

    display.setTextSize(1);
    display.println();
    display.println("Starting...");

    display.display();
}

void Display::update(const DisplayData &data)
{
    const unsigned long now = millis();

    if (
        awake &&
        now - lastActivityMs >= SCREEN_TIMEOUT_MS)
    {
        sleep();
        return;
    }

    if (!awake)
    {
        return;
    }

    if (now - lastRefreshMs < REFRESH_INTERVAL_MS)
    {
        return;
    }

    lastRefreshMs = now;

    switch (page)
    {
        case DisplayPage::Ambient:
            renderAmbient(data);
            break;

        case DisplayPage::LTE:
            renderLTE(data);
            break;

        case DisplayPage::Internet:
            renderInternet(data);
            break;

        case DisplayPage::System:
            renderSystem(data);
            break;
    }
}

void Display::nextPage()
{
    wake();

    const uint8_t next =
        (static_cast<uint8_t>(page) + 1) % 4;

    page = static_cast<DisplayPage>(next);
    lastRefreshMs = 0;
}

void Display::previousPage()
{
    wake();

    uint8_t current = static_cast<uint8_t>(page);

    if (current == 0)
    {
        current = 3;
    }
    else
    {
        current--;
    }

    page = static_cast<DisplayPage>(current);
    lastRefreshMs = 0;
}

void Display::wake()
{
    if (!awake)
    {
        display.ssd1306_command(SSD1306_DISPLAYON);
        awake = true;
    }

    resetActivityTimer();
    lastRefreshMs = 0;
}

void Display::sleep()
{
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    awake = false;
}

bool Display::isAwake() const
{
    return awake;
}

DisplayPage Display::currentPage() const
{
    return page;
}

void Display::status(
    const String &line1,
    const String &line2,
    const String &line3)
{
    wake();

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println(line1);
    display.println();
    display.println(line2);
    display.println(line3);

    display.display();
}

void Display::renderAmbient(const DisplayData &data)
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Camper Monitor");
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    display.setCursor(0, 18);
    display.println("Ambient");

    display.setTextSize(3);
    display.setCursor(0, 32);
    display.print(data.ambientTemperatureF, 1);
    display.setTextSize(1);
    display.print(" F");

    display.display();
}

void Display::renderLTE(const DisplayData &data)
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("LTE Status");
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    display.setCursor(0, 18);
    display.print("Network: ");
    display.println(
        data.networkConnected ? "Online" : "Offline");

    display.print("Carrier: ");
    display.println(data.operatorName);

    display.print("Signal: ");
    if (data.signalQuality == 99)
    {
        display.println("Unknown");
    }
    else
    {
        display.println(data.signalQuality);
    }

    display.display();
}

void Display::renderInternet(const DisplayData &data)
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Internet");
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    display.setCursor(0, 18);

    display.print("Data: ");
    display.println(
        data.dataConnected ? "Online" : "Offline");

    display.print("MQTT: ");
    display.println(
        data.mqttConnected ? "Online" : "Offline");

    display.println("IP:");
    display.println(data.ipAddress);

    display.display();
}

void Display::renderSystem(const DisplayData &data)
{
    const unsigned long hours =
        data.uptimeSeconds / 3600UL;

    const unsigned long minutes =
        (data.uptimeSeconds % 3600UL) / 60UL;

    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("System");
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    display.setCursor(0, 18);
    display.print("Firmware: ");
    display.println(FW_VERSION);

    display.print("Uptime: ");
    display.print(hours);
    display.print("h ");
    display.print(minutes);
    display.println("m");

    display.print("Free heap: ");
    display.println(ESP.getFreeHeap());

    display.display();
}

void Display::resetActivityTimer()
{
    lastActivityMs = millis();
}