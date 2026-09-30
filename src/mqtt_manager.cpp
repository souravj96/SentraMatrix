#include "mqtt_manager.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"
#include "settings.h"
#include "clock_manager.h"
#include "display.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

static unsigned long lastReconnectAttempt = 0;
static unsigned long lastPublishTime = 0;

static String getDeviceId()
{
    return "sentramatrix_" + String(ESP.getChipId(), HEX);
}

static String getAvailabilityTopic()
{
    return String(settings.mqttPubTopic) + "/availability";
}

static String getBrightnessCommandTopic()
{
    return String(settings.mqttPubTopic) + "/brightness/set";
}

static String getRestartCommandTopic()
{
    return String(settings.mqttPubTopic) + "/restart";
}

static String getQuietHoursCommandTopic()
{
    return String(settings.mqttPubTopic) + "/quiet_hours/set";
}

static String getQuietStartCommandTopic()
{
    return String(settings.mqttPubTopic) + "/quiet_start/set";
}

static String getQuietEndCommandTopic()
{
    return String(settings.mqttPubTopic) + "/quiet_end/set";
}

static String getQuietBrightnessCommandTopic()
{
    return String(settings.mqttPubTopic) + "/quiet_brightness/set";
}

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

    String topicStr = String(topic);

    // Integrate with display
    if (topicStr == settings.mqttSubTopic)
    {
        clockShowMessage(msg);
    }
    // Brightness command
    else if (topicStr == getBrightnessCommandTopic())
    {
        int newBrightness = msg.toInt();
        if (newBrightness < 0) newBrightness = 0;
        if (newBrightness > 15) newBrightness = 15;
        settings.brightness = newBrightness;
        displaySetBrightness(settings.brightness);
        saveSettings();
        mqttPublishStatus();
        Serial.printf("MQTT updated brightness to: %d\n", settings.brightness);
    }
    // Restart command
    else if (topicStr == getRestartCommandTopic())
    {
        Serial.println("MQTT restart command received. Restarting...");
        delay(300);
        ESP.restart();
    }
    // Quiet hours toggle
    else if (topicStr == getQuietHoursCommandTopic())
    {
        settings.quietHoursEnabled = (msg.equalsIgnoreCase("ON") || msg == "1");
        saveSettings();
        mqttPublishStatus();
    }
    // Quiet start hour
    else if (topicStr == getQuietStartCommandTopic())
    {
        int val = msg.toInt();
        if (val >= 0 && val <= 23) {
            settings.quietStartHour = val;
            saveSettings();
            mqttPublishStatus();
        }
    }
    // Quiet end hour
    else if (topicStr == getQuietEndCommandTopic())
    {
        int val = msg.toInt();
        if (val >= 0 && val <= 23) {
            settings.quietEndHour = val;
            saveSettings();
            mqttPublishStatus();
        }
    }
    // Quiet brightness
    else if (topicStr == getQuietBrightnessCommandTopic())
    {
        int val = msg.toInt();
        if (val >= 0 && val <= 15) {
            settings.quietBrightness = val;
            saveSettings();
            mqttPublishStatus();
        }
    }
}

