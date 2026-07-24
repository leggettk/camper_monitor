#include "WiFiService.h"

bool WiFiService::begin(Settings& settings)
{
    settings_ = &settings;

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
        Serial.println("WiFi connected!");

        Serial.print("SSID: ");
        Serial.println(ssid());

        Serial.print("IP Address: ");
        Serial.println(ipAddress());

        Serial.print("RSSI: ");
        Serial.print(rssi());
        Serial.println(" dBm");
    }
    else if (!isConnected && wasConnected_)
    {
        Serial.println("WiFi disconnected");
    }

    wasConnected_ = isConnected;

    if (isConnected)
    {
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

    Serial.print("WiFi: Connecting to ");
    Serial.println(config.wifiSSID);

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