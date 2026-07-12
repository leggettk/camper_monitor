#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Display
{
public:
    bool begin();

    void bootScreen();

    void status(
        const String &line1,
        const String &line2,
        const String &line3);

private:
    Adafruit_SSD1306 display =
        Adafruit_SSD1306(128, 64, &Wire, -1);
};

extern Display oled;