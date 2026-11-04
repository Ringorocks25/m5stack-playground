# StackChan ranger face

Custom firmware for the official **M5Stack StackChan (CoreS3)** that puts the red helmet on the screen, blinks the green visor eyes, and chatters the mouth.

The JPEG is compiled into the firmware. You do not need a microSD card.

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
- This sketch only drives the display. Head servos and RGB stay at hardware defaults.
