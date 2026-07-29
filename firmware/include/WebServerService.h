#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "AppState.h"
#include "Logger.h"
#include "Settings.h"
#include "OTAService.h"

class WebServerService
{
public:
    bool begin(
        Settings& settings,
        AppState& appState,
        Logger& logger,
        OTAService& otaService);

    void update();

    bool running() const;

private:
    WebServer server_{80};
    OTAService* otaService_ = nullptr;
    Settings* settings_ = nullptr;
    AppState* appState_ = nullptr;
    Logger* logger_ = nullptr;
    String formatTemperature() const;
    String formatBatteryVoltage() const;
    String formatUptime(uint32_t totalSeconds) const;
    String formatFreeHeap(uint32_t bytes) const;
    bool started_ = false;

    void registerRoutes();

    void handleHome();
    void handleNotFound();
    void handleOtaPage();
    void handleOtaUpload();
    void handleOtaUploadComplete();

    bool otaUploadStarted_ = false;
    bool otaUploadSuccessful_ = false;
    size_t otaBytesWritten_ = 0;
};