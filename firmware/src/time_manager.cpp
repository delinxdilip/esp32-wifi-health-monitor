#include "time_manager.h"

#include <time.h>

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
    Serial.println();
    Serial.println("[TIME] Starting NTP synchronization...");

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

        Serial.println("[TIME] NTP synchronized.");

        Serial.print("[TIME] UTC: ");
        Serial.println(getTimestamp());
    }
    else
    {
        timeSynced = false;

        Serial.println(
            "[TIME] NTP synchronization failed."
        );
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