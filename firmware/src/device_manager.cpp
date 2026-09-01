#include "device_manager.h"
#include "logger.h"

// ============================================================
// DEVICE CONFIGURATION
// ============================================================
//
// Change these values for each physical ESP32.
//
// Example:
//
// Device 1:
//   ID   = ESP32-001
//   Name = Living Room Monitor
//
// Device 2:
//   ID   = ESP32-002
//   Name = Bedroom Monitor
//
// ============================================================

static const char* DEVICE_ID =
    "ESP32-001";

static const char* DEVICE_NAME =
    "Living Room Monitor";

static const char* DEVICE_MODEL =
    "ESP32-S3-N16R8";


// ============================================================
// INITIALIZATION
// ============================================================

void deviceManagerBegin()
{
    LOG_INFO(
        "DEVICE",
        "Device manager initialized."
    );
    LOG_INFO(
        "DEVICE",
        "Device ID: %s",
        DEVICE_ID
    );
    LOG_INFO(
        "DEVICE",
        "Device Name: %s",
        DEVICE_NAME
    );
    LOG_INFO(
        "DEVICE",
        "Device Model: %s",
        DEVICE_MODEL
    );
}


// ============================================================
// GET DEVICE ID
// ============================================================

const char* getDeviceId()
{
    return DEVICE_ID;
}


// ============================================================
// GET DEVICE NAME
// ============================================================

const char* getDeviceName()
{
    return DEVICE_NAME;
}


// ============================================================
// GET DEVICE MODEL
// ============================================================

const char* getDeviceModel()
{
    return DEVICE_MODEL;
}