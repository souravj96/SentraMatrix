#pragma once

#include <Arduino.h>

bool wifiConnect();

String getIPAddress();
int getRSSI();

String wifiScan();
bool wifiConnectTo(const String& ssid, const String& password);