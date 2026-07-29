#pragma once

#include <Arduino.h>

class WebPageBuilder
{
public:
    void begin(
        const String& title,
        const String& heading,
        const String& subtitle = "",
        uint16_t refreshSeconds = 0);

    void beginCard(const String& title);
    void endCard();

    void addStatus(
        const String& label,
        const String& value,
        bool healthy = true);

    void addParagraph(const String& text);

    String build();

private:
    String html_;
    bool cardOpen_ = false;

    void appendStyles();
    String escapeHtml(const String& value) const;
};