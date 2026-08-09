#include "WebServerService.h"
#include "WebPageBuilder.h"
#include "Constants.h"
#include "Web/DashboardPage.h"
#include <Update.h>

DashboardPage dashboardPage_;
static String formatUptimeValue(uint32_t totalSeconds)
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

bool WebServerService::begin(
    Settings& settings,
    AppState& appState,
    Logger& logger,
    OTAService& otaService)
{
    if (started_)
    {
        return true;
    }
    otaService_ = &otaService;
    settings_ = &settings;
    appState_ = &appState;
    logger_ = &logger;
    dashboardPage_.begin(
    server_,
    appState);

    registerRoutes();

    server_.begin();
    started_ = true;

    logger_->info(String("Web server started on port 80"));

    return true;
}

void WebServerService::update()
{
    if (!started_)
    {
        return;
    }

    server_.handleClient();
}

bool WebServerService::running() const
{
    return started_;
}

void WebServerService::registerRoutes()
{
    server_.on(
        "/",
        HTTP_GET,
        [this]()
        {
            dashboardPage_.handle();
        });

    server_.onNotFound(
        [this]()
        {
            handleNotFound();
        });

    server_.on(
        "/ota",
        HTTP_GET,
        [this]()
        {
            handleOtaPage();
        });

    server_.on(
        "/ota",
        HTTP_POST,
        [this]()
        {
            handleOtaUploadComplete();
        },
    [this]()
    {
        handleOtaUpload();
    });

    server_.on(
    "/settings",
    HTTP_GET,
    [this]()
    {
        handleSettings();
    });

    server_.on(
    "/settings/alerts",
    HTTP_GET,
    [this]()
    {
        handleAlertSettings();
    });

    server_.on(
    "/settings/alerts",
    HTTP_POST,
    [this]()
    {
        handleAlertSettingsSave();
    });

    server_.on(
    "/settings/reporting",
    HTTP_GET,
    [this]()
    {
        handleReportingSettings();
    });

    server_.on(
    "/settings/device",
    HTTP_GET,
    [this]()
    {
        handleDeviceSettings();
    });

    server_.on(
    "/settings/connectivity",
    HTTP_GET,
    [this]()
    {
        WebPageBuilder page;

        page.begin(
            PROJECT_NAME,
            "Connectivity Settings",
            "Wi-Fi, MQTT, and cellular configuration.");

        page.addNavigation("settings");

        page.addHealthBanner(
            "Connectivity configuration",
            "Editable controls will be added later.",
            StatusLevel::Neutral);

        page.addLink(
            "Back to Settings",
            "/settings");

        server_.send(
            200,
            "text/html; charset=utf-8",
            page.build());
        server_.on(
            "/settings/alerts",
            HTTP_POST,
            [this]()
        {
            handleAlertSettingsSave();
        });
    });
server_.on(
    "/settings/reporting",
    HTTP_GET,
    [this]()
    {
        handleReportingSettings();
    });

server_.on(
    "/settings/reporting",
    HTTP_POST,
    [this]()
    {
        handleReportingSettingsSave();
    });
server_.on(
    "/settings/device",
    HTTP_GET,
    [this]()
    {
        handleDeviceSettings();
    });

server_.on(
    "/settings/device",
    HTTP_POST,
    [this]()
    {
        handleDeviceSettingsSave();
    });
}

void WebServerService::handleNotFound()
{
    String message = "Not Found\n\n";
    message += "URI: ";
    message += server_.uri();
    message += "\nMethod: ";
    message += server_.method() == HTTP_GET ? "GET" : "OTHER";
    message += "\n";

    server_.send(
        404,
        "text/plain; charset=utf-8",
        message);
}
void WebServerService::handleOtaPage()
{
    const String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta
        name="viewport"
        content="width=device-width, initial-scale=1.0">

    <title>Camper Sentinel OTA</title>
</head>
<body>
    <h1>Camper Sentinel Firmware Update</h1>

    <form
        method="POST"
        action="/ota"
        enctype="multipart/form-data">

        <input
            type="file"
            name="firmware"
            accept=".bin"
            required>

        <button type="submit">
            Upload Firmware
        </button>
    </form>

    <p>
        <a href="/">Back to Dashboard</a>
    </p>
</body>
</html>
)rawliteral";

    server_.send(
        200,
        "text/html; charset=utf-8",
        html);
}
void WebServerService::handleOtaUpload()
{
    HTTPUpload& upload = server_.upload();

    switch (upload.status)
    {
        case UPLOAD_FILE_START:
            otaService_->beginUpload();
            break;

        case UPLOAD_FILE_WRITE:
            otaService_->writeChunk(
                upload.buf,
                upload.currentSize);
            break;

        case UPLOAD_FILE_END:
            otaService_->finishUpload();
            break;

        case UPLOAD_FILE_ABORTED:
            otaService_->abortUpload();
            break;

        default:
            break;
    }
}

