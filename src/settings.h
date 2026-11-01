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

    uint8_t useHADiscovery;
    uint8_t quietHoursEnabled;
    uint8_t quietStartHour;
    uint8_t quietEndHour;
    uint8_t quietBrightness;

    // Text effects (MD_Parola). Effect fields store an index into
    // the table in effects.cpp. Uses space previously reserved, so
    // the EEPROM layout of all other fields is unchanged.
    uint8_t effectsVersion;     // EFFECTS_VERSION when initialized
    uint8_t clockEffect;        // Clock entry effect (after date/message)
    uint8_t dateEffectIn;       // Day/date entry effect
    uint8_t dateEffectOut;      // Day/date exit effect
    uint8_t messageEffectIn;    // MQTT message entry effect
    uint8_t messageEffectOut;   // MQTT message exit effect
    uint8_t effectSpeed;        // Animation frame delay in ms (lower = faster)
    uint8_t textPause;          // Seconds day/date/message stay before exit
    uint8_t invertDisplay;      // 1 = inverted pixels
    uint8_t reserved[2];
};

extern Settings settings;

void loadSettings();
void saveSettings();
void resetSettings();
void resetEffectSettings();
