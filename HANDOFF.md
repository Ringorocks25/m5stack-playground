# Handoff: Leroy (StackChan CoreS3)

Use this file as the start of **local** work. Open the GitHub clone in Cursor Desktop and talk to a **local** Agent (not Cloud). The M5 is plugged into that PC; the cloud agent cannot see USB.

## Repo

- GitHub: https://github.com/Ringorocks25/m5stack-playground
- Branch: `main`
- Clone path on the last machine: `C:\Users\Owner\Documents\m5stack-playground`

```bat
git clone https://github.com/Ringorocks25/m5stack-playground.git
cd m5stack-playground
```

Do **not** clone `origin.cursor.com`. That is Cursor’s private git and will 401 unless Origin CLI is logged in. GitHub is the copy to use.

## Hardware

Official **M5Stack StackChan**, controller **CoreS3** (ESP32-S3). FCC / case: StackChan.

- Flash via USB-C on the **base**, not the head if you can avoid it
- Download mode: hold **RST ~2 seconds** until the green LED, then release
- Board in PlatformIO / Arduino: `m5stack-cores3` / **M5CoreS3**
- Also owned: StackChan **MiniJoyC + StickC-Plus remote** (ESP-NOW). Firmware does **not** speak that protocol yet
- Do not flash Core2 images

Factory firmware restore: [M5Burner](https://docs.m5stack.com/en/download) → search **StackChan** → Only Official.

## Character

- Name: **Leroy** (not “helmet”, not Andrew)
- Face: red Strong Bad–style helmet JPEG, 320×240, baked into `src/face_jpg.h`
- Voice: **Andrew** neural TTS (`en-US-AndrewMultilingualNeural`). `en-US-AndrewNeural` is often blocked by the TTS service
- Boot line (spoken + caption): **“Leroy online.”**
- Idle: visor blink, mouth chatter
- Tap / serial: talk. Offline baked lines if Ollama is down

## What already works

| Piece | Where |
|---|---|
| Face + blink + mouth overlay | `src/main.cpp` |
| Andrew clips for boot + 4 offline lines | `src/andrew_wavs.h`, sources in `assets/voice/andrew/` |
| Speaker playback while mouth moves | `M5.Speaker.playWav` |
| Optional Wi-Fi + Ollama chat | `src/ollama_chat.cpp`, `src/config.h` |
| Optional live TTS from a PC helper | `src/tts_client.cpp`, `tools/tts_server.py` |
| Voice picker (browser) | `voice-preview/` (port 43145) |

## What is not in firmware yet

- Head servos / idle look-around
- RGB LEDs, body touch, NFC, IR
- Proximity sleep, IMU dizzy
- ESP-NOW remote (the M5 Stick remote)
- On-device STT or on-device LLM
- Strong Bad’s actual voice (do not clone)

## Flash on Windows (this PC)

Python from **python.org**, with **Add python.exe to PATH**. Do not use the Microsoft Store stub.

```bat
cd C:\Users\Owner\Documents\m5stack-playground
python -m pip install -U platformio
python -m platformio run -e m5stack-cores3 -t upload
python -m platformio device monitor
```

Or Cursor extension **PlatformIO IDE** → Upload, env `m5stack-cores3`.

If the face is sideways, change `kFaceRotation` in `src/main.cpp` (`0`–`3`) and upload again.

## Local secrets (gitignored)

```bat
copy src\config.local.h.example src\config.local.h
```

Edit Wi-Fi, Ollama IP, TTS IP. Never commit `src/config.local.h`.

```cpp
#define WIFI_SSID "your-wifi"
#define WIFI_PASS "your-wifi-password"
#define OLLAMA_HOST "192.168.1.50"
#define OLLAMA_MODEL "llama3.2:3b"
#define TTS_VOICE "en-US-AndrewMultilingualNeural"
#define TTS_HOST "192.168.1.50"
#define TTS_PORT 43146
```

Home PC (Ollama + live speech):

```bat
ollama pull llama3.2:3b
set OLLAMA_HOST=0.0.0.0
ollama serve
python tools\tts_server.py
```

Need `edge-tts` and `ffmpeg` for the TTS helper. Without `TTS_HOST`, baked clips still play; new LLM sentences only move the mouth.

## Layout

```
platformio.ini          CoreS3 env
src/main.cpp            Face, talk loop, clip lookup
src/config.h            Defaults + includes config.local.h
src/ollama_chat.*       HTTP to Ollama /api/chat
src/tts_client.*        HTTP WAV fetch
src/face_jpg.h          Embedded 320×240 face
src/andrew_wavs.h       Embedded 16 kHz mono WAVs
tools/embed_face.py     Rebuild face header from PNG
tools/tts_server.py     Local Andrew WAV server
assets/                 Source PNG + WAVs
voice-preview/          Next.js voice lab
```

Rebuild face: replace `assets/ranger-helmet.png`, run `python tools/embed_face.py`, then fix eye/mouth constants if the art moved.

## Local Agent prompt (paste this)

You are working in the local clone of github.com/Ringorocks25/m5stack-playground. Hardware is an M5Stack StackChan CoreS3 named Leroy, USB on this PC. Flash with `python -m platformio run -e m5stack-cores3 -t upload`. Voice is Andrew multilingual; spoken identity is Leroy. Do not clone cartoon voices. Read HANDOFF.md and README.md before editing.

## Next useful firmware (if you continue)

1. M5StackChan BSP: servos + RGB + body touch (`m5stack/StackChan-BSP`)
2. ESP-NOW receiver for the official StickC-Plus remote (channel must match Wi-Fi if STA is on)
3. Shake (IMU) and proximity sleep
4. Wire `TTS_HOST` on the home LAN and test tap → Ollama → Andrew speech
