#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <Arduino.h>

// ============================================================
// DEVICE IDENTITY
// ============================================================

void deviceManagerBegin();
const char* getDeviceId();
const char* getDeviceName();
const char* getDeviceModel();

#endif