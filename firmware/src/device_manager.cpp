#include "device_manager.h"

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
    Serial.println();
    Serial.println("==============================");
    Serial.println("[DEVICE] Device identity");
    Serial.println("==============================");

    Serial.print("[DEVICE] ID: ");
    Serial.println(DEVICE_ID);

    Serial.print("[DEVICE] Name: ");
    Serial.println(DEVICE_NAME);

    Serial.print("[DEVICE] Model: ");
    Serial.println(DEVICE_MODEL);
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