#include "web_server.h"

#ifdef ESP32
  #include <WebServer.h>
  #include <WiFi.h>
#else
  #include <ESP8266WebServer.h>
  #include <ESP8266WiFi.h>
  using WebServer = ESP8266WebServer;
#endif
#include <MD_MAX72xx.h>
#include <WiFiManager.h>

#include "config.h"
#include "settings.h"
#include "mqtt_manager.h"
#include "display.h"
#include "wifi_manager.h"

WebServer server(80);

// ============================================================
// HARDWARE NAME
// ============================================================

String hardwareName(uint8_t type)
{
    switch (type)
    {
        case MD_MAX72XX::PAROLA_HW:
            return "PAROLA_HW";

        case MD_MAX72XX::GENERIC_HW:
            return "GENERIC_HW";

        case MD_MAX72XX::ICSTATION_HW:
            return "ICSTATION_HW";

        case MD_MAX72XX::FC16_HW:
            return "FC16_HW";

        default:
            return "UNKNOWN";
    }
}

// ============================================================
// NAVIGATION BAR
// ============================================================

String makeNav(const String& active)
{
    String html;

    html += "<div class='nav'>";

    html += "<a href='/'";
    if (active == "home")
        html += " class='active'";
    html += ">Home</a>";

    html += "<a href='/hardware'";
    if (active == "hardware")
        html += " class='active'";
    html += ">Hardware</a>";

    html += "<a href='/wifi'";
    if (active == "wifi")
        html += " class='active'";
    html += ">WiFi</a>";

    html += "<a href='/time'";
    if (active == "time")
        html += " class='active'";
    html += ">Time</a>";

    html += "<a href='/mqtt'";
    if (active == "mqtt")
        html += " class='active'";
    html += ">MQTT</a>";

    html += "</div>";

    return html;
}

// ============================================================
// COMMON PAGE HEADER
// ============================================================

String makeHeader(const String& title, const String& active)
{
    String html;

    html += F(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>SentraMatrix</title>"

        "<style>"

        "*{box-sizing:border-box;}"

        "body{"
        "font-family:Arial,sans-serif;"
        "background:#111;"
        "color:#eee;"
        "margin:0;"
        "padding:20px;"
        "}"

        ".box{"
        "max-width:700px;"
        "margin:auto;"
        "background:#222;"
        "padding:20px;"
        "border-radius:12px;"
        "}"

        "h1{"
        "margin-top:0;"
        "margin-bottom:5px;"
        "}"

        "h2{"
        "margin-top:25px;"
        "}"

        ".subtitle{"
        "color:#999;"
        "margin-bottom:20px;"
        "}"

        ".nav{"
        "display:flex;"
        "gap:6px;"
        "background:#181818;"
        "padding:6px;"
        "border-radius:8px;"
        "margin:20px 0;"
        "overflow-x:auto;"
        "}"

        ".nav a{"
        "flex:1;"
        "text-align:center;"
        "text-decoration:none;"
        "color:#aaa;"
        "padding:12px 10px;"
        "border-radius:6px;"
        "white-space:nowrap;"
        "}"

        ".nav a:hover{"
        "background:#333;"
        "color:white;"
        "}"

        ".nav a.active{"
        "background:#008cff;"
        "color:white;"
        "}"

        ".info{"
        "background:#181818;"
        "padding:15px;"
        "border-radius:8px;"
        "margin-top:15px;"
        "line-height:1.8;"
        "}"

        ".section{"
        "background:#181818;"
        "padding:15px;"
        "border-radius:8px;"
        "margin-top:15px;"
        "}"

        "label{"
        "display:block;"
        "margin-top:18px;"
        "}"

        "select,input{"
        "width:100%;"
        "padding:12px;"
        "margin-top:6px;"
        "background:#333;"
        "color:white;"
        "border:1px solid #555;"
        "border-radius:6px;"
        "font-size:15px;"
        "}"

        "button{"
        "width:100%;"
        "padding:14px;"
        "margin-top:25px;"
        "background:#008cff;"
        "color:white;"
        "border:0;"
        "border-radius:6px;"
        "font-size:16px;"
        "}"

        ".status-good{"
        "color:#4ade80;"
        "}"

        ".status-bad{"
        "color:#f87171;"
        "}"

        "</style>"

        "</head>"
        "<body>"
        "<div class='box'>"
    );

    html += "<h1>SentraMatrix</h1>";
    html += "<div class='subtitle'>";
    html += title;
    html += "</div>";

    html += makeNav(active);

    return html;
}

