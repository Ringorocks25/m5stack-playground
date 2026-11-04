#include <M5Unified.h>

#include "face_jpg.h"

// Screen is 320x240. These match the scaled helmet JPEG in src/face_jpg.h.
static constexpr int kLeftEyeX = 125;
static constexpr int kRightEyeX = 194;
static constexpr int kEyeY = 115;
static constexpr int kEyeRx = 16;
static constexpr int kEyeRy = 14;

static constexpr int kMouthX = 161;
static constexpr int kMouthY = 191;
static constexpr int kMouthW = 55;
static constexpr int kMouthOpenH = 36;
static constexpr int kMouthClosedH = 5;
static constexpr int kMouthCoverRx = 32;
static constexpr int kMouthCoverRy = 24;

// If the face is sideways after flashing, try 0, 1, 2, or 3.
static constexpr int kFaceRotation = 1;

static uint16_t helmetRed;
static uint16_t mouthPink;

static bool blinking = false;
static uint32_t blinkUntilMs = 0;
static uint32_t nextBlinkMs = 0;

static int mouthH = kMouthOpenH;
static uint32_t nextMouthMs = 0;
static int talkStepsLeft = 0;

static void drawFace() {
  M5.Display.drawJpg(face_jpg, face_jpg_len, 0, 0, M5.Display.width(),
                     M5.Display.height());
}

static void drawBlink() {
  M5.Display.fillEllipse(kLeftEyeX, kEyeY, kEyeRx, kEyeRy, TFT_BLACK);
  M5.Display.fillEllipse(kRightEyeX, kEyeY, kEyeRx, kEyeRy, TFT_BLACK);
}

static void drawMouth(int height) {
  M5.Display.fillEllipse(kMouthX, kMouthY, kMouthCoverRx, kMouthCoverRy,
                         helmetRed);
  height = constrain(height, kMouthClosedH, kMouthOpenH);
  const int x = kMouthX - (kMouthW / 2);
  const int y = kMouthY - (height / 2);
  const int radius = min(12, height / 2);
  M5.Display.fillRoundRect(x, y, kMouthW, height, radius, mouthPink);
}

static void paintOverlays() {
  if (blinking) {
    drawBlink();
  }
  if (mouthH != kMouthOpenH || talkStepsLeft > 0) {
    drawMouth(mouthH);
  }
}

static void redraw() {
  drawFace();
  paintOverlays();
}

static void scheduleBlink(uint32_t now) {
  nextBlinkMs = now + 2500 + (esp_random() % 2500);
}

static void scheduleMouth(uint32_t now) {
  nextMouthMs = now + 2800 + (esp_random() % 3200);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(kFaceRotation);
  M5.Display.setBrightness(200);
  helmetRed = M5.Display.color565(205, 1, 1);
  mouthPink = M5.Display.color565(252, 217, 209);

  M5.Display.fillScreen(TFT_BLACK);
  const uint32_t now = millis();
  scheduleBlink(now);
  scheduleMouth(now);
  redraw();
}

void loop() {
  const uint32_t now = millis();

  if (blinking && now >= blinkUntilMs) {
    blinking = false;
    scheduleBlink(now);
    redraw();
  } else if (!blinking && now >= nextBlinkMs) {
    blinking = true;
    blinkUntilMs = now + 120;
    drawBlink();
  }

  if (now >= nextMouthMs) {
    if (talkStepsLeft <= 0) {
      talkStepsLeft = 4 + (esp_random() % 4);
    }

    const int choice = esp_random() % 3;
    if (choice == 0) {
      mouthH = kMouthClosedH;
    } else if (choice == 1) {
      mouthH = kMouthClosedH + 8 + (esp_random() % 10);
    } else {
      mouthH = kMouthOpenH;
    }
    drawMouth(mouthH);

    talkStepsLeft--;
    if (talkStepsLeft <= 0) {
      mouthH = kMouthOpenH;
      drawMouth(mouthH);
      scheduleMouth(now);
    } else {
      nextMouthMs = now + 120 + (esp_random() % 140);
    }
  }

  delay(10);
}
