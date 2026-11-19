#pragma once

#include <Arduino.h>
#include <MD_Parola.h>

// ============================================================
// TEXT EFFECTS
// Index-based table of all MD_Parola textEffect_t values.
// Settings store the table index (not the raw enum), so the
// stored value stays valid if the library enum changes.
// ============================================================

// Bump when the effect settings layout/defaults change
#define EFFECTS_VERSION 0xE1

// Index 0 is always "None" (PA_NO_EFFECT): valid only as exit effect
#define EFFECT_INDEX_NONE 0

uint8_t effectCount();
const char* effectName(uint8_t index);
textEffect_t effectValue(uint8_t index);

// Returns the table index for an effect value (0 if not found)
uint8_t effectIndexOf(textEffect_t effect);

// Accepts an effect name (case-insensitive) or a numeric index.
// Returns -1 if not found.
int effectParse(const String& value);

// Entry effects cannot be "None" (text would never appear)
bool effectValidIn(int index);
bool effectValidOut(int index);

// JSON array of effect names, e.g. ["Print","Scroll Up",...]
String effectOptionsJson(bool includeNone);

// Sprite used by the "Sprite" effect
void effectApplySprite(MD_Parola* display);
