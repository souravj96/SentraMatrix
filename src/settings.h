#pragma once

#include <Arduino.h>
#include "config.h"

struct Settings
{
    uint16_t magic;

    // Hardware
    uint8_t dataPin;
    uint8_t clkPin;
    uint8_t csPin;
    uint8_t hardwareType;
    uint8_t deviceCount;
    uint8_t brightness;

    // Time
    uint8_t useNTP;
    uint8_t use24Hour;
    uint8_t dateFormat;
    
    int32_t gmtOffsetSec;
    int32_t daylightOffsetSec;
    
    char ntpServer1[33];
    char ntpServer2[33];
    char ntpServer3[33];

    // MQTT
    uint8_t useMQTT;
    char mqttBroker[65];
    uint16_t mqttPort;
    char mqttUser[33];
    char mqttPassword[65];
    char mqttClientId[33];
    char mqttSubTopic[65];
    char mqttPubTopic[65];
    uint16_t mqttPubInterval;

    uint8_t reserved[16];
};

extern Settings settings;

void loadSettings();
void saveSettings();
void resetSettings();
