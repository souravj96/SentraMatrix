#include "display.h"

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "config.h"
#include "settings.h"
#include "fonts.h"
#include "effects.h"

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

    effectApplySprite(display);
    display->setInvert(settings.invertDisplay);

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
     
    display->displayText(text, PA_CENTER, CLOCK_EFFECT_SPEED, 0, static_cast<textEffect_t>(effectIn), static_cast<textEffect_t>(effectOut));
}

// ============================================================
// DATE
// ============================================================

void displayDate(const char* text, uint8_t effectIn, uint8_t effectOut)
{
    if (display == nullptr)
        return;

    display->setFont(nullptr);
    // not clearing for scroll transition

    Serial.print("DISPLAY DATE: ");
    Serial.println(text);

    textEffect_t in = static_cast<textEffect_t>(effectIn);
    textEffect_t out = static_cast<textEffect_t>(effectOut);

    // Text wider than the display can only be shown entirely by
    // scrolling: force a horizontal scroll entry in that case.
    if (in != PA_SCROLL_LEFT && in != PA_SCROLL_RIGHT)
    {
        uint16_t startCol, endCol;
        display->getDisplayExtent(startCol, endCol);
        if (display->getTextColumns(text) > (endCol - startCol + 1))
        {
            in = PA_SCROLL_LEFT;
        }
    }

    bool scrolling = (in == PA_SCROLL_LEFT || in == PA_SCROLL_RIGHT);

    display->displayText(
        text,
        scrolling ? PA_LEFT : PA_CENTER,
        settings.effectSpeed,
        (uint16_t)settings.textPause * 1000,
        in,
        out
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

void displaySetBrightness(uint8_t brightness)
{
    if (display == nullptr) return;
    if (brightness > 15) brightness = 15;
    display->setIntensity(brightness);
}

void displaySetPower(bool on)
{
    if (display == nullptr) return;
    display->displayShutdown(!on);
}

void displaySetInvert(bool invert)
{
    if (display == nullptr) return;
    display->setInvert(invert);
}


