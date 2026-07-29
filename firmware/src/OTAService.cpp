#include "OTAService.h"

#include <Update.h>

#include "Logger.h"

bool OTAService::begin(Logger& logger)
{
    logger_ = &logger;
    return true;
}

bool OTAService::inProgress() const
{
    return uploadInProgress_;
}

bool OTAService::successful() const
{
    return uploadSuccessful_;
}

size_t OTAService::bytesWritten() const
{
    return bytesWritten_;
}

String OTAService::lastError() const
{
    return lastError_;
}

void OTAService::setError(const String& error)
{
    lastError_ = error;

    if (logger_)
        logger_->error(error);
}

bool OTAService::beginUpload()
{
    uploadInProgress_ = false;
    uploadSuccessful_ = false;
    bytesWritten_ = 0;
    expectedSize_ = 0;
    lastProgress_ = -1;
    lastError_.clear();

    if (logger_)
        logger_->info("OTA upload started");

    const size_t availableSpace =
        (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

    if (logger_)
    {
        logger_->info(
            "OTA available space: "
            + String(availableSpace / 1024)
            + " KB");
    }

    if (availableSpace == 0)
    {
        setError("No OTA update space available");
        return false;
    }

    if (!Update.begin(availableSpace, U_FLASH))
    {
        setError(
            "Unable to begin OTA: "
            + String(Update.errorString()));
        return false;
    }

    uploadInProgress_ = true;
    return true;
}

bool OTAService::writeChunk(
    uint8_t* data,
    size_t length)
{
    if (!uploadInProgress_)
        return false;

    if (Update.write(data, length) != length)
    {
        setError(Update.errorString());
        return false;
    }

bytesWritten_ += length;

if (logger_ && bytesWritten_ / 65536 !=
                   (bytesWritten_ - length) / 65536)
{
    logger_->info(
        "OTA written: "
        + String(bytesWritten_ / 1024)
        + " KB");
}

    return true;
}

bool OTAService::finishUpload()
{
    uploadInProgress_ = false;

    if (logger_)
        logger_->info("Verifying firmware...");

    if (!Update.end(true))
    {
        setError("Unable to finish OTA: " + String(Update.errorString()));
        return false;
    }

    uploadSuccessful_ = true;

    if (logger_)
        logger_->info("OTA successful");

    return true;
}

void OTAService::abortUpload()
{
    Update.abort();

    uploadInProgress_ = false;
    uploadSuccessful_ = false;

    setError("OTA aborted");
}