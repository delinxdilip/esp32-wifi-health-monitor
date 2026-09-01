#include "logger.h"
#include <stdarg.h>

// ============================================================
// LOGGER INITIALIZATION
// ============================================================

void loggerBegin()
{
    Serial.println();
    Serial.println(
        "=============================="
    );
    Serial.println(
        "[LOGGER] Logger initialized."
    );
    Serial.println(
        "=============================="
    );
}

// ============================================================
// LOG MESSAGE
// ============================================================

void loggerLog(
    LogLevel level,
    const char *source,
    const char *format,
    ...
)
{
    if (level < CURRENT_LOG_LEVEL)
    {
        return;
    }

    const char *levelName;

    switch (level)
    {
        case LOG_LEVEL_DEBUG:
            levelName = "DEBUG";
            break;

        case LOG_LEVEL_INFO:
            levelName = "INFO ";
            break;

        case LOG_LEVEL_WARN:
            levelName = "WARN ";
            break;

        case LOG_LEVEL_ERROR:
            levelName = "ERROR";
            break;

        default:
            levelName = "?????";
            break;
    }

    Serial.print("[");
    Serial.print(levelName);
    Serial.print("] [");
    Serial.print(source);
    Serial.print("] ");

    va_list args;

    va_start(args, format);

    char buffer[256];

    vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args
    );

    va_end(args);

    Serial.println(buffer);
}