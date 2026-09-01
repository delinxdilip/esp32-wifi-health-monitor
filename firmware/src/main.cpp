#include <Arduino.h>

#include "led_manager.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "time_manager.h"
#include "firebase_manager.h"
#include "device_manager.h"
#include "logger.h"


void setup()
{
    Serial.begin(115200);

    delay(2000);

    loggerBegin();


    Serial.println();
    Serial.println("==============================");
    Serial.println("ESP32 WiFi Health Monitor");
    Serial.println("Booting...");
    Serial.println("==============================");


    deviceManagerBegin();

    ledManagerBegin();
    ledBlinkOrange(3);
    delay(1000);

    wifiManagerBegin();
    ledBlinkBlue(3);
    delay(1000);

    timeManagerBegin();
    ledBlinkGreen(3);
    delay(1000);

    firebaseManagerBegin();
    ledBlinkWhite(3);
    delay(1000);

    webServerBegin();
    ledBlinkYellow(3);
    delay(1000);


    Serial.println();
    Serial.println("==============================");
    Serial.println("ESP32 initialization complete");
    Serial.println("==============================");


    if (isWifiConnected())
    {
        ledGreen();
    }
}


void loop()
{

    ledOff();
    delay(1000);
    wifiManagerLoop();

    firebaseManagerLoop();

    webServerLoop();

    delay(10);
}