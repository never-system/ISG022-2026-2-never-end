#include "network/stream_service.h"

#include <WiFi.h>

#include "camera/camera_service.h"
#include "debug/logger.h"
#include "system/app_config.h"

namespace {

WiFiServer gServer(app::kStreamPort);
WiFiClient gStreamClient;
bool gStarted = false;
bool gEnabled = false;
uint32_t gLastFrameMs = 0;

void stopStreamClient() {
  if (gStreamClient) {
    gStreamClient.stop();
  }
}

void discardHeaders(WiFiClient &client) {
  uint32_t start = millis();
  while (client.connected() && millis() - start < 300) {
    if (!client.available()) {
      delay(1);
      continue;
    }

    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      return;
    }
  }
}

void sendText(WiFiClient &client, const char *status, const String &body) {
  client.printf("HTTP/1.1 %s\r\n", status);
  client.println("Content-Type: text/plain; charset=utf-8");
  client.printf("Content-Length: %u\r\n", static_cast<unsigned>(body.length()));
  client.println("Connection: close\r\n");
  client.print(body);
  client.stop();
}

void sendRoot(WiFiClient &client) {
  String body;
  body.reserve(180);
  body += "Never Dice camera stream\n";
  body += "Status: ";
  body += gEnabled ? "ON\n" : "OFF\n";
  body += "Stream: ";
  body += stream_service::streamUrl();
  body += "\n";
  sendText(client, "200 OK", body);
}

void startStream(WiFiClient &client) {
  if (!gEnabled) {
    sendText(client, "403 Forbidden", "Streaming desligado. Use STREAM ON ou MQTT commands/stream=on.\n");
    return;
  }

  if (gStreamClient && gStreamClient.connected()) {
    sendText(client, "409 Conflict", "Ja existe um cliente de streaming conectado.\n");
    return;
  }

  gStreamClient = client;
  gStreamClient.println("HTTP/1.1 200 OK");
  gStreamClient.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  gStreamClient.println("Cache-Control: no-cache");
  gStreamClient.println("Pragma: no-cache");
  gStreamClient.println("Connection: close\r\n");
  gLastFrameMs = 0;
}

void sendCapture(WiFiClient &client) {
  if (!gEnabled) {
    sendText(client, "403 Forbidden", "Streaming desligado.\n");
    return;
  }

  camera_fb_t *fb = camera_service::capture();
  if (fb == nullptr) {
    sendText(client, "503 Service Unavailable", "Camera indisponivel.\n");
    return;
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.printf("Content-Length: %u\r\n", static_cast<unsigned>(fb->len));
  client.println("Connection: close\r\n");
  client.write(fb->buf, fb->len);
  camera_service::release(fb);
  client.stop();
}

void handleClient(WiFiClient &client) {
  client.setTimeout(250);
  String requestLine = client.readStringUntil('\r');
  client.readStringUntil('\n');
  requestLine.trim();
  discardHeaders(client);

  if (requestLine.startsWith("GET /stream ")) {
    startStream(client);
    return;
  }

  if (requestLine.startsWith("GET /capture.jpg ")) {
    sendCapture(client);
    return;
  }

  if (requestLine.startsWith("GET / ")) {
    sendRoot(client);
    return;
  }

  sendText(client, "404 Not Found", "Endpoint nao encontrado.\n");
}

void acceptClient() {
  WiFiClient client = gServer.available();
  if (!client) {
    return;
  }

  handleClient(client);
}

void sendStreamFrame() {
  if (!gStreamClient || !gStreamClient.connected()) {
    stopStreamClient();
    return;
  }

  uint32_t now = millis();
  if (now - gLastFrameMs < app::kStreamFrameIntervalMs) {
    return;
  }
  gLastFrameMs = now;

  camera_fb_t *fb = camera_service::capture();
  if (fb == nullptr) {
    return;
  }

  gStreamClient.println("--frame");
  gStreamClient.println("Content-Type: image/jpeg");
  gStreamClient.printf("Content-Length: %u\r\n\r\n", static_cast<unsigned>(fb->len));
  gStreamClient.write(fb->buf, fb->len);
  gStreamClient.println();
  camera_service::release(fb);
}

}  // namespace

namespace stream_service {

void begin() {
  if (gStarted) {
    return;
  }

  gServer.begin();
  gStarted = true;
}

void loop() {
  if (!gStarted || WiFi.status() != WL_CONNECTED) {
    stopStreamClient();
    return;
  }

  acceptClient();
  if (gEnabled) {
    sendStreamFrame();
  } else {
    stopStreamClient();
  }
}

void setEnabled(bool enabled) {
  if (gEnabled == enabled) {
    return;
  }

  gEnabled = enabled;
  if (!gEnabled) {
    stopStreamClient();
    camera_service::applyLedState(false, 0);
    LOGI("Streaming desligado");
    return;
  }

  begin();
  if (!camera_service::isReady()) {
    camera_service::init();
  }
  camera_service::applyLedState(true, app::kStreamLedIntensity);
  LOGI("Streaming ligado: %s", streamUrl().c_str());
}

bool isEnabled() { return gEnabled; }

String streamUrl() {
  if (WiFi.status() != WL_CONNECTED) {
    return String("WiFi desconectado");
  }

  return String("http://") + WiFi.localIP().toString() + "/stream";
}

}  // namespace stream_service
