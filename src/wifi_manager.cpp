#include "wifi_manager.h"

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiManager.h>

#include "config.h"
#include "settings.h"

bool wifiConnect()
{
    Serial.println();
    Serial.println("Starting WiFi...");

    WiFi.mode(WIFI_STA);

    WiFiManager wm;

    const char* apName = AP_NAME;
    const char* apPassword = AP_PASSWORD;

    bool result = wm.autoConnect(apName, apPassword);

    if (!result)
    {
        Serial.println("WiFi connection failed.");
        return false;
    }

    Serial.println();
    Serial.println("WiFi connected.");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());

    return true;
}

String getIPAddress()
{
    return WiFi.localIP().toString();
}

int getRSSI()
{
    return WiFi.RSSI();
}


// ============================================================
// WIFI SCAN
// ============================================================

String wifiScan()
{
    Serial.println();
    Serial.println("Scanning WiFi networks...");

    int count = WiFi.scanNetworks();

    Serial.print("Networks found: ");
    Serial.println(count);

    String json = "[";

    for (int i = 0; i < count; i++)
    {
        if (i > 0)
            json += ",";

        String ssid = WiFi.SSID(i);

        // Escape quotes
        ssid.replace("\\", "\\\\");
        ssid.replace("\"", "\\\"");

        json += "{";
        json += "\"ssid\":\"";
        json += ssid;
        json += "\",";
        json += "\"rssi\":";
        json += String(WiFi.RSSI(i));
        json += ",";
        json += "\"encryption\":";
        json += String(WiFi.encryptionType(i));
        json += "}";
    }

    json += "]";

    WiFi.scanDelete();

    return json;
}


// ============================================================
// CONNECT TO WIFI
// ============================================================

bool wifiConnectTo(
    const String& ssid,
    const String& password
)
{
    Serial.println();
    Serial.println("Connecting to new WiFi...");

    Serial.print("SSID: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);

    WiFi.disconnect();
    delay(200);

    WiFi.begin(
        ssid.c_str(),
        password.c_str()
    );

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        yield();

        Serial.print(".");

        if (millis() - start > 20000)
        {
            Serial.println();
            Serial.println("WiFi connection timeout.");

            return false;
        }
    }

    Serial.println();
    Serial.println("WiFi connected!");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());

    return true;
}