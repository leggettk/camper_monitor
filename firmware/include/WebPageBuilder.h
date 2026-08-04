#pragma once

#include <Arduino.h>

enum class StatusLevel
{
    Normal,
    Warning,
    Critical,
    Neutral
};

class WebPageBuilder
{
public:
    void begin(
        const String& title,
        const String& heading,
        const String& subtitle = "",
        uint16_t refreshSeconds = 0);

    void addNavigation(const String& activePage);

    void addHealthBanner(
        const String& title,
        const String& message,
        StatusLevel level);

    void beginGrid();
    void endGrid();

    void addMetricCard(
        const String& title,
        const String& value,
        const String& detail,
        StatusLevel level = StatusLevel::Neutral);

    void beginCard(const String& title);
    void endCard();

    void addStatus(
        const String& label,
        const String& value,
        StatusLevel level = StatusLevel::Neutral);

    void addValue(
        const String& label,
        const String& value);

    void addLink(
        const String& label,
        const String& url);

    void addFooter(
        const String& version,
        const String& uptime);

    String build();

private:
    String html_;

    bool cardOpen_ = false;
    bool gridOpen_ = false;
    bool footerAdded_ = false;

    void appendStyles();

    String escapeHtml(const String& value) const;
    String statusClass(StatusLevel level) const;
    String statusLabel(StatusLevel level) const;
};