// ============================================================
// PAGE FOOTER
// ============================================================

String makeFooter()
{
    String html;

    html += F(
        "</div>"
        "</body>"
        "</html>"
    );

    return html;
}

// ============================================================
// HOME PAGE
// ============================================================

String makeHomePage()
{
    String html;

    html += makeHeader(
        "Device Status",
        "home"
    );

    // --------------------------------------------------------
    // DEVICE STATUS
    // --------------------------------------------------------

    html += "<div class='section'>";

    html += "<h2>Device</h2>";

    html += "Status: <span class='status-good'>Online</span><br>";

    html += "Firmware: SentraMatrix<br>";

    html += "Free Heap: ";
    html += String(ESP.getFreeHeap());
    html += " bytes";

    html += "</div>";

    // --------------------------------------------------------
    // WIFI STATUS
    // --------------------------------------------------------

    html += "<div class='section'>";

    html += "<h2>WiFi</h2>";

    if (WiFi.status() == WL_CONNECTED)
    {
        html += "Status: <span class='status-good'>Connected</span><br>";

        html += "IP Address: ";
        html += WiFi.localIP().toString();
        html += "<br>";

        html += "RSSI: ";
        html += String(WiFi.RSSI());
        html += " dBm<br>";

        html += "SSID: ";
        html += WiFi.SSID();
    }
    else
    {
        html += "Status: <span class='status-bad'>Disconnected</span>";
    }

    html += "</div>";

    // --------------------------------------------------------
    // DISPLAY STATUS
    // --------------------------------------------------------

    html += "<div class='section'>";

    html += "<h2>Display</h2>";

    html += "Hardware: ";
    html += hardwareName(settings.hardwareType);
    html += "<br>";

    html += "Modules: ";
    html += String(settings.deviceCount);
    html += "<br>";

    html += "Brightness: ";
    html += String(settings.brightness);
    html += " / 15<br>";

    html += "Quiet Hours: ";
    html += settings.quietHoursEnabled ? "Enabled (" + String(settings.quietStartHour) + ":00 - " + String(settings.quietEndHour) + ":00)" : "Disabled";

    html += "</div>";


    // --------------------------------------------------------
    // MQTT STATUS
    // --------------------------------------------------------

    html += "<div class='section'>";
    html += "<h2>MQTT</h2>";
    if (!settings.useMQTT)
    {
        html += "Status: <span class='status-bad'>Disabled</span>";
    }
    else if (mqttIsConnected())
    {
        html += "Status: <span class='status-good'>Connected</span><br>";
        html += "Broker: " + String(settings.mqttBroker) + "<br>";
        html += "Topic: " + String(settings.mqttSubTopic);
    }
    else
    {
        html += "Status: <span class='status-bad'>Disconnected</span><br>";
        html += "Broker: " + String(settings.mqttBroker);
    }
    if (settings.useMQTT)
    {
        html += "<br>HA Discovery: " + String(settings.useHADiscovery ? "Enabled" : "Disabled");
    }
    html += "</div>";

    // --------------------------------------------------------
    // CURRENT SETTINGS / WIRING

    // --------------------------------------------------------

    html += "<div class='section'>";
    html += "<h2>Hardware Pins</h2>";
#ifdef ESP32
    html += "DIN: GPIO23<br>";
    html += "CLK: GPIO18<br>";
    html += "CS: GPIO5";
#else
    html += "DIN: D7 / GPIO13<br>";
    html += "CLK: D5 / GPIO14<br>";
    html += "CS: D4 / GPIO2";
#endif
    html += "</div>";

    html += makeFooter();

    return html;
}

// ============================================================
// HARDWARE PAGE
// ============================================================

