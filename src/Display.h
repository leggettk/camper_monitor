#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

enum class DisplayPage : uint8_t
{
    Ambient = 0,
    LTE,
    Internet,
    System
};

struct DisplayData
{
    float ambientTemperatureF = 0.0f;

    bool networkConnected = false;
    bool dataConnected = false;
    bool mqttConnected = false;

    String operatorName = "Unknown";
    String ipAddress = "No IP";

    int signalQuality = 99;
    unsigned long uptimeSeconds = 0;
};

class Display
{
public:
    bool begin();
    void bootScreen();

    void update(const DisplayData &data);

    void nextPage();
    void previousPage();

    void wake();
    void sleep();
    bool isAwake() const;

    DisplayPage currentPage() const;

    // Keep this available for temporary alerts and errors.
    void status(
        const String &line1,
        const String &line2,
        const String &line3);

private:
    void renderAmbient(const DisplayData &data);
    void renderLTE(const DisplayData &data);
    void renderInternet(const DisplayData &data);
    void renderSystem(const DisplayData &data);

    void resetActivityTimer();

    Adafruit_SSD1306 display =
        Adafruit_SSD1306(128, 64, &Wire, -1);

    DisplayPage page = DisplayPage::Ambient;

    bool awake = true;
    unsigned long lastActivityMs = 0;
    unsigned long lastRefreshMs = 0;

    static constexpr unsigned long REFRESH_INTERVAL_MS = 1000;
    static constexpr unsigned long SCREEN_TIMEOUT_MS = 300000;
};

extern Display oled;