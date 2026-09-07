#include "display.h"

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "config.h"
#include "settings.h"
#include "fonts.h"

// ============================================================
// DISPLAY OBJECT
// ============================================================

MD_Parola* display = nullptr;

// ============================================================
// CREATE DISPLAY
// ============================================================

void createDisplay()
{
    // Delete previous object if one exists
    if (display != nullptr)
    {
        delete display;
        display = nullptr;
    }

    Serial.println();
    Serial.println("Creating MAX7219 display...");

    Serial.print("Hardware type: ");
    Serial.println(settings.hardwareType);

    Serial.print("DATA pin: ");
    Serial.println(settings.dataPin);

    Serial.print("CLK pin: ");
    Serial.println(settings.clkPin);

    Serial.print("CS pin: ");
    Serial.println(settings.csPin);

    Serial.print("Modules: ");
    Serial.println(settings.deviceCount);

    display = new MD_Parola(
        static_cast<MD_MAX72XX::moduleType_t>(
            settings.hardwareType
        ),
        settings.dataPin,
        settings.clkPin,
        settings.csPin,
        settings.deviceCount
    );

    display->begin();

    display->setIntensity(
        settings.brightness
    );

    display->displayClear();

    Serial.println("MAX7219 initialized.");
}

// ============================================================
// INITIALIZE
// ============================================================

void displayInit()
{
    createDisplay();
}

// ============================================================
// STATIC BOOT MESSAGES
// ============================================================

void displayStatic(const char* text)
{
    if (display == nullptr) return;
    display->setFont(nullptr);
    display->displayClear();
    display->displayText(text, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    while(!display->displayAnimate()) { yield(); }
}

void displayScroll(const char* text)
{
    if (display == nullptr) return;
    display->setFont(nullptr);
    display->displayClear();
    display->displayText(text, PA_LEFT, 50, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    while (!display->displayAnimate()) { yield(); }
}

// ============================================================
// CLOCK
// ============================================================

void displayClock(const char* text, uint8_t effectIn, uint8_t effectOut)
{
    if (display == nullptr) return;
    display->setFont(clockFont);
     
    display->displayText(text, PA_CENTER, 30, 0, static_cast<textEffect_t>(effectIn), static_cast<textEffect_t>(effectOut));
}

// ============================================================
// DATE
// ============================================================

void displayDate(const char* text)
{
    if (display == nullptr)
        return;

    display->setFont(nullptr);
    // not clearing for scroll transition

    Serial.print("DISPLAY DATE: ");
    Serial.println(text);

    display->displayText(
        text,
        PA_LEFT,
        50,
        0,
        PA_SCROLL_LEFT,
        PA_SCROLL_LEFT
    );
}

// ============================================================
// CLEAR
// ============================================================

void displayClear()
{
    if (display == nullptr)
        return;

     
}

// ============================================================
// DISPLAY LOOP
// ============================================================

void displayLoop()
{
    if (display == nullptr)
        return;

    display->displayAnimate();
}
bool displayIsAnimationFinished()
{
    if (display == nullptr) return true;
    // MD_Parola's getZoneStatus() can check if animation is complete
    return display->getZoneStatus(0);
}
