#include "led_manager.h"
#include "device_config.h"
#include "logger.h"

#include <Adafruit_NeoPixel.h>

// ============================================================
// ONBOARD RGB LED
// ============================================================

Adafruit_NeoPixel rgbLed(
    DEVICE_RGB_LED_COUNT,
    DEVICE_RGB_LED_PIN,
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
    rgbLed.setBrightness(DEVICE_RGB_BRIGHTNESS);

    ledOff();

    LOG_INFO(
        "LED",
        "RGB LED initialized."
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

void ledYellow()
{
    setLED(255, 255, 0);
}

void ledCyan()
{
    setLED(0, 255, 255);
}

void ledMagenta()
{
    setLED(255, 0, 255);
}

void ledOrange()
{
    setLED(255, 165, 0);
}


// ============================================================
// BLINK FUNCTIONS
// ============================================================

void ledBlinkWhite(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        ledWhite();
        delay(300);

        ledOff();
        delay(300);
    }
}

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

void ledBlinkYellow(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        ledYellow();
        delay(300);

        ledOff();
        delay(300);
    }
}

void ledBlinkCyan(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        ledCyan();
        delay(300);

        ledOff();
        delay(300);
    }
}

void ledBlinkMagenta(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        ledMagenta();
        delay(300);

        ledOff();
        delay(300);
    }
}

void ledBlinkOrange(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        ledOrange();
        delay(300);

        ledOff();
        delay(300);
    }
}