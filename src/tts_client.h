#pragma once

#include <Arduino.h>

bool ttsFetchWav(const String& text, uint8_t** out, size_t* outLen);
