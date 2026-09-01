#include "time_manager.h"

#include <time.h>
#include "logger.h"
#include "led_manager.h"

// ============================================================
// NTP CONFIGURATION
// ============================================================

static const char* NTP_SERVER_1 = "pool.ntp.org";
static const char* NTP_SERVER_2 = "time.nist.gov";

static bool timeSynced = false;


// ============================================================
// INITIALIZE TIME
// ============================================================

void timeManagerBegin()
{
    LOG_INFO(
        "TIME",
        "Starting NTP synchronization..."
    );

    ledBlinkBlue(2);

    configTime(
        0,
        0,
        NTP_SERVER_1,
        NTP_SERVER_2
    );

    struct tm timeInfo;

    if (getLocalTime(&timeInfo, 10000))
    {
        timeSynced = true;

        LOG_INFO(
            "TIME",
            "NTP synchronized."
        );

        LOG_INFO(
            "TIME",
            "UTC: %s",
            getTimestamp().c_str()
        );

        ledGreen();
    }
    else
    {
        timeSynced = false;

        LOG_ERROR(
            "TIME",
            "NTP synchronization failed."
        );

        ledRed();
    }
}


// ============================================================
// CHECK SYNCHRONIZATION
// ============================================================

bool isTimeSynchronized()
{
    return timeSynced;
}


// ============================================================
// GET UTC TIMESTAMP
// ============================================================

String getTimestamp()
{
    struct tm timeInfo;

    if (!getLocalTime(&timeInfo, 1000))
    {
        return "";
    }

    char buffer[32];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%dT%H:%M:%SZ",
        &timeInfo
    );

    return String(buffer);
}