String makeHardwarePage()
{
    String html;

    html += makeHeader(
        "Display Hardware Configuration",
        "hardware"
    );

    html += "<form method='POST' action='/save'>";

    // --------------------------------------------------------
    // DISPLAY PINS
    // --------------------------------------------------------

    html += "<div class='section'>";

    html += "<h2>Display Pins</h2>";

    html += "<label>DIN / Data</label>";

    html += "<select name='dataPin'>";

#ifdef ESP32
    const char* pinNames[] =
    {
        "GPIO2",
        "GPIO4",
        "GPIO5 (VSPI CS)",
        "GPIO12",
        "GPIO13",
        "GPIO14",
        "GPIO15",
        "GPIO16",
        "GPIO17",
        "GPIO18 (VSPI CLK)",
        "GPIO19",
        "GPIO21",
        "GPIO22",
        "GPIO23 (VSPI DIN)",
        "GPIO25",
        "GPIO26",
        "GPIO27",
        "GPIO32",
        "GPIO33"
    };

    const uint8_t pinValues[] =
    {
        2, 4, 5, 12, 13, 14, 15,
        16, 17, 18, 19, 21, 22,
        23, 25, 26, 27, 32, 33
    };

    const int numPins = sizeof(pinValues) / sizeof(pinValues[0]);
#else
    const char* pinNames[] =
    {
        "D0",
        "D1",
        "D2",
        "D3",
        "D4",
        "D5",
        "D6",
        "D7",
        "D8"
    };

    const uint8_t pinValues[] =
    {
        D0,
        D1,
        D2,
        D3,
        D4,
        D5,
        D6,
        D7,
        D8
    };

    const int numPins = 9;
#endif

    for (int i = 0; i < numPins; i++)
    {
        html += "<option value='";
        html += String(pinValues[i]);
        html += "'";

        if (settings.dataPin == pinValues[i])
            html += " selected";

        html += ">";

        html += pinNames[i];

        html += "</option>";
    }

    html += "</select>";


    html += "<label>CLK / Clock</label>";

    html += "<select name='clkPin'>";

    for (int i = 0; i < numPins; i++)
    {
        html += "<option value='";
        html += String(pinValues[i]);
        html += "'";

        if (settings.clkPin == pinValues[i])
            html += " selected";

        html += ">";

        html += pinNames[i];

        html += "</option>";
    }

    html += "</select>";


    html += "<label>CS / Chip Select</label>";

    html += "<select name='csPin'>";

    for (int i = 0; i < numPins; i++)
    {
        html += "<option value='";
        html += String(pinValues[i]);
        html += "'";

        if (settings.csPin == pinValues[i])
            html += " selected";

        html += ">";

        html += pinNames[i];

        html += "</option>";
    }

    html += "</select>";

    html += "</div>";

    // --------------------------------------------------------
    // HARDWARE TYPE
    // --------------------------------------------------------

    html += "<div class='section'>";

    html += "<h2>Display Module</h2>";

    html += "<label>MAX7219 Hardware Type</label>";

    html += "<select name='hardware'>";

    const char* names[] =
    {
        "PAROLA_HW",
        "GENERIC_HW",
        "ICSTATION_HW",
        "FC16_HW"
    };

    const uint8_t values[] =
    {
        MD_MAX72XX::PAROLA_HW,
        MD_MAX72XX::GENERIC_HW,
        MD_MAX72XX::ICSTATION_HW,
        MD_MAX72XX::FC16_HW
    };

    for (int i = 0; i < 4; i++)
    {
        html += "<option value='";
        html += String(values[i]);
        html += "'";

        if (settings.hardwareType == values[i])
            html += " selected";

        html += ">";

        html += names[i];

        html += "</option>";
    }

    html += "</select>";

    // --------------------------------------------------------
    // DEVICE COUNT
    // --------------------------------------------------------

    html += "<label>MAX7219 Modules</label>";

    html += "<select name='devices'>";

    for (int i = 1; i <= 16; i++)
    {
        html += "<option value='";
        html += String(i);

        if (settings.deviceCount == i)
            html += "' selected>";
        else
            html += "'>";

        html += String(i);
        html += "</option>";
    }

    html += "</select>";

    // --------------------------------------------------------
    // BRIGHTNESS
    // --------------------------------------------------------

    html += "<label>Brightness (0-15)</label>";

    html += "<input type='number' "
            "name='brightness' "
            "min='0' "
            "max='15' "
            "value='";

    html += String(settings.brightness);

    html += "'>";

    // --------------------------------------------------------
    // QUIET HOURS
    // --------------------------------------------------------

    html += "<br><br><h3>Quiet Hours</h3>";

    html += "<label>Quiet Hours</label>";
    html += "<select name='quietEnabled'>";
    html += "<option value='1'" + String(settings.quietHoursEnabled ? " selected" : "") + ">Enabled</option>";
    html += "<option value='0'" + String(!settings.quietHoursEnabled ? " selected" : "") + ">Disabled</option>";
    html += "</select>";

    html += "<label>Start Hour (0-23)</label>";
    html += "<input type='number' name='quietStart' min='0' max='23' value='" + String(settings.quietStartHour) + "'>";

    html += "<label>End Hour (0-23)</label>";
    html += "<input type='number' name='quietEnd' min='0' max='23' value='" + String(settings.quietEndHour) + "'>";

    html += "<label>Quiet Brightness (0 = Display Off, 1-15 = Dim)</label>";
    html += "<input type='number' name='quietBrightness' min='0' max='15' value='" + String(settings.quietBrightness) + "'>";

    html += "<button type='submit'>Save & Restart</button>";

    html += "</div>";

    html += "</form>";

    // --------------------------------------------------------
    // WIRING
    // --------------------------------------------------------

    html += "<div class='section'>";
    html += "<h2>Current Wiring</h2>";
#ifdef ESP32
    html += "DIN -> GPIO23<br>";
    html += "CLK -> GPIO18<br>";
    html += "CS -> GPIO5<br>";
#else
    html += "DIN -> D7 / GPIO13<br>";
    html += "CLK -> D5 / GPIO14<br>";
    html += "CS -> D4 / GPIO2<br>";
#endif
    html += "VCC -> 5V<br>";
    html += "GND -> GND";
    html += "</div>";

    html += makeFooter();

    return html;
}

