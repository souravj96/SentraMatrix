#include "settings.h"
#include <EEPROM.h>
#include <string.h>

Settings settings;

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
