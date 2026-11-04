#include <M5Unified.h>

#include "andrew_wavs.h"
#include "config.h"
#include "face_jpg.h"
#include "ollama_chat.h"
#include "tts_client.h"

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

static const char* kOfflineLines[] = {
    "Leroy online. Brain is on the home PC.",
    "Tap me again when Ollama is in range.",
    "Leroy, standing by.",
    "Say something over serial and I will answer.",
};
static constexpr int kOfflineLineCount =
    sizeof(kOfflineLines) / sizeof(kOfflineLines[0]);

static uint16_t helmetRed;
static uint16_t mouthPink;

static bool blinking = false;
static uint32_t blinkUntilMs = 0;
static uint32_t nextBlinkMs = 0;

static int mouthH = kMouthOpenH;
static uint32_t nextMouthMs = 0;
static int talkStepsLeft = 0;
static bool busyTalking = false;

static String caption;
static int offlineIndex = 0;

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

static void drawCaption() {
  if (caption.isEmpty()) {
    return;
  }
  M5.Display.fillRect(0, 0, M5.Display.width(), 36, TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextDatum(top_left);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(4, 4);
  M5.Display.setTextWrap(true);
  M5.Display.print(caption);
}

static void paintOverlays() {
  if (blinking) {
    drawBlink();
  }
  if (mouthH != kMouthOpenH || talkStepsLeft > 0 || busyTalking) {
    drawMouth(mouthH);
  }
  drawCaption();
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

static void chatterOnce() {
  const int choice = esp_random() % 3;
  if (choice == 0) {
    mouthH = kMouthClosedH;
  } else if (choice == 1) {
    mouthH = kMouthClosedH + 8 + (esp_random() % 10);
  } else {
    mouthH = kMouthOpenH;
  }
  drawMouth(mouthH);
  drawCaption();
}

static const uint8_t* clipFor(const String& text, size_t* len) {
  if (text == "Leroy online.") {
    *len = boot_wav_len;
    return boot_wav;
  }
  if (text == kOfflineLines[0]) {
    *len = offline0_wav_len;
    return offline0_wav;
  }
  if (text == kOfflineLines[1]) {
    *len = offline1_wav_len;
    return offline1_wav;
  }
  if (text == kOfflineLines[2]) {
    *len = offline2_wav_len;
    return offline2_wav;
  }
  if (text == kOfflineLines[3]) {
    *len = offline3_wav_len;
    return offline3_wav;
  }
  *len = 0;
  return nullptr;
}

static void playWhileTalking(const uint8_t* wav, size_t len, bool owned) {
  M5.Speaker.playWav(wav, len);
  while (M5.Speaker.isPlaying()) {
    chatterOnce();
    M5.update();
    delay(90);
  }
  if (owned) {
    free(const_cast<uint8_t*>(wav));
  }
}

static void speakText(const String& text) {
  busyTalking = true;
  talkStepsLeft = 0;
  caption = text;
  redraw();
  Serial.printf("[say] %s\n", text.c_str());

  size_t clipLen = 0;
  const uint8_t* clip = clipFor(text, &clipLen);
  uint8_t* fetched = nullptr;
  size_t fetchedLen = 0;

  if (clip != nullptr) {
    playWhileTalking(clip, clipLen, false);
  } else if (ttsFetchWav(text, &fetched, &fetchedLen)) {
    playWhileTalking(fetched, fetchedLen, true);
  } else {
    const int steps = constrain(8 + (int)text.length() / 3, 8, 28);
    for (int i = 0; i < steps; i++) {
      chatterOnce();
      delay(110);
    }
  }

  mouthH = kMouthOpenH;
  drawMouth(mouthH);
  busyTalking = false;
  scheduleMouth(millis());
  scheduleBlink(millis());
}

static String fallbackLine() {
  const String line = kOfflineLines[offlineIndex];
  offlineIndex = (offlineIndex + 1) % kOfflineLineCount;
  return line;
}

static void handleUtterance(const String& userText) {
  caption = wifiReady() ? "Thinking..." : "Offline...";
  redraw();

  String reply = ollamaChat(userText);
  if (reply.isEmpty()) {
    reply = fallbackLine();
  }
  reply.trim();
  if (reply.length() > 140) {
    reply = reply.substring(0, 137) + "...";
  }
  speakText(reply);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  M5.Display.setRotation(kFaceRotation);
  M5.Display.setBrightness(200);
  helmetRed = M5.Display.color565(205, 1, 1);
  mouthPink = M5.Display.color565(252, 217, 209);
  M5.Speaker.setVolume(180);

  M5.Display.fillScreen(TFT_BLACK);
  const uint32_t now = millis();
  scheduleBlink(now);
  scheduleMouth(now);
  redraw();

  wifiBegin();
  if (wifiReady()) {
    caption = String("Ollama ") + OLLAMA_MODEL;
  } else {
    caption = "Leroy offline. Tap or type.";
  }
  redraw();

  Serial.println("Tap the screen or type a line over serial.");
  speakText("Leroy online.");
}

void loop() {
  M5.update();
  const uint32_t now = millis();

  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      handleUtterance(line);
    }
  }

  const auto touch = M5.Touch.getDetail();
  if (touch.wasClicked() && !busyTalking) {
    handleUtterance(
        "A human tapped you. Greet them as Leroy in one short sentence.");
  }

  if (busyTalking) {
    delay(10);
    return;
  }

  if (blinking && now >= blinkUntilMs) {
    blinking = false;
    scheduleBlink(now);
    redraw();
  } else if (!blinking && now >= nextBlinkMs) {
    blinking = true;
    blinkUntilMs = now + 120;
    drawBlink();
    drawCaption();
  }

  if (now >= nextMouthMs) {
    if (talkStepsLeft <= 0) {
      talkStepsLeft = 4 + (esp_random() % 4);
    }
    chatterOnce();
    talkStepsLeft--;
    if (talkStepsLeft <= 0) {
      mouthH = kMouthOpenH;
      drawMouth(mouthH);
      drawCaption();
      scheduleMouth(now);
    } else {
      nextMouthMs = now + 120 + (esp_random() % 140);
    }
  }

  delay(10);
}
