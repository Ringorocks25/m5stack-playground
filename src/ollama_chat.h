#pragma once

#include <Arduino.h>

bool wifiBegin();
bool wifiReady();
String ollamaChat(const String& userText);
