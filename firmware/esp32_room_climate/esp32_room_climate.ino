#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include "secrets.h"

const int DHT_PIN = 15;
DHT dht(DHT_PIN, DHT11);

WebServer server(80);

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

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    Serial.print("FAILED. Status code: ");
    Serial.println(WiFi.status());
    return;
  }

  Serial.println();
  Serial.print("Ready at http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {
  server.handleClient();
}