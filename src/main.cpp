#include <M5Unified.h>

#include "face_jpg.h"

// Screen is 320x240. These match the scaled helmet JPEG in src/face_jpg.h.
static constexpr int kLeftEyeX = 125;
static constexpr int kRightEyeX = 194;
static constexpr int kEyeY = 115;
static constexpr int kEyeRx = 16;
static constexpr int kEyeRy = 14;

// If the face is sideways after flashing, try 0, 1, 2, or 3.
static constexpr int kFaceRotation = 1;

static void drawFace() {
  M5.Display.drawJpg(face_jpg, face_jpg_len, 0, 0, M5.Display.width(),
                     M5.Display.height());
}

static void drawBlink() {
  M5.Display.fillEllipse(kLeftEyeX, kEyeY, kEyeRx, kEyeRy, TFT_BLACK);
  M5.Display.fillEllipse(kRightEyeX, kEyeY, kEyeRx, kEyeRy, TFT_BLACK);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(kFaceRotation);
  M5.Display.setBrightness(200);
  M5.Display.fillScreen(TFT_BLACK);
  drawFace();
}

void loop() {
  delay(2500 + (esp_random() % 2500));
  drawBlink();
  delay(120);
  drawFace();
}
