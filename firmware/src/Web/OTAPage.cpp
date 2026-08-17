#include "Web/OTAPage.h"

#include "Version.h"

void OTAPage::begin(
    WebServer& server,
    AppState& state,
    Logger& logger,
    OTAService& otaService)
{
    server_ = &server;
    state_ = &state;
    logger_ = &logger;
    otaService_ = &otaService;
}

String OTAPage::formatUptime(
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

void OTAPage::handlePage()
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

    server_->send(
        200,
        "text/html; charset=utf-8",
        html);
}

void OTAPage::handleUpload()
{
    HTTPUpload& upload = server_->upload();

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

void OTAPage::handleUploadComplete()
{
    if (otaService_->successful())
    {
        server_->send(
            200,
            "text/html",
            "<h2>Firmware updated successfully.<br>"
            "Rebooting...</h2>");

        delay(1000);

        ESP.restart();
    }
    else
    {
        server_->send(
            500,
            "text/plain",
            otaService_->lastError());
    }
}