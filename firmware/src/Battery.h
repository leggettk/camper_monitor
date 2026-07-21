#pragma once

class Battery
{
public:
    void begin();
    void update();

    float voltage() const;
    bool isValid() const;

private:
    float batteryVoltage = 0.0f;
    bool valid = false;

    static constexpr float R1_OHMS = 100000.0f;
    static constexpr float R2_OHMS = 22000.0f;

    // Adjust after comparing against a multimeter.
    static constexpr float CALIBRATION_FACTOR = 1.0f;
};

extern Battery battery;