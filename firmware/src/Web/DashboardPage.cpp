#include "Web/DashboardPage.h"
#include "Version.h"
#include "Constants.h"

void DashboardPage::begin(
    WebServer& server,
    AppState& state)
{
    server_ = &server;
    state_ = &state;
}
void DashboardPage::handle()
{
    if (server_ == nullptr ||
        state_ == nullptr)
    {
        return;
    }

    const AppState& state = *state_;

    String overallTitle;
    String overallMessage;

    const StatusLevel overallLevel =
        determineOverallStatus(
            overallTitle,
            overallMessage);

    WebPageBuilder page;

    page.begin(
        FW_NAME,
        FW_NAME,
        "Know your camper. Anywhere.",
        10);

    page.addNavigation("dashboard");

    page.addHealthBanner(
        overallTitle,
        overallMessage,
        overallLevel);

    page.beginGrid();

    page.addMetricCard(
        "Ambient",
        formatTemperature(),
        state.temperature.valid
            ? "Interior temperature"
            : "Sensor unavailable",
        state.alarms.highTemperature
            ? StatusLevel::Critical
            : state.temperature.valid
                ? StatusLevel::Normal
                : StatusLevel::Neutral);

    page.addMetricCard(
        "Battery",
        formatBatteryVoltage(),
        state.power.batteryVoltageValid
            ? "Backup power"
            : "Reading unavailable",
        state.power.batteryVoltageValid
            ? StatusLevel::Normal
            : StatusLevel::Neutral);

    page.addMetricCard(
        "Power",
        state.power.shorePowerPresent
            ? "Shore"
            : "Battery",
        state.power.shorePowerPresent
            ? "External power present"
            : "Running on backup",
        state.power.shorePowerPresent
            ? StatusLevel::Normal
            : StatusLevel::Critical);

    page.endGrid();

    page.beginCard("Connectivity");

    page.addStatus(
        "Wi-Fi",
        state.wifi.connected
            ? "Connected"
            : "Disconnected",
        state.wifi.connected
            ? StatusLevel::Normal
            : StatusLevel::Neutral);

    page.addStatus(
        "LTE Network",
        state.cellular.networkConnected
            ? "Connected"
            : "Disconnected",
        state.cellular.networkConnected
            ? StatusLevel::Normal
            : StatusLevel::Critical);

    page.addStatus(
        "Cellular Data",
        state.cellular.dataConnected
            ? "Connected"
            : "Disconnected",
        state.cellular.dataConnected
            ? StatusLevel::Normal
            : StatusLevel::Critical);

    page.addStatus(
        "MQTT",
        state.mqtt.connected
            ? "Connected"
            : "Disconnected",
        state.mqtt.connected
            ? StatusLevel::Normal
            : StatusLevel::Warning);

    page.endCard();

    page.beginCard("Firmware");

    page.addValue(
        "Version",
        FW_VERSION);

    page.addValue(
        "Build",
        String(__DATE__) +
        " " +
        __TIME__);

    page.addStatus(
        "OTA Updates",
        "Enabled",
        StatusLevel::Normal);

    page.endCard();

    page.beginCard("System");

    page.addValue(
        "Uptime",
        formatUptime(
            state.system.uptimeSeconds));

    page.addValue(
        "Free Heap",
        formatFreeHeap(
            state.system.freeHeap));

    page.endCard();

    page.addFooter(
        FW_VERSION,
        formatUptime(
            state.system.uptimeSeconds));

    server_->send(
        200,
        "text/html; charset=utf-8",
        page.build());
}
StatusLevel DashboardPage::determineOverallStatus(
    String& title,
    String& message) const
{
    const AppState& state = *state_;

    if (state.alarms.highTemperature ||
        state.alarms.shorePower)
    {
        title = "Camper needs attention";

        if (state.alarms.highTemperature &&
            state.alarms.shorePower)
        {
            message =
                "High temperature and shore-power loss detected.";
        }
        else if (state.alarms.highTemperature)
        {
            message =
                "The ambient temperature alarm is active.";
        }
        else
        {
            message =
                "Shore power has been lost.";
        }

        return StatusLevel::Critical;
    }

    if (!state.mqtt.connected ||
        !state.cellular.dataConnected)
    {
        title =
            "Camper has a connectivity warning";

        message =
            "Monitoring is active, but one or more remote services are offline.";

        return StatusLevel::Warning;
    }

    title = "Camper status is healthy";

    message =
        "All monitored systems are operating normally.";

    return StatusLevel::Normal;
}
String DashboardPage::formatTemperature() const
{
    if (state_ == nullptr ||
        !state_->temperature.valid)
    {
        return "Unavailable";
    }

    return String(
        state_->temperature.ambientF,
        1) + " °F";
}

String DashboardPage::formatBatteryVoltage() const
{
    if (state_ == nullptr ||
        !state_->power.batteryVoltageValid)
    {
        return "Unavailable";
    }

    return String(
        state_->power.batteryVoltage,
        2) + " V";
}

String DashboardPage::formatUptime(
    uint32_t totalSeconds) const
{
    const uint32_t days =
        totalSeconds / 86400UL;

    totalSeconds %= 86400UL;

    const uint8_t hours =
        totalSeconds / 3600UL;

    totalSeconds %= 3600UL;

    const uint8_t minutes =
        totalSeconds / 60UL;

    const uint8_t seconds =
        totalSeconds % 60UL;

    char buffer[40];

    if (days > 0)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "%lu d %02u:%02u:%02u",
            static_cast<unsigned long>(days),
            hours,
            minutes,
            seconds);
    }
    else
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "%02u:%02u:%02u",
            hours,
            minutes,
            seconds);
    }

    return String(buffer);
}

String DashboardPage::formatFreeHeap(
    uint32_t bytes) const
{
    const float kilobytes =
        static_cast<float>(bytes) / 1024.0f;

    return String(kilobytes, 1) + " KB";
}