// ============================================================
// WIFI PAGE - PLACEHOLDER
// ============================================================

String makeWiFiPage()
{
    String html;

    html += makeHeader(
        "WiFi Configuration",
        "wifi"
    );

    // ========================================================
    // CURRENT WIFI
    // ========================================================

    html += "<div class='section'>";

    html += "<h2>Current WiFi</h2>";

    if (WiFi.status() == WL_CONNECTED)
    {
        html += "Status: <span class='status-good'>Connected</span><br>";

        html += "SSID: ";
        html += WiFi.SSID();
        html += "<br>";

        html += "IP Address: ";
        html += WiFi.localIP().toString();
        html += "<br>";

        html += "RSSI: ";
        html += String(WiFi.RSSI());
        html += " dBm";
    }
    else
    {
        html += "Status: <span class='status-bad'>Disconnected</span>";
    }

    html += "</div>";


    // ========================================================
    // CONNECT TO WIFI
    // ========================================================

    html += "<div class='section'>";

    html += "<h2>WiFi Configuration</h2>";

    html += "<label>Available Networks</label>";

    html += "<select id='ssid'>";
    html += "<option value=''>Scanning...</option>";
    html += "</select>";

    html += "<label>WiFi Password</label>";

    html += "<input "
            "type='password' "
            "id='password' "
            "placeholder='Enter WiFi password'>";

    html += "<button onclick='connectWiFi()'>"
            "Connect to WiFi"
            "</button>";

    html += "<p id='status'></p>";

    html += "</div>";


    // ========================================================
    // JAVASCRIPT
    // ========================================================

    html += F(
        "<script>"

        "function loadNetworks(){"

        "fetch('/wifi-scan')"
        ".then(r=>r.json())"
        ".then(data=>{"

        "let s=document.getElementById('ssid');"

        "s.innerHTML='';"

        "if(data.length===0){"
        "s.innerHTML='<option value=\"\">No networks found</option>';"
        "return;"
        "}"

        "data.forEach(n=>{"

        "let o=document.createElement('option');"

        "o.value=n.ssid;"

        "o.text=n.ssid+' ('+n.rssi+' dBm)';"

        "s.appendChild(o);"

        "});"

        "})"

        ".catch(()=>{"
        "document.getElementById('ssid').innerHTML="
        "'<option value=\"\">Scan failed</option>';"
        "});"

        "}"

        "function connectWiFi(){"

        "let ssid=document.getElementById('ssid').value;"

        "let password=document.getElementById('password').value;"

        "if(!ssid){"
        "alert('Please select a WiFi network.');"
        "return;"
        "}"

        "document.getElementById('status').innerHTML="
        "'Connecting to '+ssid+'...';"

        "fetch('/wifi-connect',{"
        "method:'POST',"
        "headers:{'Content-Type':'application/x-www-form-urlencoded'},"
        "body:'ssid='+encodeURIComponent(ssid)"
        "+'&password='+encodeURIComponent(password)"
        "})"

        ".then(r=>r.text())"

        ".then(data=>{"
        "document.getElementById('status').innerHTML=data;"
        "})"

        ".catch(()=>{"
        "document.getElementById('status').innerHTML="
        "'Connection request failed.';"
        "});"

        "}"

        "loadNetworks();"

        "</script>"
    );


    // ========================================================
    // ACCESS POINT
    // ========================================================

    html += "<div class='section'>";

    html += "<h2>Access Point</h2>";

    html += "<p>";
    html += "SentraMatrix creates an AP when WiFi configuration "
            "is required.";
    html += "</p>";

    html += "<b>AP Name:</b> ";
    html += AP_NAME;

    html += "<br>";

    html += "<b>AP Password:</b> ";
    html += AP_PASSWORD;

    html += "</div>";

    html += makeFooter();

    return html;
}

