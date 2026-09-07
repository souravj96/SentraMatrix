#include <Arduino.h>

#include <ESP8266WiFi.h>
#include "config.h"
#include "settings.h"
#include "display.h"
#include "wifi_manager.h"
#include "clock_manager.h"
#include "web_server.h"
#include "mqtt_manager.h"

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("       SentraMatrix");
    Serial.println("==============================");

    // --------------------------------------------------------
    // LOAD SETTINGS
    // --------------------------------------------------------

    loadSettings();

    // --------------------------------------------------------
    // INITIALIZE DISPLAY
    // --------------------------------------------------------

    displayInit();

    displayScroll("SentraMatrix");

    // --------------------------------------------------------
    // CONNECT WIFI
    // --------------------------------------------------------

    displayStatic("WiFi...");
    if (!wifiConnect())
    {
        Serial.println("WiFi failed.");
        displayScroll("WiFi Error");
        delay(2000);
        return;
    }

    displayScroll(WiFi.localIP().toString().c_str());

    // --------------------------------------------------------
    // START CLOCK
    // --------------------------------------------------------

    if (settings.useNTP) {
        displayStatic("NTP...");
    }
    clockInit();

    // --------------------------------------------------------
    // START MQTT
    // --------------------------------------------------------

    if (settings.useMQTT) {
        displayStatic("MQTT...");
    }
    mqttInit();

    // --------------------------------------------------------
    // START WEB SERVER
    // --------------------------------------------------------

    webServerInit();

    Serial.println();
    Serial.println("SentraMatrix ready.");
}

void loop()
{
    // --------------------------------------------------------
    // DISPLAY ANIMATION ENGINE
    // --------------------------------------------------------

    displayLoop();

    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    clockUpdate();

    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    webServerLoop();

    // --------------------------------------------------------
    // MQTT
    // --------------------------------------------------------

    mqttLoop();

    yield();
}
