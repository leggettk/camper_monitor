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

    settingsPage_.begin(
    server_,
    settings,
    appState,
    logger);

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
        settingsPage_.handleLanding();
    });

    server_.on(
    "/settings/alerts",
    HTTP_GET,
    [this]()
    {
        settingsPage_.handleAlerts();
    });

    server_.on(
    "/settings/alerts",
    HTTP_POST,
    [this]()
    {
        settingsPage_.handleAlertsSave();
    });

    server_.on(
    "/settings/reporting",
    HTTP_GET,
    [this]()
    {
        settingsPage_.handleReporting();
    });

    server_.on(
    "/settings/device",
    HTTP_GET,
    [this]()
    {
        settingsPage_.handleDevice();
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
            settingsPage_.handleAlertsSave();
        });
    });
server_.on(
    "/settings/reporting",
    HTTP_GET,
    [this]()
    {
        settingsPage_.handleReporting();
    });

server_.on(
    "/settings/reporting",
    HTTP_POST,
    [this]()
    {
        settingsPage_.handleReportingSave();
    });
server_.on(
    "/settings/device",
    HTTP_GET,
    [this]()
    {
        settingsPage_.handleDevice();
    });

server_.on(
    "/settings/device",
    HTTP_POST,
    [this]()
    {
        settingsPage_.handleDeviceSave();
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




