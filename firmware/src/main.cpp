#include <Arduino.h>

#include "led_manager.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "time_manager.h"
#include "firebase_manager.h"
#include "device_manager.h"


void setup()
{
    Serial.begin(115200);

    delay(2000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("ESP32 WiFi Health Monitor");
    Serial.println("Booting...");
    Serial.println("==============================");

    deviceManagerBegin();


    // Blue = booting
    ledManagerBegin();
    ledBlue();


    // Wi-Fi
    Serial.println(
        "[MAIN] Starting Wi-Fi manager..."
    );

    wifiManagerBegin();

    Serial.println(
        "[MAIN] Wi-Fi manager initialized."
    );


    timeManagerBegin();

    firebaseManagerBegin();


    // Web server
    Serial.println(
        "[MAIN] Starting web server..."
    );

    webServerBegin();

    Serial.println(
        "[MAIN] Web server initialized."
    );


    Serial.println();
    Serial.println("==============================");
    Serial.println("ESP32 initialization complete");
    Serial.println("==============================");


    // Green = system ready
    if (isWifiConnected())
    {
        ledGreen();
    }
}


void loop()
{
    wifiManagerLoop();

    firebaseManagerLoop();

    webServerLoop();

    delay(10);
}