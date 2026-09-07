#include "mqtt_manager.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"
#include "settings.h"
#include "clock_manager.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

static unsigned long lastReconnectAttempt = 0;
static unsigned long lastPublishTime = 0;

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    String msg = "";
    for (unsigned int i = 0; i < length; i++)
    {
        msg += (char)payload[i];
    }

    Serial.print("MQTT message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(msg);

    // Integrate with display
    if (String(topic) == settings.mqttSubTopic)
    {
        clockShowMessage(msg);
    }
}

bool mqttReconnect()
{
    if (!settings.useMQTT) return false;
    
    if (String(settings.mqttBroker) == "") return false;

    Serial.print("Attempting MQTT connection to ");
    Serial.print(settings.mqttBroker);
    Serial.print("...");

    bool connected = false;

    if (String(settings.mqttUser) != "")
    {
        connected = mqttClient.connect(
            settings.mqttClientId,
            settings.mqttUser,
            settings.mqttPassword
        );
    }
    else
    {
        connected = mqttClient.connect(settings.mqttClientId);
    }

    if (connected)
    {
        Serial.println(" connected!");

        if (String(settings.mqttSubTopic) != "")
        {
            mqttClient.subscribe(settings.mqttSubTopic);
            Serial.print("Subscribed to ");
            Serial.println(settings.mqttSubTopic);
        }
    }
    else
    {
        Serial.print(" failed, rc=");
        Serial.println(mqttClient.state());
    }

    return connected;
}

void mqttInit()
{
    if (!settings.useMQTT)
    {
        Serial.println("MQTT is disabled in settings.");
        return;
    }

    if (String(settings.mqttBroker) == "")
    {
        Serial.println("MQTT Broker not configured.");
        return;
    }

    Serial.println();
    Serial.println("Initializing MQTT...");

    mqttClient.setServer(settings.mqttBroker, settings.mqttPort);
    mqttClient.setCallback(mqttCallback);
    
    lastReconnectAttempt = 0;
    lastPublishTime = millis();
}

void mqttPublishStatus()
{
    if (!mqttClient.connected() || !settings.useMQTT || String(settings.mqttPubTopic) == "")
    {
        return;
    }

    String payload = "{";
    payload += "\"status\":\"online\",";
    payload += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    payload += "\"rssi\":" + String(WiFi.RSSI());
    payload += "}";

    mqttClient.publish(settings.mqttPubTopic, payload.c_str());
    Serial.print("Published MQTT status to ");
    Serial.println(settings.mqttPubTopic);
}

void mqttLoop()
{
    if (!settings.useMQTT) return;
    
    if (WiFi.status() != WL_CONNECTED) return;

    if (!mqttClient.connected())
    {
        long now = millis();
        // Non-blocking reconnect attempt every 5 seconds
        if (now - lastReconnectAttempt > 5000 || lastReconnectAttempt == 0)
        {
            lastReconnectAttempt = now;
            if (mqttReconnect())
            {
                lastReconnectAttempt = 0;
            }
        }
    }
    else
    {
        mqttClient.loop();

        // Publish status periodically
        long now = millis();
        if (settings.mqttPubInterval > 0)
        {
            if (now - lastPublishTime > settings.mqttPubInterval * 1000)
            {
                lastPublishTime = now;
                mqttPublishStatus();
            }
        }
    }
}

bool mqttIsConnected()
{
    return mqttClient.connected();
}
