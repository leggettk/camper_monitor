#pragma once

#include <Arduino.h>

class Logger;

class OTAService
{
public:
    OTAService() = default;

    bool begin(Logger& logger);

    bool beginUpload();

    bool writeChunk(
        uint8_t* data,
        size_t length);

    bool finishUpload();

    void abortUpload();

    bool inProgress() const;
    bool successful() const;

    size_t bytesWritten() const;
    String lastError() const;

private:
    Logger* logger_ = nullptr;

    bool uploadInProgress_ = false;
    bool uploadSuccessful_ = false;

    size_t bytesWritten_ = 0;
    size_t expectedSize_ = 0;

    int lastProgress_ = -1;

    String lastError_;

    void setError(const String& error);
};