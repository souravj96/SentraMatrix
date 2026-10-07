<div align="center">

# SentraMatrix

**Open-source ESP8266 & ESP32 LED matrix clock and smart display controller**

[![Build Status](https://github.com/souravj96/SentraMatrix/actions/workflows/build.yml/badge.svg)](https://github.com/souravj96/SentraMatrix/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform: ESP8266](https://img.shields.io/badge/Platform-ESP8266-blue.svg)](https://www.espressif.com/en/products/socs/esp8266)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-red.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-teal.svg)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/Built%20with-PlatformIO-orange.svg)](https://platformio.org/)

</div>

---

## 📖 Overview

SentraMatrix is a fully configurable LED matrix clock and smart display controller built on **ESP8266 NodeMCU** and **ESP32** with **MAX7219** LED matrix modules.

It combines an NTP-synchronized clock, scrolling date/day display, MQTT text message integration, and a complete browser-based configuration UI — all without any cloud dependency.

---

## ✨ Features

| Feature | Status |
|---|---|
| Dual Platform Support (ESP8266 & ESP32) | ✅ |
| NTP time synchronization | ✅ |
| 12 / 24-hour clock display | ✅ |
| Day name scrolling (every minute) | ✅ |
| Date scrolling (DD-MM-YYYY / MM-DD-YYYY / YYYY-MM-DD) | ✅ |
| Digit swipe-up animation on time change | ✅ |
| Custom bold clock font | ✅ |
| WiFi auto-connect (WiFiManager) | ✅ |
| Fallback AP configuration portal | ✅ |
| MQTT incoming message display (scrolling) | ✅ |
| MQTT periodic status publishing | ✅ |
| Boot status display (WiFi / IP / NTP / MQTT) | ✅ |
| Dark web configuration UI | ✅ |
| Hardware configuration (pins, brightness, module count) | ✅ |
| Time / timezone configuration | ✅ |
| MQTT configuration | ✅ |
| Persistent settings (EEPROM) | ✅ |
| Home Assistant MQTT Auto-Discovery | ✅ |
| Quiet Hours / Night Mode | ✅ |
| GitHub Actions CI — dual ESP8266 & ESP32 binary release | ✅ |

---

## 🔧 Hardware

### Required

| Component | Notes |
|---|---|
| ESP8266 NodeMCU v2 or ESP32 DevKit | Any ESP8266 / ESP32 board |
| MAX7219 LED Matrix module(s) | ICSTATION or compatible (4 modules standard) |
| 5V power supply | At least 2A recommended for 4 modules |

### Default Wiring

#### ESP8266 NodeMCU v2

| Signal | NodeMCU pin | GPIO |
|---|---|---|
| DIN (DATA) | D7 | GPIO13 |
| CLK | D5 | GPIO14 |
| CS | D4 | GPIO2 |
| VCC | 5V / VIN | 5V |
| GND | GND | GND |

#### ESP32 DevKit (VSPI)

| Signal | Pin | GPIO |
|---|---|---|
| DIN (DATA) | D23 / MOSI | GPIO23 |
| CLK | D18 / SCK | GPIO18 |
| CS | D5 / SS | GPIO5 |
| VCC | 5V / VIN | 5V |
| GND | GND | GND |

> **Note:** Pins can be reconfigured via the Hardware page in the web UI.

### Tested Configurations

```
- ESP8266 NodeMCU v2 + 4× ICSTATION MAX7219 modules (32×8 matrix)
- ESP32-WROOM-32 DevKit + 4× ICSTATION MAX7219 modules (32×8 matrix)
```

---

## 🚀 Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (IDE extension or CLI)
- USB cable for flashing
- A WiFi network (2.4 GHz)

### Clone & Build

```bash
git clone https://github.com/souravj96/SentraMatrix.git
cd SentraClock

# Build for ESP8266
pio run -e nodemcuv2

# Build for ESP32
pio run -e esp32dev

# Build both
pio run
```

### Flash

```bash
# Flash ESP8266
pio run -e nodemcuv2 --target upload

# Flash ESP32
pio run -e esp32dev --target upload
```

Or use the PlatformIO IDE's **Upload** button.

### First Boot

1. Power on the device. The matrix display will show the boot sequence:
   - `SentraMatrix` (scrolling)
   - `WiFi...`
   - Displays IP address once connected
   - `NTP...` (if NTP enabled)
   - `MQTT...` (if MQTT enabled)
2. If no saved WiFi credentials exist, the device opens a captive portal AP:
   - **SSID:** `SentraMatrix`
   - **Password:** `12345678`
3. Connect to the AP, configure your WiFi network, and save.
4. The device reconnects and starts displaying the time.

### Web UI

Open a browser and navigate to the device IP address shown on boot.

| Page | Path | Description |
|---|---|---|
| Home | `/` | Device, WiFi, Display, MQTT status |
| Hardware | `/hardware` | Pins, module count, brightness, Quiet Hours |
| WiFi | `/wifi` | Scan, connect, AP settings |
| Time | `/time` | NTP, timezone, 12/24hr, date format |
| MQTT | `/mqtt` | Broker, topics, credentials |

---

## 📡 MQTT

SentraMatrix subscribes to a configurable topic and scrolls any incoming payload across the matrix.

### Subscribe topic (incoming messages → display)

```
sentramatrix/message
```

### Publish topic (periodic device status)

```
sentramatrix/status
```

### Example — send a message with Mosquitto

```bash
mosquitto_pub -h <broker-ip> -t "sentramatrix/message" -m "Hello!"
```

### Example status payload

```json
{
  "status": "online",
  "ip": "192.168.1.50",
  "rssi": -67,
  "heap": 28432,
  "uptime": 12345,
  "brightness": 5,
  "quiet_hours": "ON",
  "quiet_start": 22,
  "quiet_end": 7,
  "quiet_brightness": 0
}
```

### 🏠 Home Assistant MQTT Auto-Discovery

SentraMatrix automatically registers itself in Home Assistant as a unified device via MQTT Discovery (`homeassistant/...`). No manual YAML configuration required!

**Discovered Entities:**
- **Display Message (`text`)**: Send custom messages directly from the HA UI or automation scripts.
- **Brightness (`number`)**: Real-time slider (0–15) to dynamically adjust LED matrix intensity.
- **Quiet Hours (`switch`)**: Toggle quiet hours mode on/off.
- **Quiet Start Hour (`number`)**: Start hour (0–23, e.g. `22` for 10 PM).
- **Quiet End Hour (`number`)**: End hour (0–23, e.g. `7` for 7 AM).
- **Quiet Brightness (`number`)**: Night brightness level (0 = Display completely powered off, 1–15 = Dim).
- **Restart (`button`)**: Reboot the device remotely from Home Assistant.
- **Sensors (`sensor`)**: Real-time WiFi Signal (RSSI), IP Address, Free Memory (Heap), and Uptime.
- **Availability (LWT)**: Automatic online/offline status reporting via MQTT Last Will and Testament.

#### Sending messages via Home Assistant Automations

```yaml
# Send an alert to SentraMatrix
action: mqtt.publish
data:
  topic: sentramatrix/message
  payload: "Door opened!"
```

---

## 🌙 Quiet Hours / Night Mode

Quiet Hours allows you to dim or completely power down the matrix during sleeping hours, configurable from both the **Web UI (`/hardware`)** and **Home Assistant**.

### Key Behaviors:
- **Time Span Support**: Seamlessly handles overnight schedules spanning past midnight (e.g. Start: `22` / 10 PM → End: `07` / 7 AM).
- **Zero Light Emission (Brightness = 0)**: Uses hardware `displayShutdown(true)` to power down the MAX7219 modules so LEDs emit zero light in a dark bedroom.
- **Dimmed Display (Brightness > 0)**: Keeps the clock readable at a gentle, non-distracting intensity level.
- **Suppressed Minute Animations**: Disables the periodic date/day marquee scroll during quiet hours so motion doesn't distract.
- **Wake on Incoming Message**: If an MQTT message arrives during quiet hours, the display temporarily wakes up, scrolls the text, and returns to sleep/dim mode once finished.

---

## 🗂️ Project Structure

```
SentraMatrix/
├── src/
│   ├── main.cpp            # Setup + loop
│   ├── config.h            # Compile-time constants and defaults
│   ├── settings.h/.cpp     # EEPROM settings struct + load/save
│   ├── display.h/.cpp      # MD_Parola display abstraction
│   ├── fonts.h             # Custom clock font (bold double-line digits)
│   ├── clock_manager.h/.cpp# NTP clock, state machine (Clock→Day→Date)
│   ├── wifi_manager.h/.cpp # WiFi connect + AP fallback
│   ├── web_server.h/.cpp   # ESP8266WebServer + dark web UI
│   └── mqtt_manager.h/.cpp # PubSubClient MQTT wrapper
├── .github/
│   └── workflows/
│       └── build.yml       # CI: build + upload artifact + GitHub Release
├── platformio.ini
└── README.md
```

---

## ⚙️ Configuration Reference

Key compile-time defaults in [`src/config.h`](src/config.h):

| Macro | Default | Description |
|---|---|---|
| `MATRIX_DATA_PIN` | `D7` | SPI DATA pin |
| `MATRIX_CLK_PIN` | `D5` | SPI CLK pin |
| `MATRIX_CS_PIN` | `D4` | SPI CS pin |
| `DEFAULT_HARDWARE_TYPE` | `ICSTATION_HW` | MAX7219 module type |
| `DEFAULT_DEVICE_COUNT` | `4` | Number of chained modules |
| `DEFAULT_BRIGHTNESS` | `5` | Brightness 0–15 |
| `AP_NAME` | `SentraMatrix` | Fallback AP SSID |
| `AP_PASSWORD` | `12345678` | Fallback AP password |
| `EEPROM_SIZE` | `1024` | Bytes allocated for settings |
| `GMT_OFFSET_SEC` | `19800` | Default timezone (IST +5:30) |
| `DATE_DISPLAY_TIME` | `5000` | How long date shows (ms) |

> All of these can be overridden at runtime via the web UI and persisted to EEPROM.

---

## 📦 Libraries

| Library | Version | Purpose |
|---|---|---|
| [MD_Parola](https://github.com/MajicDesigns/MD_Parola) | `^3.7.5` | LED matrix text effects |
| [MD_MAX72XX](https://github.com/MajicDesigns/MD_MAX72XX) | `^3.5.1` | MAX7219/7221 driver |
| [WiFiManager](https://github.com/tzapu/WiFiManager) | `2.0.16-rc.2` | WiFi config portal |
| [PubSubClient](https://github.com/knolleary/pubsubclient) | `^2.8` | MQTT client |

> **⚠️ WiFiManager:** Do **not** upgrade past `2.0.16-rc.2` — `2.0.17` causes a crash on ESP8266 with this build.

---

## 🤝 Contributing

Contributions are welcome!

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make your changes and test on real hardware
4. Commit: `git commit -m "feat: describe your change"`
5. Push: `git push origin feature/my-feature`
6. Open a Pull Request

When submitting hardware-related changes, please include:
- ESP8266 board used
- MAX7219 module type
- Number of modules
- PlatformIO version
- Serial output (115200 baud) if relevant

---

## 🐛 Reporting Issues

Please include:

```
ESP8266 board:
MAX7219 module type:
Number of modules:
PlatformIO version:
SentraMatrix commit (git log --oneline -1):
Serial output (115200 baud):
```

---

## 🔐 Security

SentraMatrix is designed for **local network use only**.

- Change the default AP password before deploying in a shared environment.
- Do **not** expose the web server directly to the internet.
- Do **not** commit WiFi or MQTT credentials to version control.
- The EEPROM settings include MQTT credentials — take care with JTAG/serial dumps.

---

## 🗺️ Roadmap

### ✅ Done

- ESP8266 + PlatformIO project structure
- MAX7219 / MD_Parola display driver
- WiFiManager auto-connect + AP fallback
- NTP time synchronization
- Custom bold clock font
- Clock → Day → Date state machine
- Digit swipe-up animation
- MQTT subscribe (display incoming message)
- MQTT publish (periodic status)
- Boot display sequence (IP, NTP, MQTT status)
- Dark web configuration UI (Hardware / WiFi / Time / MQTT)
- Persistent EEPROM settings
- Home Assistant MQTT Auto-Discovery
- Quiet Hours / Night Mode (Web UI & Home Assistant)
- GitHub Actions CI + Release

### 🔮 Future Ideas

- OTA firmware updates
- Factory reset button / web trigger
- Config backup/restore (JSON export)
- REST API
- Multiple display zones
- Scheduled messages
- Sensor integration (temperature, humidity)
- Mobile-optimized UI
- Custom scroll speed per message

---

## 📜 License

MIT License — see [LICENSE](LICENSE) for full text.

```
Copyright (c) 2026 Sourav Jana
```

---

## ❤️ Credits

- [MD_Parola](https://github.com/MajicDesigns/MD_Parola) — Marco Colli
- [MD_MAX72XX](https://github.com/MajicDesigns/MD_MAX72XX) — Marco Colli
- [WiFiManager](https://github.com/tzapu/WiFiManager) — tzapu
- [PubSubClient](https://github.com/knolleary/pubsubclient) — Nick O'Leary

---

<div align="center">

**Built for makers. Built to be configurable. Built to be extended.**

⭐ Star the repo if you find it useful!

</div>