void mqttPublishDiscovery()
{
    if (!mqttClient.connected() || !settings.useHADiscovery) return;

    Serial.println("Publishing Home Assistant MQTT discovery...");

    String devId = getDeviceId();
    String devName = (String(settings.mqttClientId).length() > 0) ? String(settings.mqttClientId) : "SentraMatrix";
    String cu = "http://" + WiFi.localIP().toString() + "/";
    String devJson = "\"dev\":{\"ids\":[\"" + devId + "\"],\"name\":\"" + devName + "\",\"mf\":\"SentraMatrix\",\"mdl\":\"ESP8266 MAX7219 Clock\",\"sw\":\"1.0.0\",\"cu\":\"" + cu + "\"}";
    String availTopic = getAvailabilityTopic();
    String availJson = "\"avty_t\":\"" + availTopic + "\",\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\"";

    // 1. Text Message entity
    if (String(settings.mqttSubTopic).length() > 0)
    {
        String topic = "homeassistant/text/" + devId + "/message/config";
        String payload = "{"
            "\"name\":\"Display Message\","
            "\"uniq_id\":\"" + devId + "_message\","
            "\"cmd_t\":\"" + String(settings.mqttSubTopic) + "\","
            "\"mode\":\"text\","
            "\"max\":120,"
            "\"icon\":\"mdi:led-strip-variant\","
            + availJson + ","
            + devJson +
        "}";
        mqttClient.publish(topic.c_str(), payload.c_str(), true);
    }

    // 2. Brightness Number entity
    if (String(settings.mqttPubTopic).length() > 0)
    {
        String topic = "homeassistant/number/" + devId + "/brightness/config";
        String payload = "{"
            "\"name\":\"Brightness\","
            "\"uniq_id\":\"" + devId + "_brightness\","
            "\"cmd_t\":\"" + getBrightnessCommandTopic() + "\","
            "\"stat_t\":\"" + String(settings.mqttPubTopic) + "\","
            "\"val_tpl\":\"{{ value_json.brightness }}\","
            "\"min\":0,\"max\":15,\"step\":1,"
            "\"icon\":\"mdi:brightness-6\","
            + availJson + ","
            + devJson +
        "}";
        mqttClient.publish(topic.c_str(), payload.c_str(), true);
    }

    // 3. Restart Button entity
    if (String(settings.mqttPubTopic).length() > 0)
    {
        String topic = "homeassistant/button/" + devId + "/restart/config";
        String payload = "{"
            "\"name\":\"Restart\","
            "\"uniq_id\":\"" + devId + "_restart\","
            "\"cmd_t\":\"" + getRestartCommandTopic() + "\","
            "\"dev_cla\":\"restart\","
            "\"ent_cat\":\"config\","
            + availJson + ","
            + devJson +
        "}";
        mqttClient.publish(topic.c_str(), payload.c_str(), true);
    }

    // 4. Sensors
    if (String(settings.mqttPubTopic).length() > 0)
    {
        // WiFi RSSI
        {
            String topic = "homeassistant/sensor/" + devId + "/rssi/config";
            String payload = "{"
                "\"name\":\"WiFi Signal\","
                "\"uniq_id\":\"" + devId + "_rssi\","
                "\"stat_t\":\"" + String(settings.mqttPubTopic) + "\","
                "\"val_tpl\":\"{{ value_json.rssi }}\","
                "\"unit_of_meas\":\"dBm\","
                "\"dev_cla\":\"signal_strength\","
                "\"stat_cla\":\"measurement\","
                "\"ent_cat\":\"diagnostic\","
                "\"icon\":\"mdi:wifi\","
                + availJson + ","
                + devJson +
            "}";
            mqttClient.publish(topic.c_str(), payload.c_str(), true);
        }

        // IP Address
        {
            String topic = "homeassistant/sensor/" + devId + "/ip/config";
            String payload = "{"
                "\"name\":\"IP Address\","
                "\"uniq_id\":\"" + devId + "_ip\","
                "\"stat_t\":\"" + String(settings.mqttPubTopic) + "\","
                "\"val_tpl\":\"{{ value_json.ip }}\","
                "\"ent_cat\":\"diagnostic\","
                "\"icon\":\"mdi:ip-network\","
                + availJson + ","
                + devJson +
            "}";
            mqttClient.publish(topic.c_str(), payload.c_str(), true);
        }

        // Free Heap
        {
            String topic = "homeassistant/sensor/" + devId + "/heap/config";
            String payload = "{"
                "\"name\":\"Free Memory\","
                "\"uniq_id\":\"" + devId + "_heap\","
                "\"stat_t\":\"" + String(settings.mqttPubTopic) + "\","
                "\"val_tpl\":\"{{ value_json.heap }}\","
                "\"unit_of_meas\":\"B\","
                "\"dev_cla\":\"data_size\","
                "\"stat_cla\":\"measurement\","
                "\"ent_cat\":\"diagnostic\","
                "\"icon\":\"mdi:memory\","
                + availJson + ","
                + devJson +
            "}";
            mqttClient.publish(topic.c_str(), payload.c_str(), true);
        }

        // Uptime
        {
            String topic = "homeassistant/sensor/" + devId + "/uptime/config";
            String payload = "{"
                "\"name\":\"Uptime\","
                "\"uniq_id\":\"" + devId + "_uptime\","
                "\"stat_t\":\"" + String(settings.mqttPubTopic) + "\","
                "\"val_tpl\":\"{{ value_json.uptime }}\","
                "\"unit_of_meas\":\"s\","
                "\"dev_cla\":\"duration\","
                "\"stat_cla\":\"total_increasing\","
                "\"ent_cat\":\"diagnostic\","
                "\"icon\":\"mdi:timer-outline\","
                + availJson + ","
                + devJson +
            "}";
            mqttClient.publish(topic.c_str(), payload.c_str(), true);
        }
    }

    // 5. Quiet Hours Entities
    if (String(settings.mqttPubTopic).length() > 0)
    {
        // Quiet Hours Switch
        {
            String topic = "homeassistant/switch/" + devId + "/quiet_hours/config";
            String payload = "{"
                "\"name\":\"Quiet Hours\","
                "\"uniq_id\":\"" + devId + "_quiet_hours\","
                "\"cmd_t\":\"" + getQuietHoursCommandTopic() + "\","
                "\"stat_t\":\"" + String(settings.mqttPubTopic) + "\","
                "\"val_tpl\":\"{{ value_json.quiet_hours }}\","
                "\"pl_on\":\"ON\","
                "\"pl_off\":\"OFF\","
                "\"icon\":\"mdi:sleep\","
                + availJson + ","
                + devJson +
            "}";
            mqttClient.publish(topic.c_str(), payload.c_str(), true);
        }

        // Quiet Start Hour
        {
            String topic = "homeassistant/number/" + devId + "/quiet_start/config";
            String payload = "{"
                "\"name\":\"Quiet Start Hour\","
                "\"uniq_id\":\"" + devId + "_quiet_start\","
                "\"cmd_t\":\"" + getQuietStartCommandTopic() + "\","
                "\"stat_t\":\"" + String(settings.mqttPubTopic) + "\","
                "\"val_tpl\":\"{{ value_json.quiet_start }}\","
                "\"min\":0,\"max\":23,\"step\":1,"
                "\"icon\":\"mdi:clock-start\","
                "\"ent_cat\":\"config\","
                + availJson + ","
                + devJson +
            "}";
            mqttClient.publish(topic.c_str(), payload.c_str(), true);
        }

        // Quiet End Hour
        {
            String topic = "homeassistant/number/" + devId + "/quiet_end/config";
            String payload = "{"
                "\"name\":\"Quiet End Hour\","
                "\"uniq_id\":\"" + devId + "_quiet_end\","
                "\"cmd_t\":\"" + getQuietEndCommandTopic() + "\","
                "\"stat_t\":\"" + String(settings.mqttPubTopic) + "\","
                "\"val_tpl\":\"{{ value_json.quiet_end }}\","
                "\"min\":0,\"max\":23,\"step\":1,"
                "\"icon\":\"mdi:clock-end\","
                "\"ent_cat\":\"config\","
                + availJson + ","
                + devJson +
            "}";
            mqttClient.publish(topic.c_str(), payload.c_str(), true);
        }

        // Quiet Brightness
        {
            String topic = "homeassistant/number/" + devId + "/quiet_brightness/config";
            String payload = "{"
                "\"name\":\"Quiet Brightness\","
                "\"uniq_id\":\"" + devId + "_quiet_brightness\","
                "\"cmd_t\":\"" + getQuietBrightnessCommandTopic() + "\","
                "\"stat_t\":\"" + String(settings.mqttPubTopic) + "\","
                "\"val_tpl\":\"{{ value_json.quiet_brightness }}\","
                "\"min\":0,\"max\":15,\"step\":1,"
                "\"icon\":\"mdi:brightness-4\","
                "\"ent_cat\":\"config\","
                + availJson + ","
                + devJson +
            "}";
            mqttClient.publish(topic.c_str(), payload.c_str(), true);
        }
    }

    Serial.println("Home Assistant MQTT discovery published.");
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

        // Subscribe to brightness command topic
        String brightnessCmd = getBrightnessCommandTopic();
        mqttClient.subscribe(brightnessCmd.c_str());
        Serial.print("Subscribed to ");
        Serial.println(brightnessCmd);

        // Subscribe to restart command topic
        String restartCmd = getRestartCommandTopic();
        mqttClient.subscribe(restartCmd.c_str());
        Serial.print("Subscribed to ");
        Serial.println(restartCmd);

        // Subscribe to quiet hours command topics
        mqttClient.subscribe(getQuietHoursCommandTopic().c_str());
        mqttClient.subscribe(getQuietStartCommandTopic().c_str());
        mqttClient.subscribe(getQuietEndCommandTopic().c_str());
        mqttClient.subscribe(getQuietBrightnessCommandTopic().c_str());

        // Home Assistant Discovery
        if (settings.useHADiscovery)
        {
            mqttPublishDiscovery();
        }

        // Publish initial status
        mqttPublishStatus();
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
    payload += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    payload += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
    payload += "\"uptime\":" + String(millis() / 1000) + ",";
    payload += "\"brightness\":" + String(settings.brightness) + ",";
    payload += "\"quiet_hours\":\"" + String(settings.quietHoursEnabled ? "ON" : "OFF") + "\",";
    payload += "\"quiet_start\":" + String(settings.quietStartHour) + ",";
    payload += "\"quiet_end\":" + String(settings.quietEndHour) + ",";
    payload += "\"quiet_brightness\":" + String(settings.quietBrightness);
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
