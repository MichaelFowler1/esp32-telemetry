#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include "secrets.h"

const int DHT_PIN = 15;
DHT dht(DHT_PIN, DHT11);

WebServer server(80);

const unsigned long WIFI_CHECK_INTERVAL_MS = 10000;
unsigned long lastWifiCheckMs = 0;
bool wifiWasConnected = false;

// Called every WIFI_CHECK_INTERVAL_MS from loop(). Never blocks: if the
// connection is down it (re)starts a connection attempt and returns; a later
// check observes whether it succeeded.
void ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiWasConnected) {
      wifiWasConnected = true;
      Serial.print("WiFi connected. Ready at http://");
      Serial.println(WiFi.localIP());
    }
    return;
  }

  if (wifiWasConnected) {
    wifiWasConnected = false;
    Serial.println("WiFi lost. Reconnecting...");
  }
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void handleRoot() {
  server.send(200, "text/plain", "ESP32 sensor node. Try /data");
}

void handleData() {
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();

  if (isnan(humidity) || isnan(tempC)) {
    server.send(503, "application/json", "{\"error\":\"sensor read failed\"}");
    return;
  }

  String json = "{";
  json += "\"temperature_c\":" + String(tempC, 1) + ",";
  json += "\"humidity_pct\":" + String(humidity, 1) + ",";
  json += "\"uptime_ms\":" + String(millis());
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  dht.begin();

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected yet; the watchdog in loop() keeps trying.");
  }

  // Start the server unconditionally: it just listens on whatever network
  // stack exists, so it must not be held hostage by boot-time WiFi luck.
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  ensureWifi();  // logs the IP if connected, starts a retry if not
}

void loop() {
  server.handleClient();

  if (millis() - lastWifiCheckMs >= WIFI_CHECK_INTERVAL_MS) {
    lastWifiCheckMs = millis();
    ensureWifi();
  }
}