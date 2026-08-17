#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "AppState.h"
#include "Logger.h"
#include "OTAService.h"
#include "WebPageBuilder.h"

class OTAPage
{
public:
    void begin(
        WebServer& server,
        AppState& state,
        Logger& logger,
        OTAService& otaService);

    void handlePage();
    void handleUpload();
    void handleUploadComplete();
    String formatUptime (uint32_t totalSeconds) const;
private:
    WebServer* server_ = nullptr;
    AppState* state_ = nullptr;
    Logger* logger_ = nullptr;
    OTAService* otaService_ = nullptr;
};