#pragma once

#include <WebServer.h>

#include "Logger.h"
#include "Settings.h"

class WebServerService
{
public:
    bool begin(Settings& settings, Logger& logger);

    void update();

    bool running() const;

private:
    WebServer server_{80};

    Settings* settings_ = nullptr;
    Logger* logger_ = nullptr;

    bool started_ = false;

    void registerRoutes();

    void handleHome();
};