#pragma once

#include <Arduino.h>

void displayInit();
void displayStatic(const char* text);
void displayScroll(const char* text);
void displayClock(const char* text, uint8_t effectIn, uint8_t effectOut);
void displayDate(const char* text);
void displayClear();
void displayLoop();
bool displayIsAnimationFinished();
