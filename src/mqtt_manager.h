#pragma once

#include <Arduino.h>

void mqttInit();
void mqttLoop();
void mqttPublishStatus();
void mqttPublishDiscovery();
bool mqttIsConnected();