// ============================================================
// TIME PAGE - PLACEHOLDER
// ============================================================

String makeTimePage()
{
    String html;

    html += makeHeader(
        "Time Configuration",
        "time"
    );

    html += "<div class='section'>";
    html += "<h2>Time Settings</h2>";
    html += "<form action='/save' method='POST'>";

    // NTP Enable
    html += "<label>Enable NTP</label>";
    html += "<select name='useNTP'>";
    html += "<option value='1' " + String(settings.useNTP ? "selected" : "") + ">Yes</option>";
    html += "<option value='0' " + String(!settings.useNTP ? "selected" : "") + ">No</option>";
    html += "</select>";

    // NTP Servers
    html += "<label>NTP Server 1</label>";
    html += "<input type='text' name='ntp1' value='" + String(settings.ntpServer1) + "'>";

    html += "<label>NTP Server 2</label>";
    html += "<input type='text' name='ntp2' value='" + String(settings.ntpServer2) + "'>";

    html += "<label>NTP Server 3</label>";
    html += "<input type='text' name='ntp3' value='" + String(settings.ntpServer3) + "'>";

    // Timezone Offset
    html += "<label>Timezone Offset (Seconds)</label>";
    html += "<input type='number' name='gmtOffset' value='" + String(settings.gmtOffsetSec) + "'>";
    html += "<small>Example: 19800 for IST (UTC+5:30), -18000 for EST (UTC-5)</small><br><br>";

    // Daylight Offset
    html += "<label>Daylight Saving Offset (Seconds)</label>";
    html += "<input type='number' name='dstOffset' value='" + String(settings.daylightOffsetSec) + "'>";
    html += "<small>Usually 3600 or 0.</small><br><br>";

    // 12/24 Hour format
    html += "<label>Time Format</label>";
    html += "<select name='use24Hour'>";
    html += "<option value='1' " + String(settings.use24Hour ? "selected" : "") + ">24-Hour (HH:MM)</option>";
    html += "<option value='0' " + String(!settings.use24Hour ? "selected" : "") + ">12-Hour (HH:MM)</option>";
    html += "</select>";

    // Date Format
    html += "<label>Date Format</label>";
    html += "<select name='dateFormat'>";
    html += "<option value='0' " + String(settings.dateFormat == 0 ? "selected" : "") + ">DD-MM-YYYY</option>";
    html += "<option value='1' " + String(settings.dateFormat == 1 ? "selected" : "") + ">MM-DD-YYYY</option>";
    html += "<option value='2' " + String(settings.dateFormat == 2 ? "selected" : "") + ">YYYY-MM-DD</option>";
    html += "</select>";

    html += "<br><button type='submit'>Save Time Settings</button>";
    html += "</form>";
    html += "</div>";

    html += makeFooter();

    return html;
}

