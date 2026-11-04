#include "ollama_chat.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "config.h"

static const char kSystemPrompt[] =
    "You are Leroy, a desk robot. Never call yourself a helmet. "
    "Stay in character. Replies must be one short spoken sentence, "
    "no lists, no markdown.";

static bool gWifiTried = false;
static bool gWifiOk = false;

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

bool wifiBegin() {
  gWifiTried = true;
  if (String(WIFI_SSID).isEmpty()) {
    Serial.println("[wifi] SSID not set; staying offline");
    gWifiOk = false;
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("[wifi] connecting to %s\n", WIFI_SSID);

  const uint32_t deadline = millis() + 15000;
  while (WiFi.status() != WL_CONNECTED && millis() < deadline) {
    delay(200);
  }

  gWifiOk = WiFi.status() == WL_CONNECTED;
  if (gWifiOk) {
    Serial.printf("[wifi] %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[wifi] failed; using offline lines");
  }
  return gWifiOk;
}

bool wifiReady() {
  if (!gWifiTried) {
    return false;
  }
  gWifiOk = WiFi.status() == WL_CONNECTED;
  return gWifiOk;
}

String ollamaChat(const String& userText) {
  if (!wifiReady()) {
    return "";
  }

  String url = String("http://") + OLLAMA_HOST + ":" + String(OLLAMA_PORT) +
               "/api/chat";
  String body = String("{\"model\":\"") + OLLAMA_MODEL +
                "\",\"stream\":false,\"messages\":["
                "{\"role\":\"system\",\"content\":\"" +
                jsonEscape(kSystemPrompt) +
                "\"},{\"role\":\"user\",\"content\":\"" + jsonEscape(userText) +
                "\"}]}";

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(45000);
  if (!http.begin(client, url)) {
    Serial.println("[ollama] begin failed");
    return "";
  }
  http.addHeader("Content-Type", "application/json");

  Serial.printf("[ollama] POST %s model=%s\n", url.c_str(), OLLAMA_MODEL);
  const int code = http.POST(body);
  String payload = http.getString();
  http.end();

  if (code != 200) {
    Serial.printf("[ollama] HTTP %d %s\n", code, payload.c_str());
    return "";
  }

  JsonDocument doc;
  const DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.printf("[ollama] json %s\n", err.c_str());
    return "";
  }

  const char* content = doc["message"]["content"];
  if (content == nullptr || content[0] == '\0') {
    Serial.println("[ollama] empty content");
    return "";
  }
  return String(content);
}
