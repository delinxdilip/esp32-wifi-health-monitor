#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// Initialize Wi-Fi manager.
void wifiManagerBegin();

// Background Wi-Fi processing.
void wifiManagerLoop();

// Current Wi-Fi connection state.
bool isWifiConnected();

// Whether ESP32 is currently in provisioning mode.
bool isWifiProvisioning();

// Start Wi-Fi provisioning mode.
void wifiManagerStartProvisioning();

// Test new credentials and save them only if connection succeeds.
bool wifiManagerConfigure(
    const String &ssid,
    const String &password
);

#endif