// ============================================================
// MQTT PAGE - PLACEHOLDER
// ============================================================

String makeMQTTPage()
{
    String html;

    html += makeHeader(
        "MQTT Configuration",
        "mqtt"
    );

    html += "<div class='section'>";
    html += "<h2>MQTT Settings</h2>";
    html += "<form action='/save' method='POST'>";

    // MQTT Enable
    html += "<label>Enable MQTT</label>";
    html += "<select name='useMQTT'>";
    html += "<option value='1' " + String(settings.useMQTT ? "selected" : "") + ">Yes</option>";
    html += "<option value='0' " + String(!settings.useMQTT ? "selected" : "") + ">No</option>";
    html += "</select>";

    // Broker
    html += "<label>Broker IP / Hostname</label>";
    html += "<input type='text' name='mqttBroker' value='" + String(settings.mqttBroker) + "'>";

    // Port
    html += "<label>Port</label>";
    html += "<input type='number' name='mqttPort' value='" + String(settings.mqttPort) + "'>";

    // Username
    html += "<label>Username (Optional)</label>";
    html += "<input type='text' name='mqttUser' value='" + String(settings.mqttUser) + "'>";

    // Password
    html += "<label>Password (Optional)</label>";
    html += "<input type='password' name='mqttPassword' value='" + String(settings.mqttPassword) + "'>";

    // Client ID
    html += "<label>Client ID</label>";
    html += "<input type='text' name='mqttClientId' value='" + String(settings.mqttClientId) + "'>";

    // Sub Topic
    html += "<label>Incoming Message Topic (Subscribe)</label>";
    html += "<input type='text' name='mqttSubTopic' value='" + String(settings.mqttSubTopic) + "'>";
    
    // Pub Topic
    html += "<label>Status Topic (Publish)</label>";
    html += "<input type='text' name='mqttPubTopic' value='" + String(settings.mqttPubTopic) + "'>";
    
    // Pub Interval
    html += "<label>Publish Interval (Seconds)</label>";
    html += "<input type='number' name='mqttPubInterval' value='" + String(settings.mqttPubInterval) + "'>";

    // Home Assistant Discovery
    html += "<label>Home Assistant MQTT Discovery</label>";
    html += "<select name='useHADiscovery'>";
    html += "<option value='1' " + String(settings.useHADiscovery ? "selected" : "") + ">Enabled</option>";
    html += "<option value='0' " + String(!settings.useHADiscovery ? "selected" : "") + ">Disabled</option>";
    html += "</select>";

    html += "<br><button type='submit'>Save MQTT Settings</button>";
    html += "</form>";
    html += "</div>";

    html += makeFooter();

    return html;
}

// ============================================================
// ROOT / HOME
// ============================================================

void handleRoot()
{
    server.send(
        200,
        "text/html",
        makeHomePage()
    );
}

// ============================================================
// HARDWARE
// ============================================================

void handleHardware()
{
    server.send(
        200,
        "text/html",
        makeHardwarePage()
    );
}

// ============================================================
// WIFI
// ============================================================

void handleWiFi()
{
    server.send(
        200,
        "text/html",
        makeWiFiPage()
    );
}

void handleWiFiConfig()
{
    WiFiManager wm;

    wm.setConfigPortalTimeout(180);

    Serial.println();
    Serial.println("Starting WiFi configuration portal...");

    bool result = wm.startConfigPortal(
        AP_NAME,
        AP_PASSWORD
    );

    if (result)
    {
        Serial.println("WiFi configuration successful.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        server.send(
            200,
            "text/html",
            "<html>"
            "<head>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "</head>"
            "<body>"
            "<h2>WiFi Connected</h2>"
            "<p>WiFi configuration successful.</p>"
            "<p>Restarting SentraMatrix...</p>"
            "</body>"
            "</html>"
        );

        delay(1000);
        ESP.restart();
    }
    else
    {
        server.send(
            200,
            "text/html",
            "<html>"
            "<head>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "</head>"
            "<body>"
            "<h2>WiFi Configuration</h2>"
            "<p>Configuration portal timed out or failed.</p>"
            "<p><a href='/wifi'>Return to WiFi settings</a></p>"
            "</body>"
            "</html>"
        );
    }
}

