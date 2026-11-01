#include "effects.h"

// ============================================================
// EFFECT TABLE
// ============================================================

struct EffectDef
{
    const char* name;
    textEffect_t value;
};

static const EffectDef EFFECTS[] =
{
    { "None",              PA_NO_EFFECT },
    { "Print",             PA_PRINT },
    { "Scroll Up",         PA_SCROLL_UP },
    { "Scroll Down",       PA_SCROLL_DOWN },
    { "Scroll Left",       PA_SCROLL_LEFT },
    { "Scroll Right",      PA_SCROLL_RIGHT },
#if ENA_SPRITE
    { "Sprite",            PA_SPRITE },
#endif
#if ENA_MISC
    { "Slice",             PA_SLICE },
    { "Mesh",              PA_MESH },
    { "Fade",              PA_FADE },
    { "Dissolve",          PA_DISSOLVE },
    { "Blinds",            PA_BLINDS },
    { "Random",            PA_RANDOM },
#endif
#if ENA_WIPE
    { "Wipe",              PA_WIPE },
    { "Wipe Cursor",       PA_WIPE_CURSOR },
#endif
#if ENA_SCAN
    { "Scan Horiz",        PA_SCAN_HORIZ },
    { "Scan HorizX",       PA_SCAN_HORIZX },
    { "Scan Vert",         PA_SCAN_VERT },
    { "Scan VertX",        PA_SCAN_VERTX },
#endif
#if ENA_OPNCLS
    { "Opening",           PA_OPENING },
    { "Opening Cursor",    PA_OPENING_CURSOR },
    { "Closing",           PA_CLOSING },
    { "Closing Cursor",    PA_CLOSING_CURSOR },
#endif
#if ENA_SCR_DIA
    { "Scroll Up Left",    PA_SCROLL_UP_LEFT },
    { "Scroll Up Right",   PA_SCROLL_UP_RIGHT },
    { "Scroll Down Left",  PA_SCROLL_DOWN_LEFT },
    { "Scroll Down Right", PA_SCROLL_DOWN_RIGHT },
#endif
#if ENA_GROW
    { "Grow Up",           PA_GROW_UP },
    { "Grow Down",         PA_GROW_DOWN },
#endif
};

static const uint8_t EFFECT_COUNT = sizeof(EFFECTS) / sizeof(EFFECTS[0]);

// ============================================================
// SPRITE DATA
// "Ghost pursued by a pacman" from the MD_Parola
// Parola_Sprites_Library example
// ============================================================

#if ENA_SPRITE
static const uint8_t F_PMAN2 = 6;
static const uint8_t W_PMAN2 = 18;
static const uint8_t PROGMEM pacman2[F_PMAN2 * W_PMAN2] =
{
    0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
    0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
    0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
    0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
    0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
    0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
};
#endif

// ============================================================
// API
// ============================================================

uint8_t effectCount()
{
    return EFFECT_COUNT;
}

const char* effectName(uint8_t index)
{
    if (index >= EFFECT_COUNT) return EFFECTS[EFFECT_INDEX_NONE].name;
    return EFFECTS[index].name;
}

textEffect_t effectValue(uint8_t index)
{
    if (index >= EFFECT_COUNT) return PA_NO_EFFECT;
    return EFFECTS[index].value;
}

uint8_t effectIndexOf(textEffect_t effect)
{
    for (uint8_t i = 0; i < EFFECT_COUNT; i++)
    {
        if (EFFECTS[i].value == effect) return i;
    }
    return EFFECT_INDEX_NONE;
}

int effectParse(const String& value)
{
    String v = value;
    v.trim();
    if (v.length() == 0) return -1;

    for (uint8_t i = 0; i < EFFECT_COUNT; i++)
    {
        if (v.equalsIgnoreCase(EFFECTS[i].name)) return i;
    }

    // Numeric index fallback (for manual MQTT publishing)
    bool numeric = true;
    for (unsigned int i = 0; i < v.length(); i++)
    {
        if (!isDigit(v[i])) { numeric = false; break; }
    }
    if (numeric)
    {
        int idx = v.toInt();
        if (idx >= 0 && idx < EFFECT_COUNT) return idx;
    }

    return -1;
}

bool effectValidIn(int index)
{
    return index > EFFECT_INDEX_NONE && index < EFFECT_COUNT;
}

bool effectValidOut(int index)
{
    return index >= 0 && index < EFFECT_COUNT;
}

String effectOptionsJson(bool includeNone)
{
    String json = "[";
    bool first = true;
    for (uint8_t i = includeNone ? 0 : 1; i < EFFECT_COUNT; i++)
    {
        if (!first) json += ",";
        json += "\"";
        json += EFFECTS[i].name;
        json += "\"";
        first = false;
    }
    json += "]";
    return json;
}

void effectApplySprite(MD_Parola* display)
{
#if ENA_SPRITE
    if (display == nullptr) return;
    display->setSpriteData(pacman2, W_PMAN2, F_PMAN2, pacman2, W_PMAN2, F_PMAN2);
#endif
}