void WebServerService::handleOtaUploadComplete()
{
    if (otaService_->successful())
    {
        server_.send(
            200,
            "text/html",
            "<h2>Firmware updated successfully.<br>"
            "Rebooting...</h2>");

        delay(1000);

        ESP.restart();
    }
    else
    {
        server_.send(
            500,
            "text/plain",
            otaService_->lastError());
    }
}
void WebServerService::handleSettings()
{
    WebPageBuilder page;

    page.begin(
        PROJECT_NAME,
        "Settings",
        "Configure Camper Sentinel.",
        0);

    page.addNavigation("settings");

    page.addHealthBanner(
        "Device configuration",
        "Choose a category below. Changes are stored in persistent memory.",
        StatusLevel::Neutral);

    page.addSettingsCard(
        "Alerts",
        "Temperature limits, shore-power alerts, and SMS behavior.",
        "/settings/alerts");

    page.addSettingsCard(
        "Reporting",
        "MQTT telemetry and heartbeat intervals.",
        "/settings/reporting");

    page.addSettingsCard(
        "Device",
        "Device name, hostname, and general system settings.",
        "/settings/device");

    page.addSettingsCard(
        "Connectivity",
        "Wi-Fi, MQTT, and cellular preferences. Coming next.",
        "/settings/connectivity");

    page.addSettingsCard(
        "Firmware",
        "View version information and upload OTA firmware.",
        "/ota");

    page.addFooter(
        PROJECT_VERSION,
        formatUptimeValue(
            appState_ != nullptr
                ? appState_->system.uptimeSeconds
                : 0));

    server_.send(
        200,
        "text/html; charset=utf-8",
        page.build());
}

void WebServerService::handleReportingSettings()
{
    if (settings_ == nullptr)
    {
        server_.send(
            500,
            "text/plain; charset=utf-8",
            "Settings service is unavailable.");

        return;
    }

    const DeviceSettings& config =
        settings_->get();

    WebPageBuilder page;

    page.begin(
        PROJECT_NAME,
        "Reporting Settings",
        "Configure telemetry and heartbeat timing.");

    page.addNavigation("settings");

    if (server_.hasArg("saved"))
    {
        page.addToast(
            "Reporting settings saved successfully.",
            StatusLevel::Normal);
    }

    page.beginCard("Telemetry");

    page.beginForm(
        "/settings/reporting",
        "POST");

    page.addNumberInput(
        "Temperature publish interval",
        "tempPublishSeconds",
        config.temperaturePublishIntervalSeconds,
        30.0f,
        3600.0f,
        30.0f,
        "sec",
        "How often ambient temperature is published to MQTT.");

    page.addNumberInput(
        "Heartbeat interval",
        "heartbeatSeconds",
        config.heartbeatIntervalSeconds,
        60.0f,
        86400.0f,
        60.0f,
        "sec",
        "How often Camper Sentinel publishes its heartbeat and system status.");

    page.addSubmitButton(
        "Save Settings");

    page.endForm();

    // Keep this even though dirty-form detection
    // is currently optional.
    page.addSettingsFormScript();

    page.endCard();

    page.addLink(
        "Back to Settings",
        "/settings");

    page.addFooter(
        PROJECT_VERSION,
        formatUptimeValue(
            appState_ != nullptr
                ? appState_->system.uptimeSeconds
                : 0));

    server_.send(
        200,
        "text/html; charset=utf-8",
        page.build());
}
void WebServerService::handleReportingSettingsSave()
{
    if (settings_ == nullptr)
    {
        server_.send(
            500,
            "text/plain; charset=utf-8",
            "Settings service is unavailable.");

        return;
    }

    if (!server_.hasArg("tempPublishSeconds") ||
        !server_.hasArg("heartbeatSeconds"))
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "Required settings are missing.");

        return;
    }

    const uint32_t tempPublishSeconds =
        server_.arg("tempPublishSeconds").toInt();

    const uint32_t heartbeatSeconds =
        server_.arg("heartbeatSeconds").toInt();

    if (tempPublishSeconds < 30 ||
        tempPublishSeconds > 3600)
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "Temperature publish interval must be between 30 and 3600 seconds.");

        return;
    }

    if (heartbeatSeconds < 60 ||
        heartbeatSeconds > 86400)
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "Heartbeat interval must be between 60 and 86400 seconds.");

        return;
    }

    DeviceSettings& config =
        settings_->get();

    config.temperaturePublishIntervalSeconds =
        tempPublishSeconds;

    config.heartbeatIntervalSeconds =
        heartbeatSeconds;

    if (!settings_->save())
    {
        logger_->error(
            "Failed to save reporting settings");

        server_.send(
            500,
            "text/plain; charset=utf-8",
            "Failed to save settings.");

        return;
    }

    logger_->info(
        String("Reporting settings saved: temp publish=") +
        String(config.temperaturePublishIntervalSeconds) +
        " sec, heartbeat=" +
        String(config.heartbeatIntervalSeconds) +
        " sec");

    server_.sendHeader(
        "Location",
        "/settings/reporting?saved=1");

    server_.send(
        303,
        "text/plain",
        "");
}

