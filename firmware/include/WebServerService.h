#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include "Web/DashboardPage.h"
#include "Web/SettingsPage.h"
#include "Web/OTAPage.h"
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
    SettingsPage settingsPage_;
    bool started_ = false;
    void registerRoutes();
    void handleNotFound();
    void handleOtaPage();
    void handleOtaUpload();
    void handleOtaUploadComplete();
    // void handleSettings();
    void handleDeviceSettings();
    void handleDeviceSettingsSave();
    bool otaUploadStarted_ = false;
    bool otaUploadSuccessful_ = false;
    size_t otaBytesWritten_ = 0;
    DashboardPage dashboardPage_;
    OTAPage otaPage_;
};