#include "settings.h"
#include <EEPROM.h>
#include <string.h>

#include "effects.h"

Settings settings;

void resetEffectSettings()
{
    settings.effectsVersion = EFFECTS_VERSION;
    settings.clockEffect = effectIndexOf(PA_SCROLL_UP);
    settings.dateEffectIn = effectIndexOf(PA_SCROLL_LEFT);
    settings.dateEffectOut = effectIndexOf(PA_SCROLL_LEFT);
    settings.messageEffectIn = effectIndexOf(PA_SCROLL_LEFT);
    settings.messageEffectOut = effectIndexOf(PA_SCROLL_LEFT);
    settings.effectSpeed = DEFAULT_EFFECT_SPEED;
    settings.textPause = DEFAULT_TEXT_PAUSE;
    settings.invertDisplay = 0;
}

void resetOtaSettings()
{
    settings.otaVersion = OTA_SETTINGS_VERSION;
    memset(settings.otaPassword, 0, sizeof(settings.otaPassword));
}

static void normalizeOtaSettings()
{
    if (settings.otaVersion != OTA_SETTINGS_VERSION)
    {
        resetOtaSettings();
        return;
    }
    settings.otaPassword[sizeof(settings.otaPassword) - 1] = '\0';
}

static void normalizeEffectSettings()
{
    if (settings.effectsVersion != EFFECTS_VERSION)
    {
        // Upgrade from a version without effect settings
        resetEffectSettings();
        return;
    }

    if (!effectValidIn(settings.clockEffect)) settings.clockEffect = effectIndexOf(PA_SCROLL_UP);
    if (!effectValidIn(settings.dateEffectIn)) settings.dateEffectIn = effectIndexOf(PA_SCROLL_LEFT);
    if (!effectValidOut(settings.dateEffectOut)) settings.dateEffectOut = effectIndexOf(PA_SCROLL_LEFT);
    if (!effectValidIn(settings.messageEffectIn)) settings.messageEffectIn = effectIndexOf(PA_SCROLL_LEFT);
    if (!effectValidOut(settings.messageEffectOut)) settings.messageEffectOut = effectIndexOf(PA_SCROLL_LEFT);
    if (settings.effectSpeed < MIN_EFFECT_SPEED || settings.effectSpeed > MAX_EFFECT_SPEED) settings.effectSpeed = DEFAULT_EFFECT_SPEED;
    if (settings.textPause > MAX_TEXT_PAUSE) settings.textPause = DEFAULT_TEXT_PAUSE;
    if (settings.invertDisplay > 1) settings.invertDisplay = 0;
}

void resetSettings()
{
    memset(&settings, 0, sizeof(settings));

    settings.magic = SETTINGS_MAGIC;

    // Hardware defaults
    settings.dataPin = MATRIX_DATA_PIN;
    settings.clkPin = MATRIX_CLK_PIN;
    settings.csPin = MATRIX_CS_PIN;
    settings.hardwareType = static_cast<uint8_t>(DEFAULT_HARDWARE_TYPE);
    settings.deviceCount = DEFAULT_DEVICE_COUNT;
    settings.brightness = DEFAULT_BRIGHTNESS;

    // Time defaults
    settings.useNTP = 1;
    settings.use24Hour = 1;
    settings.dateFormat = 0; // 0: DD/MM/YYYY, 1: MM/DD/YYYY, etc
    
    settings.gmtOffsetSec = GMT_OFFSET_SEC;
    settings.daylightOffsetSec = DAYLIGHT_OFFSET_SEC;
    
    strncpy(settings.ntpServer1, "pool.ntp.org", sizeof(settings.ntpServer1) - 1);
    strncpy(settings.ntpServer2, "time.nist.gov", sizeof(settings.ntpServer2) - 1);
    strncpy(settings.ntpServer3, "time.google.com", sizeof(settings.ntpServer3) - 1);

    // MQTT defaults
    settings.useMQTT = 0;
    strncpy(settings.mqttBroker, "192.168.1.100", sizeof(settings.mqttBroker) - 1);
    settings.mqttPort = 1883;
    strncpy(settings.mqttUser, "", sizeof(settings.mqttUser) - 1);
    strncpy(settings.mqttPassword, "", sizeof(settings.mqttPassword) - 1);
    strncpy(settings.mqttClientId, "SentraMatrix", sizeof(settings.mqttClientId) - 1);
    strncpy(settings.mqttSubTopic, "sentramatrix/message", sizeof(settings.mqttSubTopic) - 1);
    strncpy(settings.mqttPubTopic, "sentramatrix/status", sizeof(settings.mqttPubTopic) - 1);
    settings.mqttPubInterval = 60;
    settings.useHADiscovery = 1;

    // Quiet Hours defaults
    settings.quietHoursEnabled = 0;
    settings.quietStartHour = 22; // 10 PM
    settings.quietEndHour = 7;    // 7 AM
    settings.quietBrightness = 0; // 0 = Display Off

    // Effect defaults (same behavior as before effects were configurable)
    resetEffectSettings();

    // OTA defaults (no password)
    resetOtaSettings();
}

void loadSettings()
{
    EEPROM.begin(EEPROM_SIZE);

    EEPROM.get(0, settings);

    if (settings.magic != SETTINGS_MAGIC)
    {
        Serial.println("No valid settings found.");
        Serial.println("Loading defaults...");

        resetSettings();
        saveSettings();
    }
    else
    {
        Serial.println("Settings loaded from EEPROM.");

        // Normalize if upgraded from previous version
        if (settings.useHADiscovery > 1) settings.useHADiscovery = 1;
        if (settings.quietHoursEnabled > 1) settings.quietHoursEnabled = 0;
        if (settings.quietStartHour > 23) settings.quietStartHour = 22;
        if (settings.quietEndHour > 23) settings.quietEndHour = 7;
        if (settings.quietBrightness > 15) settings.quietBrightness = 0;
        normalizeEffectSettings();
        normalizeOtaSettings();

        Serial.println();
        Serial.println("----- Display Settings -----");
        Serial.print("DATA pin: "); Serial.println(settings.dataPin);
        Serial.print("CLK pin: "); Serial.println(settings.clkPin);
        Serial.print("CS pin: "); Serial.println(settings.csPin);
        Serial.print("Hardware type: "); Serial.println(settings.hardwareType);
        Serial.print("Device count: "); Serial.println(settings.deviceCount);
        Serial.print("Brightness: "); Serial.println(settings.brightness);
        Serial.println("-----------------------------");
    }
}

void saveSettings()
{
    settings.magic = SETTINGS_MAGIC;

    EEPROM.put(0, settings);
    EEPROM.commit();

    Serial.println("Settings saved.");
}
