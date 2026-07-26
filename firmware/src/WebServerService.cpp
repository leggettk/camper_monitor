#include "WebServerService.h"

bool WebServerService::begin(Settings& settings, Logger& logger)
{
    settings_ = &settings;
    logger_ = &logger;

    registerRoutes();

    server_.begin();

    started_ = true;

    logger_->info("Web server started");

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
    server_.on("/", [this]()
    {
        handleHome();
    });
}

void WebServerService::handleHome()
{
    server_.send(
        200,
        "text/plain",
        "Camper Sentinel");
}