#pragma once

#include <Arduino.h>

// ArduinoOTA (espota): network upload from PlatformIO / Arduino IDE
void otaInit();
void otaLoop();

// mDNS / OTA hostname, e.g. "sentramatrix-a1b2c3"
String otaHostname();
