#pragma once

#include <Arduino.h>

void clockInit();

bool clockIsValid();

void clockUpdate();

String getCurrentTime();

String getCurrentDate();

void clockShowMessage(const String& msg);