void handleWiFiReset()
{
    Serial.println();
    Serial.println("Resetting WiFi credentials...");

    WiFiManager wm;

    wm.resetSettings();

    server.send(
        200,
        "text/html",
        "<html>"
        "<head>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "</head>"
        "<body>"
        "<h2>WiFi credentials reset.</h2>"
        "<p>Restarting SentraMatrix...</p>"
        "</body>"
        "</html>"
    );

    delay(1000);

    ESP.restart();
}

void handleWiFiScan()
{
    String result = wifiScan();

    server.send(
        200,
        "application/json",
        result
    );
}

void handleWiFiConnect()
{
    if (!server.hasArg("ssid"))
    {
        server.send(
            400,
            "text/plain",
            "SSID is required."
        );

        return;
    }

    String ssid = server.arg("ssid");
    String password = server.arg("password");

    Serial.println();
    Serial.println("================================");
    Serial.println("WiFi configuration request");
    Serial.println("================================");

    bool connected = wifiConnectTo(
        ssid,
        password
    );

    if (connected)
    {
        server.send(
            200,
            "text/plain",
            "Connected successfully. IP: " +
            WiFi.localIP().toString() +
            ". Restarting..."
        );

        delay(1500);

        ESP.restart();
    }
    else
    {
        server.send(
            200,
            "text/plain",
            "Failed to connect. Please check the password."
        );
    }
}

// ============================================================
// TIME
// ============================================================

void handleTime()
{
    server.send(
        200,
        "text/html",
        makeTimePage()
    );
}

// ============================================================
// MQTT
// ============================================================

void handleMQTT()
{
    server.send(
        200,
        "text/html",
        makeMQTTPage()
    );
}

// ============================================================
// SAVE
// ============================================================

