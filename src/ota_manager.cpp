#include "ota_manager.h"

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "settings.h"
#include "display.h"

static bool otaStarted = false;

String otaHostname()
{
    return "sentramatrix-" + String(ESP.getChipId(), HEX);
}

void otaInit()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("OTA not started: WiFi not connected.");
        return;
    }

    String hostname = otaHostname();

    ArduinoOTA.setHostname(hostname.c_str());
    ArduinoOTA.setPort(OTA_PORT);

    if (strlen(settings.otaPassword) > 0)
    {
        ArduinoOTA.setPassword(settings.otaPassword);
    }

    ArduinoOTA.onStart([]()
    {
        Serial.println("OTA update started.");
        displaySetPower(true);
        displayStatic("OTA");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
        static int lastShown = -1;
        int pct = total ? (int)((progress * 100UL) / total) : 0;

        // Refresh the display every 10% to keep the upload fast
        if (pct / 10 != lastShown)
        {
            lastShown = pct / 10;
            String text = String(pct) + "%";
            displayStatic(text.c_str());
            Serial.printf("OTA progress: %d%%\n", pct);
        }
    });

    ArduinoOTA.onEnd([]()
    {
        Serial.println("OTA update finished. Rebooting...");
        displayStatic("Reboot");
    });

    ArduinoOTA.onError([](ota_error_t error)
    {
        Serial.printf("OTA error [%u]\n", error);
        displayStatic("OTA Err");
    });

    ArduinoOTA.begin();
    otaStarted = true;

    Serial.print("ArduinoOTA ready: ");
    Serial.print(hostname);
    Serial.print(".local:");
    Serial.print(OTA_PORT);
    Serial.println(strlen(settings.otaPassword) > 0 ? " (password protected)" : " (no password)");
}

void otaLoop()
{
    if (otaStarted)
    {
        ArduinoOTA.handle();
    }
}
