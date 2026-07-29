#include "WebServerService.h"
#include "WebPageBuilder.h"
#include "Constants.h"
#include <Update.h>

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
            handleHome();
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
}

void WebServerService::handleHome()
{
    if (appState_ == nullptr)
    {
        server_.send(
            500,
            "text/plain; charset=utf-8",
            "Application state is unavailable.");

        return;
    }

    WebPageBuilder page;

    page.begin(
        PROJECT_NAME,
        PROJECT_NAME,
        String("Know your camper. Anywhere. · ") + PROJECT_VERSION,
        5);

    page.beginCard("Connectivity");

    page.addStatus(
        "Wi-Fi",
        appState_->wifi.connected
            ? appState_->wifi.ssid
            : "Disconnected",
        appState_->wifi.connected);

    page.addStatus(
        "Wi-Fi IP",
        appState_->wifi.connected
            ? appState_->wifi.ipAddress
            : "No IP",
        appState_->wifi.connected);

    page.addStatus(
        "LTE Network",
        appState_->cellular.networkConnected
            ? appState_->cellular.operatorName
            : "Disconnected",
        appState_->cellular.networkConnected);

    page.addStatus(
        "LTE Data",
        appState_->cellular.dataConnected
            ? appState_->cellular.ipAddress
            : "Disconnected",
        appState_->cellular.dataConnected);

    page.addStatus(
        "MQTT",
        appState_->mqtt.connected
            ? "Connected"
            : "Disconnected",
        appState_->mqtt.connected);

    page.endCard();

    page.beginCard("Sensors");

    page.addStatus(
        "Ambient Temperature",
        formatTemperature(),
        appState_->temperature.valid &&
            !appState_->alarms.highTemperature);

    page.addStatus(
        "Battery",
        formatBatteryVoltage(),
        appState_->power.batteryVoltageValid);

    page.addStatus(
        "Shore Power",
        appState_->power.shorePowerPresent
            ? "Present"
            : "Not Present",
        appState_->power.shorePowerPresent &&
            !appState_->alarms.shorePower);

    page.endCard();

    page.beginCard("System");

    page.addStatus(
        "Uptime",
        formatUptime(appState_->system.uptimeSeconds),
        true);

    page.addStatus(
        "Free Heap",
        formatFreeHeap(appState_->system.freeHeap),
        appState_->system.freeHeap > 20000);

    page.addStatus(
        "Last SMS",
        appState_->sms.lastSuccessful
            ? "Successful"
            : "None / Failed",
        appState_->sms.lastSuccessful);

    page.endCard();

    server_.send(
        200,
        "text/html; charset=utf-8",
        page.build());
}

String WebServerService::formatTemperature() const
{
    if (appState_ == nullptr ||
        !appState_->temperature.valid)
    {
        return "Unavailable";
    }

    return String(
        appState_->temperature.ambientF,
        1) + " °F";
}

String WebServerService::formatBatteryVoltage() const
{
    if (appState_ == nullptr ||
        !appState_->power.batteryVoltageValid)
    {
        return "Unavailable";
    }

    return String(
        appState_->power.batteryVoltage,
        2) + " V";
}

String WebServerService::formatUptime(
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

String WebServerService::formatFreeHeap(
    uint32_t bytes) const
{
    const float kilobytes =
        static_cast<float>(bytes) / 1024.0f;

    return String(kilobytes, 1) + " KB";
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