#pragma once

class ShorePower
{
public:
    void begin();
    void update();

    bool isPresent() const;
    bool changed();

private:
    bool present = false;
    bool previousPresent = false;
    bool changeEvent = false;

    unsigned long lastRawChangeMs = 0;

    static constexpr unsigned long DEBOUNCE_MS = 1000;
};

extern ShorePower shorePower;