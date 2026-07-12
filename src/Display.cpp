#include "Display.h"
#include <Wire.h>
# include "Pins.h"
Display oled;

bool Display::begin()
{
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        return false;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    return true;
}

void Display::bootScreen()
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("Camper");

    display.println("Monitor");

    display.setTextSize(1);
    display.println();
    display.println("Firmware 0.1.0");

    display.display();
}

void Display::status(
    const String &line1,
    const String &line2,
    const String &line3)
{
    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println(line1);

    display.println(line2);

    display.println(line3);

    display.display();
}