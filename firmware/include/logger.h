#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// ============================================================
// LOG LEVELS
// ============================================================

enum LogLevel
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
};

// ============================================================
// CONFIGURATION
// ============================================================

#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG

// ============================================================
// LOGGER
// ============================================================

void loggerBegin();

void loggerLog(
    LogLevel level,
    const char *source,
    const char *format,
    ...
);

// ============================================================
// CONVENIENCE MACROS
// ============================================================

#define LOG_DEBUG(source, format, ...) \
    loggerLog(LOG_LEVEL_DEBUG, source, format, ##__VA_ARGS__)

#define LOG_INFO(source, format, ...) \
    loggerLog(LOG_LEVEL_INFO, source, format, ##__VA_ARGS__)

#define LOG_WARN(source, format, ...) \
    loggerLog(LOG_LEVEL_WARN, source, format, ##__VA_ARGS__)

#define LOG_ERROR(source, format, ...) \
    loggerLog(LOG_LEVEL_ERROR, source, format, ##__VA_ARGS__)

#endif