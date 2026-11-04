#pragma once

// Optional local overrides (gitignored). Copy from config.local.h.example.
#if __has_include("config.local.h")
#include "config.local.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

// Home PC running Ollama. Use a LAN IP at home, or a Tailscale IP later.
#ifndef OLLAMA_HOST
#define OLLAMA_HOST "192.168.1.50"
#endif

#ifndef OLLAMA_PORT
#define OLLAMA_PORT 11434
#endif

// Any of these after `ollama pull`:
//   llama3.2:3b     (default)
//   qwen2.5:3b
//   gemma2:2b
//   phi3:mini
#ifndef OLLAMA_MODEL
#define OLLAMA_MODEL "llama3.2:3b"
#endif

// Andrew. The Neural ID is blocked by the TTS service; multilingual is the same speaker.
#ifndef TTS_VOICE
#define TTS_VOICE "en-US-AndrewMultilingualNeural"
#endif

#ifndef TTS_HOST
#define TTS_HOST ""
#endif

#ifndef TTS_PORT
#define TTS_PORT 43146
#endif
