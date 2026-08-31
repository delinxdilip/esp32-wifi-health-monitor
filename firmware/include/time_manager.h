#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>

void timeManagerBegin();
bool isTimeSynchronized();
String getTimestamp();

#endif