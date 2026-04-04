#include "network/webhook_client.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "debug/logger.h"
#include "system/app_config.h"

namespace {

struct ParsedUrl {
  String host;
  String path;
  uint16_t port;
};

bool parseUrl(const String &url, ParsedUrl &parsed) {
  int scheme = url.indexOf("://");
  if (scheme < 0) {
    return false;
  }

  String protocol = url.substring(0, scheme);
  if (protocol == "https") {
    parsed.port = 443;
  } else if (protocol == "http") {
    parsed.port = 80;
  } else {
    return false;
  }

  int hostStart = scheme + 3;
  int pathStart = url.indexOf('/', hostStart);
  String hostPort = (pathStart >= 0) ? url.substring(hostStart, pathStart) : url.substring(hostStart);
  parsed.path = (pathStart >= 0) ? url.substring(pathStart) : "/";

  int portSep = hostPort.indexOf(':');
  if (portSep >= 0) {
    parsed.host = hostPort.substring(0, portSep);
    parsed.port = static_cast<uint16_t>(hostPort.substring(portSep + 1).toInt());
  } else {
    parsed.host = hostPort;
  }

  return parsed.host.length() > 0;
}

bool waitResponse(WiFiClientSecure &client) {
  uint32_t start = millis();
  while (client.connected() && !client.available()) {
    if (millis() - start > app::kHttpResponseTimeoutMs) {
      return false;
    }
    delay(10);
  }
  return true;
}

}  // namespace

namespace webhook_client {

app::Status sendJsonFallback(const String &url, const RollMetrics &m) {
  WiFiClientSecure client;
  HTTPClient http;

  client.setInsecure();
  if (!http.begin(client, url)) {
    return app::Status::kNetworkError;
  }

  char body[220];
  snprintf(body, sizeof(body),
           "{\"pulses\":%lu,\"rollTime\":%lu,\"entropy\":%lu,\"dice\":%u,\"source\":\"%s\"}",
           static_cast<unsigned long>(m.pulses),
           static_cast<unsigned long>(m.rollTimeUs),
           static_cast<unsigned long>(m.entropy), static_cast<unsigned>(m.dice), app::kSourceTag);

  http.addHeader("Content-Type", "application/json");
  int code = http.POST(String(body));
  http.end();
  LOGI("Fallback JSON HTTP: %d", code);

  return (code >= 200 && code < 300) ? app::Status::kOk : app::Status::kSendError;
}

app::Status sendPhotoMultipart(const String &url, camera_fb_t *fb, const RollMetrics &m) {
  if (fb == nullptr || fb->len == 0) {
    return app::Status::kInvalidArgument;
  }

  ParsedUrl parsed;
  if (!parseUrl(url, parsed)) {
    return app::Status::kParseError;
  }

  WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect(parsed.host.c_str(), parsed.port)) {
    return app::Status::kNetworkError;
  }

  const char *boundary = "----ESP32CamBoundary";
  String head;
  head.reserve(380);
  head += String("--") + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"pulses\"\r\n\r\n" + String(m.pulses) + "\r\n";
  head += String("--") + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"rollTime\"\r\n\r\n" + String(m.rollTimeUs) + "\r\n";
  head += String("--") + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"entropy\"\r\n\r\n" + String(m.entropy) + "\r\n";
  head += String("--") + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"dice\"\r\n\r\n" + String(m.dice) + "\r\n";
  head += String("--") + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"source\"\r\n\r\n" + String(app::kSourceTag) + "\r\n";
  head += String("--") + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"photo\"; filename=\"capture.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  String tail = String("\r\n--") + boundary + "--\r\n";
  size_t totalLen = head.length() + fb->len + tail.length();

  client.printf("POST %s HTTP/1.1\r\n", parsed.path.c_str());
  client.printf("Host: %s\r\n", parsed.host.c_str());
  client.println("User-Agent: ESP32-CAM-SW420/2.0");
  client.printf("Content-Type: multipart/form-data; boundary=%s\r\n", boundary);
  client.printf("Content-Length: %u\r\n", static_cast<unsigned>(totalLen));
  client.println("Connection: close\r\n");

  client.print(head);

  size_t sent = 0;
  while (sent < fb->len) {
    size_t block = min(app::kUploadChunkSize, fb->len - sent);
    size_t wrote = client.write(fb->buf + sent, block);
    if (wrote == 0) {
      if (!client.connected()) {
        return app::Status::kNetworkError;
      }
      delay(2);
      continue;
    }
    sent += wrote;
  }

  client.print(tail);
  if (!waitResponse(client)) {
    client.stop();
    return app::Status::kTimeout;
  }

  String statusLine = client.readStringUntil('\n');
  statusLine.trim();
  client.stop();
  LOGI("Webhook: %s", statusLine.c_str());

  return statusLine.indexOf(" 2") > 0 ? app::Status::kOk : app::Status::kSendError;
}

}  // namespace webhook_client
