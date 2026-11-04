#include "tts_client.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_heap_caps.h>

#include "config.h"
#include "ollama_chat.h"

static String jsonEscape(const String& in) {
  String out;
  out.reserve(in.length() + 8);
  for (size_t i = 0; i < in.length(); i++) {
    const char c = in[i];
    if (c == '\\' || c == '"') {
      out += '\\';
      out += c;
    } else if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else {
      out += c;
    }
  }
  return out;
}

bool ttsFetchWav(const String& text, uint8_t** out, size_t* outLen) {
  *out = nullptr;
  *outLen = 0;
  if (!wifiReady() || String(TTS_HOST).isEmpty()) {
    return false;
  }

  String url = String("http://") + TTS_HOST + ":" + String(TTS_PORT) + "/speak";
  String body = String("{\"voice\":\"") + TTS_VOICE +
                "\",\"text\":\"" + jsonEscape(text) + "\"}";

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(20000);
  if (!http.begin(client, url)) {
    Serial.println("[tts] begin failed");
    return false;
  }
  http.addHeader("Content-Type", "application/json");
  const int code = http.POST(body);
  if (code != 200) {
    Serial.printf("[tts] HTTP %d\n", code);
    http.end();
    return false;
  }

  const int len = http.getSize();
  if (len <= 44 || len > 800000) {
    Serial.printf("[tts] bad size %d\n", len);
    http.end();
    return false;
  }

  uint8_t* buf = static_cast<uint8_t*>(
      heap_caps_malloc(len, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM));
  if (buf == nullptr) {
    buf = static_cast<uint8_t*>(malloc(len));
  }
  if (buf == nullptr) {
    Serial.println("[tts] oom");
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  int got = 0;
  const uint32_t deadline = millis() + 20000;
  while (got < len && millis() < deadline) {
    const int avail = stream->available();
    if (avail <= 0) {
      delay(10);
      continue;
    }
    const int n = stream->readBytes(buf + got, len - got);
    if (n <= 0) {
      break;
    }
    got += n;
  }
  http.end();

  if (got < 44 || buf[0] != 'R' || buf[1] != 'I' || buf[2] != 'F' ||
      buf[3] != 'F') {
    Serial.println("[tts] not a wav");
    free(buf);
    return false;
  }

  *out = buf;
  *outLen = static_cast<size_t>(got);
  Serial.printf("[tts] wav %d bytes\n", got);
  return true;
}
