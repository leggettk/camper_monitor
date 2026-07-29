#include "WiFiService.h"
AppState* appState_= nullptr;

bool WiFiService::begin(Settings& settings, AppState& appState, Logger& logger)
{
    settings_ = &settings;
    logger_ = &logger;
    appState_ = &appState;

    WiFi.mode(WIFI_STA);

    started_ = true;

    connect();

    return true;
}
void WiFiService::update()
{
    if (!started_)
    {
        return;
    }

    const bool isConnected = connected();

    if (isConnected && !wasConnected_)
    {
        if (logger_ != nullptr)
        {
            logger_->info("WiFi connected");

            logger_->info(
                String("WiFi SSID: ") +
                ssid());

            logger_->info(
                String("WiFi IP: ") +
                ipAddress());

            logger_->info(
                String("WiFi RSSI: ") +
                String(rssi()) +
                " dBm");
        }
        appState_->wifi.connected = true;
        appState_->wifi.ssid = WiFi.SSID();
        appState_->wifi.ipAddress =
        WiFi.localIP().toString();
        appState_->wifi.rssi = WiFi.RSSI();
    }
    else if (!isConnected && wasConnected_)
    {
        if (logger_ != nullptr)
        {
            logger_->warning("WiFi disconnected");
        }
        appState_->wifi.connected = false;
        appState_->wifi.ipAddress = "No IP";
        appState_->wifi.rssi = 0;
    }

    wasConnected_ = isConnected;

    if (isConnected)
    {
        if (WiFi.status() == WL_CONNECTED &&
    appState_ != nullptr)
{
    appState_->wifi.connected = true;
    appState_->wifi.ssid = WiFi.SSID();
    appState_->wifi.ipAddress =
        WiFi.localIP().toString();
    appState_->wifi.rssi = WiFi.RSSI();
}
        return;
    }

    const uint32_t now = millis();

    if ((now - lastReconnectAttemptMs_) <
        WIFI_RECONNECT_INTERVAL_MS)
    {
        return;
    }

    lastReconnectAttemptMs_ = now;
    connect();
}

void WiFiService::connect()
{
    if (settings_ == nullptr)
    {
        return;
    }

    if (!enabled() || !configured())
    {
        return;
    }

    const DeviceSettings& config = settings_->get();

    WiFi.setHostname(config.hostname.c_str());

    if (logger_ != nullptr)
    {
        logger_->info(
            String("WiFi: Connecting to ") +
            config.wifiSSID);
    }

    WiFi.begin(
        config.wifiSSID.c_str(),
        config.wifiPassword.c_str()
    );

    connectionAttempted_ = true;
    lastReconnectAttemptMs_ = millis();
}
bool WiFiService::connected() const
{
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiService::configured() const
{
    if (settings_ == nullptr)
    {
        return false;
    }

    const DeviceSettings& config = settings_->get();

    return !config.wifiSSID.isEmpty();
}

bool WiFiService::enabled() const
{
    if (settings_ == nullptr)
    {
        return false;
    }

    return settings_->get().wifiEnabled;
}
String WiFiService::ssid() const
{
    if (!connected())
    {
        return "";
    }

    return WiFi.SSID();
}

String WiFiService::ipAddress() const
{
    if (!connected())
    {
        return "";
    }

    return WiFi.localIP().toString();
}

int32_t WiFiService::rssi() const
{
    if (!connected())
    {
        return 0;
    }

    return WiFi.RSSI();
}