void WebServerService::handleDeviceSettings()
{
    DeviceSettings& config =
        settings_->get();

    WebPageBuilder page;
    if (server_.hasArg("saved"))
{
    if (server_.hasArg("restart"))
    {
        page.addToast(
            "Settings saved. Restart Camper Sentinel to apply the new hostname.",
            StatusLevel::Warning);
    }
    else
    {
        page.addToast(
            "Device settings saved successfully.",
            StatusLevel::Normal);
    }
}
    page.begin(
        PROJECT_NAME,
        "Device Settings",
        "Identity and general configuration.");
    
    page.beginForm(
    "/settings/device",
    "POST");
    page.addNavigation("settings");

    page.addHealthBanner(
    "Camper Sentinel settings",
    "Changes are stored on the device and remain available after a restart.",
    StatusLevel::Neutral);

    page.addSettingsCard(
    "Alerts",
    "Temperature thresholds, shore-power alerts, and SMS notifications.",
    "/settings/alerts");

page.addSettingsCard(
    "Reporting",
    "Control telemetry publishing and heartbeat timing.",
    "/settings/reporting");

page.addSettingsCard(
    "Device",
    "Change the device name, hostname, and general behavior.",
    "/settings/device");

page.addSettingsCard(
    "Connectivity",
    "Manage Wi-Fi, MQTT, and cellular preferences.",
    "/settings/connectivity");

page.addSettingsCard(
    "Firmware",
    "View firmware details and install an OTA update.",
    "/ota");
page.addTextInput(
    "Device Name",
    "deviceName",
    config.deviceName,
    32,
    "Friendly name shown throughout Camper Sentinel.");

page.addTextInput(
    "Hostname",
    "hostname",
    config.hostname,
    32,
    "Local network hostname. Changing this requires a restart.");

page.addCheckbox(
    "Enable Wi-Fi",
    "wifiEnabled",
    config.wifiEnabled,
    "Enable Wi-Fi access, the local dashboard, and OTA updates.");

    page.addSubmitButton(
    "Save Settings");

page.endForm();
    page.addLink(
        "Back to Settings",
        "/settings");

    server_.send(
        200,
        "text/html; charset=utf-8",
        page.build());
}
void WebServerService::handleDeviceSettingsSave()
{
    if (settings_ == nullptr)
    {
        server_.send(
            500,
            "text/plain; charset=utf-8",
            "Settings service is unavailable.");

        return;
    }

    if (!server_.hasArg("deviceName") ||
        !server_.hasArg("hostname"))
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "Required settings are missing.");

        return;
    }

    String deviceName =
        server_.arg("deviceName");

    String hostname =
        server_.arg("hostname");

    deviceName.trim();
    hostname.trim();

    //
    // Validate device name
    //
    if (deviceName.length() == 0 ||
        deviceName.length() > 32)
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "Device name must be between 1 and 32 characters.");

        return;
    }

    //
    // Normalize hostname
    //
    hostname.toLowerCase();
    hostname.replace(" ", "-");

    String cleanHostname;

    for (size_t i = 0;
         i < hostname.length();
         ++i)
    {
        const char c = hostname[i];

        if (
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-')
        {
            cleanHostname += c;
        }
    }

    hostname = cleanHostname;

    if (hostname.length() == 0 ||
        hostname.length() > 32)
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "Hostname must contain between 1 and 32 valid characters.");

        return;
    }

    //
    // Avoid leading/trailing hyphens
    //
    if (hostname.startsWith("-") ||
        hostname.endsWith("-"))
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "Hostname cannot begin or end with a hyphen.");

        return;
    }

    DeviceSettings& config =
        settings_->get();

    const bool hostnameChanged =
        hostname != config.hostname;

    config.deviceName =
        deviceName;

    config.hostname =
        hostname;

    config.wifiEnabled =
        server_.hasArg("wifiEnabled");

    if (!settings_->save())
    {
        logger_->error(
            "Failed to save device settings");

        server_.send(
            500,
            "text/plain; charset=utf-8",
            "Failed to save device settings.");

        return;
    }

    logger_->info(
        String("Device settings saved: name=") +
        config.deviceName +
        ", hostname=" +
        config.hostname +
        ", WiFi=" +
        (config.wifiEnabled
            ? "enabled"
            : "disabled"));

    //
    // Hostname changes require restart.
    //
    if (hostnameChanged)
    {
        server_.sendHeader(
            "Location",
            "/settings/device?saved=1&restart=1");
    }
    else
    {
        server_.sendHeader(
            "Location",
            "/settings/device?saved=1");
    }

    server_.send(
        303,
        "text/plain",
        "");
}
void WebServerService::handleAlertSettings()
{
    if (settings_ == nullptr)
    {
        server_.send(
            500,
            "text/plain; charset=utf-8",
            "Settings service is unavailable.");

        return;
    }

    const DeviceSettings& config =
        settings_->get();

    WebPageBuilder page;

    page.begin(
        PROJECT_NAME,
        "Alert Settings",
        "Configure temperature and notification behavior.");

    page.addNavigation("settings");

    if (server_.hasArg("saved"))
    {
        page.addToast(
            "Alert settings saved successfully.",
            StatusLevel::Normal);
    }

    page.beginCard("Temperature Alerts");

    page.beginForm(
        "/settings/alerts",
        "POST");

    page.addNumberInput(
    "High temperature alarm",
    "tempHigh",
    config.tempAlarmHighF,
    40.0f,
    150.0f,
    0.5f,
    "°F",
    "An alert is triggered after the configured delay when the camper reaches this temperature.");

    page.addNumberInput(
    "Temperature reset point",
    "tempReset",
    config.tempAlarmResetF,
    35.0f,
    145.0f,
    0.5f,
    "°F",
    "The alarm clears only after the temperature falls to or below this value.");

    page.addCheckbox(
        "SMS alerts",
        "smsEnabled",
        config.smsEnabled,
        "Send text messages for high temperature and shore-power events.");

    page.addSubmitButton("Save Settings");

    page.endForm();
    page.addSettingsFormScript();
    page.endCard();

    page.addLink(
        "Back to Settings",
        "/settings");

    page.addFooter(
        PROJECT_VERSION,
        formatUptimeValue(
            appState_ != nullptr
                ? appState_->system.uptimeSeconds
                : 0));

    server_.send(
        200,
        "text/html; charset=utf-8",
        page.build());
}
void WebServerService::handleAlertSettingsSave()
{
    if (settings_ == nullptr)
    {
        server_.send(
            500,
            "text/plain; charset=utf-8",
            "Settings service is unavailable.");

        return;
    }

    if (!server_.hasArg("tempHigh") ||
        !server_.hasArg("tempReset"))
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "Required settings are missing.");

        return;
    }

    const float highTemperature =
        server_.arg("tempHigh").toFloat();

    const float resetTemperature =
        server_.arg("tempReset").toFloat();

    if (highTemperature < 40.0f ||
        highTemperature > 150.0f)
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "High-temperature threshold must be between 40 and 150 F.");

        return;
    }

    if (resetTemperature < 35.0f ||
        resetTemperature >= highTemperature)
    {
        server_.send(
            400,
            "text/plain; charset=utf-8",
            "Reset temperature must be at least 35 F and below the alarm threshold.");

        return;
    }

    DeviceSettings& config =
        settings_->get();

    config.tempAlarmHighF =
        highTemperature;

    config.tempAlarmResetF =
        resetTemperature;

    config.smsEnabled =
        server_.hasArg("smsEnabled");

    if (!settings_->save())
    {
        logger_->error(
            "Failed to save alert settings");

        server_.send(
            500,
            "text/plain; charset=utf-8",
            "Failed to save settings.");

        return;
    }

    logger_->info(
        String("Alert settings saved: high=") +
        String(config.tempAlarmHighF, 1) +
        " F, reset=" +
        String(config.tempAlarmResetF, 1) +
        " F, SMS=" +
        (config.smsEnabled ? "enabled" : "disabled"));

    server_.sendHeader(
        "Location",
        "/settings/alerts?saved=1");

    server_.send(
        303,
        "text/plain",
        "");
}
