#include "led_manager.h"

#include <Adafruit_NeoPixel.h>

// ============================================================
// ONBOARD RGB LED
// ============================================================

#define RGB_LED_PIN   38
#define RGB_LED_COUNT 1

Adafruit_NeoPixel rgbLed(
    RGB_LED_COUNT,
    RGB_LED_PIN,
    NEO_GRB + NEO_KHZ800
);

// ============================================================
// INTERNAL FUNCTION
// ============================================================

void setLED(
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    rgbLed.setPixelColor(
        0,
        rgbLed.Color(
            red,
            green,
            blue
        )
    );

    rgbLed.show();
}

// ============================================================
// INITIALIZATION
// ============================================================

void ledManagerBegin()
{
    rgbLed.begin();

    // Keep the onboard LED reasonably dim.
    rgbLed.setBrightness(40);

    ledOff();

    Serial.println(
        "[LED] RGB LED initialized."
    );
}

// ============================================================
// BASIC COLORS
// ============================================================

void ledOff()
{
    setLED(0, 0, 0);
}

void ledRed()
{
    setLED(255, 0, 0);
}

void ledGreen()
{
    setLED(0, 255, 0);
}

void ledBlue()
{
    setLED(0, 0, 255);
}

void ledWhite()
{
    setLED(255, 255, 255);
}

// ============================================================
// BLINK FUNCTIONS
// ============================================================

void ledBlinkRed(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        ledRed();
        delay(300);

        ledOff();
        delay(300);
    }
}

void ledBlinkGreen(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        ledGreen();
        delay(300);

        ledOff();
        delay(300);
    }
}

void ledBlinkBlue(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        ledBlue();
        delay(300);

        ledOff();
        delay(300);
    }
}