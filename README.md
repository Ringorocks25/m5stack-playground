# StackChan ranger face

Custom firmware for the official **M5Stack StackChan (CoreS3)**. The character is **Leroy**. He blinks, chatters the mouth, and speaks in Andrew’s voice. Boot line: **“Leroy online.”**

Tap the screen or type a line over serial to talk. At home it calls **Ollama** on your PC (`llama3.2:3b` by default). At work, or if Ollama is unreachable, it uses short offline Leroy lines.

The JPEG is compiled into the firmware. You do not need a microSD card.

## Pick a voice (hear it first)

The helmet cannot run a neural voice on-chip. Preview voices in the browser, then we can play that same voice from your PC later.

```bash
pip install edge-tts
cd voice-preview && npm install && npm run dev
```

The voice is **Andrew** (`en-US-AndrewMultilingualNeural`) speaking as **Leroy**. Boot and offline lines are baked into the firmware. For live Ollama replies, run a TTS helper on the PC:

```bash
python3 tools/tts_server.py
```

Set `TTS_HOST` in `src/config.local.h` to that PC’s IP, then flash again.

## What you need

- This StackChan (board: **M5CoreS3**)
- USB-C cable (use the port on the **base** if you can)
- [PlatformIO](https://platformio.org/) (CLI or the VS Code/Cursor extension)

## Build and flash

```bash
pio run -e m5stack-cores3 -t upload
pio device monitor
```

If the serial port does not appear, hold **RST about 2 seconds** until the green LED comes on, then release (download mode) and upload again.

### Arduino IDE instead

1. Board: **M5CoreS3** (M5Stack board manager ≥ 3.2.2)
2. Libraries: **M5Unified**, **M5GFX**
3. Open `src/main.cpp` (copy `src/face_jpg.h` next to it)
4. Upload

## Talk to an LLM (home PC)

The CoreS3 cannot run the model. Ollama stays on the PC. Pull one of these, then leave Ollama running:

```bash
ollama pull llama3.2:3b    # default in firmware
# ollama pull qwen2.5:3b
# ollama pull gemma2:2b
# ollama pull phi3:mini
```

Allow LAN clients (Ollama is localhost-only by default):

```bash
# Linux/macOS
OLLAMA_HOST=0.0.0.0 ollama serve
```

Copy `src/config.local.h.example` to `src/config.local.h` and set Wi-Fi plus the PC’s IP:

```cpp
#define WIFI_SSID "your-wifi"
#define WIFI_PASS "your-wifi-password"
#define OLLAMA_HOST "192.168.1.50"
#define OLLAMA_MODEL "llama3.2:3b"
```

`config.local.h` is gitignored. Flash, then:

- **Tap** the screen for a Leroy greeting
- **Serial monitor** (115200): type a sentence and press Enter

If Wi-Fi or Ollama fails, the helmet still answers with built-in lines. There is no on-device speech-to-text yet; serial is the keyboard.

## If the face is sideways

In `src/main.cpp` change `kFaceRotation` from `1` to `0`, `2`, or `3`, then flash again.

## Swap in a different picture

1. Replace `assets/ranger-helmet.png`
2. `python3 tools/embed_face.py` (needs Pillow: `pip install pillow`)
3. If the eyes moved, update `kLeftEyeX` / `kRightEyeX` / `kEyeY` in `src/main.cpp`
4. Flash again

## Restore the factory robot

Use [M5Burner](https://docs.m5stack.com/en/download), search **StackChan**, enable **Only Official**, and burn that image.

## Notes

- Screen is **320×240**. The helmet is letterboxed so it is not stretched.
- Blink is a black oval over each visor eye. Mouth motion covers the photo mouth with helmet red, then draws a smaller or larger opening. Neither uses a second photo.
- Head servos and RGB stay at hardware defaults.
- Default brain is Llama 3.2 3B through Ollama. Swap `OLLAMA_MODEL` for Qwen 2.5 3B, Gemma 2 2B, or Phi-3 mini.
