#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>

void ledManagerBegin();

void ledOff();

void ledRed();

void ledGreen();

void ledBlue();

void ledWhite();

void ledBlinkRed(uint8_t times);

void ledBlinkGreen(uint8_t times);

void ledBlinkBlue(uint8_t times);

#endif