void handleSave()
{
    if (server.hasArg("hardware"))
    {
        settings.hardwareType =
            server.arg("hardware").toInt();
    }

    if (server.hasArg("dataPin"))
    {
        settings.dataPin =
            server.arg("dataPin").toInt();
    }

    if (server.hasArg("clkPin"))
    {
        settings.clkPin =
            server.arg("clkPin").toInt();
    }

    if (server.hasArg("csPin"))
    {
        settings.csPin =
            server.arg("csPin").toInt();
    }

    if (server.hasArg("devices"))
    {
        settings.deviceCount =
            server.arg("devices").toInt();

        if (settings.deviceCount < 1)
            settings.deviceCount = 1;

        if (settings.deviceCount > 16)
            settings.deviceCount = 16;
    }

    if (server.hasArg("brightness"))
    {
        settings.brightness =
            server.arg("brightness").toInt();

        if (settings.brightness > 15)
            settings.brightness = 15;
    }


    if (server.hasArg("useNTP")) {
        settings.useNTP = server.arg("useNTP").toInt();
    }
    if (server.hasArg("ntp1")) {
        strncpy(settings.ntpServer1, server.arg("ntp1").c_str(), sizeof(settings.ntpServer1) - 1);
    }
    if (server.hasArg("ntp2")) {
        strncpy(settings.ntpServer2, server.arg("ntp2").c_str(), sizeof(settings.ntpServer2) - 1);
    }
    if (server.hasArg("ntp3")) {
        strncpy(settings.ntpServer3, server.arg("ntp3").c_str(), sizeof(settings.ntpServer3) - 1);
    }
    if (server.hasArg("gmtOffset")) {
        settings.gmtOffsetSec = server.arg("gmtOffset").toInt();
    }
    if (server.hasArg("dstOffset")) {
        settings.daylightOffsetSec = server.arg("dstOffset").toInt();
    }
    if (server.hasArg("use24Hour")) {
        settings.use24Hour = server.arg("use24Hour").toInt();
    }
    if (server.hasArg("dateFormat")) {
        settings.dateFormat = server.arg("dateFormat").toInt();
    }


    if (server.hasArg("useMQTT")) {
        settings.useMQTT = server.arg("useMQTT").toInt();
    }
    if (server.hasArg("mqttBroker")) {
        strncpy(settings.mqttBroker, server.arg("mqttBroker").c_str(), sizeof(settings.mqttBroker) - 1);
    }
    if (server.hasArg("mqttPort")) {
        settings.mqttPort = server.arg("mqttPort").toInt();
    }
    if (server.hasArg("mqttUser")) {
        strncpy(settings.mqttUser, server.arg("mqttUser").c_str(), sizeof(settings.mqttUser) - 1);
    }
    if (server.hasArg("mqttPassword")) {
        strncpy(settings.mqttPassword, server.arg("mqttPassword").c_str(), sizeof(settings.mqttPassword) - 1);
    }
    if (server.hasArg("mqttClientId")) {
        strncpy(settings.mqttClientId, server.arg("mqttClientId").c_str(), sizeof(settings.mqttClientId) - 1);
    }
    if (server.hasArg("mqttSubTopic")) {
        strncpy(settings.mqttSubTopic, server.arg("mqttSubTopic").c_str(), sizeof(settings.mqttSubTopic) - 1);
    }
    if (server.hasArg("mqttPubTopic")) {
        strncpy(settings.mqttPubTopic, server.arg("mqttPubTopic").c_str(), sizeof(settings.mqttPubTopic) - 1);
    }
    if (server.hasArg("mqttPubInterval")) {
        settings.mqttPubInterval = server.arg("mqttPubInterval").toInt();
    }
    if (server.hasArg("useHADiscovery")) {
        settings.useHADiscovery = server.arg("useHADiscovery").toInt();
    }
    if (server.hasArg("quietEnabled")) {
        settings.quietHoursEnabled = server.arg("quietEnabled").toInt();
    }
    if (server.hasArg("quietStart")) {
        settings.quietStartHour = server.arg("quietStart").toInt();
    }
    if (server.hasArg("quietEnd")) {
        settings.quietEndHour = server.arg("quietEnd").toInt();
    }
    if (server.hasArg("quietBrightness")) {
        settings.quietBrightness = server.arg("quietBrightness").toInt();
    }

    saveSettings();

    String html =
        "<html><body>"
        "<h2>Settings saved.</h2>"
        "<p>Restarting SentraMatrix...</p>"
        "</body></html>";

    server.send(
        200,
        "text/html",
        html
    );

    delay(1000);

    ESP.restart();
}

// ============================================================
// RESTART
// ============================================================

void handleRestart()
{
    server.send(
        200,
        "text/html",
        "<h2>Restarting...</h2>"
    );

    delay(500);

    ESP.restart();
}

// ============================================================
// SERVER INIT
// ============================================================

void webServerInit()
{
    server.on(
        "/",
        HTTP_GET,
        handleRoot
    );

    server.on(
        "/hardware",
        HTTP_GET,
        handleHardware
    );

    server.on(
        "/wifi-scan",
        HTTP_GET,
        handleWiFiScan
    );

    server.on(
        "/wifi-connect",
        HTTP_POST,
        handleWiFiConnect
    );
    
    server.on(
        "/wifi-config",
        HTTP_GET,
        handleWiFiConfig
    );

    server.on(
        "/wifi-reset",
        HTTP_GET,
        handleWiFiReset
    );

    server.on(
        "/wifi",
        HTTP_GET,
        handleWiFi
    );

    server.on(
        "/time",
        HTTP_GET,
        handleTime
    );

    server.on(
        "/mqtt",
        HTTP_GET,
        handleMQTT
    );

    server.on(
        "/save",
        HTTP_POST,
        handleSave
    );

    server.on(
        "/restart",
        HTTP_GET,
        handleRestart
    );

    server.begin();

    Serial.println("Web server started.");

    Serial.print("Open: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
}

// ============================================================
// SERVER LOOP
// ============================================================

void webServerLoop()
{
    server.handleClient();
}
