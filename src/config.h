#pragma once

#include <Arduino.h>
#include <MD_MAX72xx.h>

// ============================================================
// DEFAULT DISPLAY CONFIGURATION
// ============================================================

#define MATRIX_DATA_PIN D7
#define MATRIX_CLK_PIN  D5
#define MATRIX_CS_PIN   D4

#define DEFAULT_HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define DEFAULT_DEVICE_COUNT  4
#define DEFAULT_BRIGHTNESS   5

// ============================================================
// WIFI
// ============================================================

#define AP_NAME     "SentraMatrix"
#define AP_PASSWORD "12345678"

// ============================================================
// EEPROM
// ============================================================

#define EEPROM_SIZE 1024
#define SETTINGS_MAGIC 0x5352

// ============================================================
// TIME
// ============================================================

#define GMT_OFFSET_SEC 19800
#define DAYLIGHT_OFFSET_SEC 0

// ============================================================
// TEXT EFFECTS
// ============================================================

#define DEFAULT_EFFECT_SPEED 50   // ms per animation frame
#define MIN_EFFECT_SPEED     10
#define MAX_EFFECT_SPEED     200
#define DEFAULT_TEXT_PAUSE   0    // seconds
#define MAX_TEXT_PAUSE       30
#define CLOCK_EFFECT_SPEED   30   // ms per frame for the clock entry effect

#define TEST_DISPLAY_TIME 1200
#define DATE_DISPLAY_TIME 5000