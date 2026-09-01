#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>

void ledManagerBegin();

void ledOff();

void ledRed();

void ledGreen();

void ledBlue();

void ledWhite();

void ledYellow();

void ledCyan();

void ledMagenta();

void ledOrange();

void ledBlinkWhite(uint8_t times);

void ledBlinkRed(uint8_t times);

void ledBlinkGreen(uint8_t times);

void ledBlinkBlue(uint8_t times);

void ledBlinkYellow(uint8_t times);

void ledBlinkCyan(uint8_t times);

void ledBlinkMagenta(uint8_t times);

void ledBlinkOrange(uint8_t times);

#endif