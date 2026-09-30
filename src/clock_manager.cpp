#include "clock_manager.h"

#include <time.h>

#include "config.h"
#include "settings.h"
#include "display.h"
#include <MD_Parola.h>

// ============================================================
// STATE
// ============================================================

static int lastSecond = -1;
static int lastMinute = -1;

enum ClockState { STATE_CLOCK, STATE_DAY, STATE_DATE, STATE_MESSAGE };
static ClockState currentState = STATE_CLOCK;

static String currentDate = "";
static String currentDay = "";
static String currentTime = "";
static String currentMessage = "";

// ============================================================
// NTP INIT
// ============================================================

void clockInit()
{
    if (!settings.useNTP)
    {
        Serial.println("NTP is disabled in settings.");
        return;
    }

    Serial.println();
    Serial.println("Starting NTP...");

    configTime(
        settings.gmtOffsetSec,
        settings.daylightOffsetSec,
        settings.ntpServer1,
        settings.ntpServer2,
        settings.ntpServer3
    );

    Serial.println("Waiting for time...");

    time_t now = time(nullptr);
    unsigned long start = millis();

    while (now < 1700000000)
    {
        delay(500);
        yield();

        now = time(nullptr);

        Serial.print(".");
        
        if (millis() - start > 30000)
        {
            Serial.println();
            Serial.println("NTP timeout.");
            return;
        }
    }

    Serial.println();
    Serial.println("Time synchronized.");

    struct tm* timeinfo = localtime(&now);

    Serial.printf(
        "Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
        timeinfo->tm_year + 1900,
        timeinfo->tm_mon + 1,
        timeinfo->tm_mday,
        timeinfo->tm_hour,
        timeinfo->tm_min,
        timeinfo->tm_sec
    );

    lastSecond = -1;
    lastMinute = -1;
    currentState = STATE_CLOCK;
}

// ============================================================
// TIME VALID
// ============================================================

bool clockIsValid()
{
    return time(nullptr) >= 1700000000;
}

// ============================================================
// GET TIME
// HH:MM
// BLINK COLON
// ============================================================

String getCurrentTime()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char buffer[16];

    if (settings.use24Hour)
    {
        if (timeinfo->tm_sec % 2 == 0)
        {
            strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
        }
        else
        {
            strftime(buffer, sizeof(buffer), "%H;%M", timeinfo); // ';' = dark colon (same width)
        }
    }
    else
    {
        if (timeinfo->tm_sec % 2 == 0)
        {
            strftime(buffer, sizeof(buffer), "%I:%M", timeinfo);
        }
        else
        {
            strftime(buffer, sizeof(buffer), "%I;%M", timeinfo); // ';' = dark colon (same width)
        }

        // Strip leading zero for 12-hour format (e.g., 09:00 -> 9:00)
        if (buffer[0] == '0')
        {
            return String(buffer + 1);
        }
    }

    return String(buffer);
}

// ============================================================
// GET DATE
// ============================================================

String getCurrentDate()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char buffer[16];

    if (settings.dateFormat == 1)
    {
        // MM/DD/YYYY
        strftime(buffer, sizeof(buffer), "%m-%d-%Y", timeinfo);
    }
    else if (settings.dateFormat == 2)
    {
        // YYYY/MM/DD
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    }
    else
    {
        // DD/MM/YYYY
        strftime(buffer, sizeof(buffer), "%d-%m-%Y", timeinfo);
    }

    return String(buffer);
}

String getCurrentDay()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%A", timeinfo);
    return String(buffer);
}

bool isQuietHour()
{
    if (!settings.quietHoursEnabled) return false;
    time_t now = time(nullptr);
    if (now < 1700000000) return false;
    struct tm* t = localtime(&now);
    int h = t->tm_hour;
    if (settings.quietStartHour == settings.quietEndHour) return false;
    if (settings.quietStartHour < settings.quietEndHour)
        return (h >= settings.quietStartHour && h < settings.quietEndHour);
    else
        return (h >= settings.quietStartHour || h < settings.quietEndHour);
}

// ============================================================
// CLOCK UPDATE
// ============================================================

void clockUpdate()
{
    if (!clockIsValid()) return;

    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    int currentSecond = timeinfo->tm_sec;
    int currentMinute = timeinfo->tm_min;

    static bool inQuietState = false;
    bool quiet = isQuietHour();
    if (quiet != inQuietState)
    {
        inQuietState = quiet;
        if (inQuietState)
        {
            if (settings.quietBrightness == 0)
                displaySetPower(false);
            else
                displaySetBrightness(settings.quietBrightness);
        }
        else
        {
            displaySetPower(true);
            displaySetBrightness(settings.brightness);
            lastSecond = -1;
        }
    }

    if (currentState == STATE_MESSAGE)
    {
        if (displayIsAnimationFinished())
        {
            currentState = STATE_CLOCK;
            if (inQuietState && settings.quietBrightness == 0)
            {
                displaySetPower(false);
            }
            else
            {
                currentTime = getCurrentTime();
                displayClock(currentTime.c_str(), PA_SCROLL_UP, PA_SCROLL_UP);
            }
            lastSecond = currentSecond;
        }
        return;
    }

    if (inQuietState && settings.quietBrightness == 0)
    {
        return;
    }

    if (currentState == STATE_DAY)
    {
        if (displayIsAnimationFinished())
        {
            currentState = STATE_DATE;
            currentDate = getCurrentDate();
            displayDate(currentDate.c_str());
        }
        return;
    }

    if (currentState == STATE_DATE)
    {
        if (displayIsAnimationFinished())
        {
            currentState = STATE_CLOCK;
            currentTime = getCurrentTime();
            displayClock(currentTime.c_str(), PA_SCROLL_UP, PA_SCROLL_UP);
            lastSecond = currentSecond;
        }
        return;
    }

    if (lastSecond == -1)
    {
        lastSecond = currentSecond;
        lastMinute = currentMinute;
        currentTime = getCurrentTime();
        displayClock(currentTime.c_str(), PA_PRINT, PA_NO_EFFECT);
        return;
    }

    if (currentMinute != lastMinute)
    {
        lastMinute = currentMinute;
        if (!inQuietState)
        {
            currentState = STATE_DAY;
            currentDay = getCurrentDay();
            displayDate(currentDay.c_str());
            return;
        }
        else
        {
            currentTime = getCurrentTime();
            displayClock(currentTime.c_str(), PA_PRINT, PA_NO_EFFECT);
            return;
        }
    }

    if (currentSecond != lastSecond)
    {
        lastSecond = currentSecond;
        currentTime = getCurrentTime();
        if (displayIsAnimationFinished()) {
            displayClock(currentTime.c_str(), PA_PRINT, PA_NO_EFFECT);
        }
    }
}

void clockShowMessage(const String& msg)
{
    currentState = STATE_MESSAGE;
    currentMessage = msg;
    displaySetPower(true);
    displayDate(currentMessage.c_str());
}
