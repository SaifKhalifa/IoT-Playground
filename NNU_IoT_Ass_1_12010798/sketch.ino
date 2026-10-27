/*
  An-Najah National University
  IoT Course
  Assignment #1 : ESP32 -> ThingSpeak (temperature + humidity)

  Sensor : DHT22
  Data pin: GPIO 15 (Wokwi default).
  Wiring : VCC -> 3V3, GND -> GND, DATA -> GPIO 15

  Field 1 = Temperature (C)
  Field 2 = Humidity (%)
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// ---------- CONSTANTS ----------
const char* WIFI_SSID     = "Wokwi-GUEST";  // Wokwi's built-in open Wi-Fi
const char* WIFI_PASSWORD = "";             // empty for Wokwi-GUEST

// Replace with your own channel Write API key (never commit a real key)
const char* THINGSPEAK_WRITE_API_KEY = "YOUR_WRITE_API_KEY";

const char* FULL_NAME     = "Saif_Khalifa";
const char* UNIVERSITY_ID = "12010798";
// --------------------------------

#define DHT_PIN  15
#define DHTTYPE  DHT22

// ThingSpeak free tier: minimum 15 s between updates (20 s used here)
const unsigned long SEND_INTERVAL_MS = 20000;

DHT dht(DHT_PIN, DHTTYPE);
unsigned long lastSend = 0;

void connectWiFi()
{
  Serial.print("Connecting to Wi-Fi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);  // channel 6 speeds up Wokwi connect

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.print("Connected, IP: ");

  Serial.println(WiFi.localIP());
}

void sendToThingSpeak(float temperature, float humidity)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }

  // University ID is sent in the "status" parameter of the update request
  String url = "http://api.thingspeak.com/update?api_key=";
  url += THINGSPEAK_WRITE_API_KEY;
  url += "&field1=" + String(temperature, 1);
  url += "&field2=" + String(humidity, 1);
  url += "&status=" + String(FULL_NAME) + "_" + String(UNIVERSITY_ID);

  HTTPClient http;
  http.begin(url);
  int code = http.GET();

  if (code > 0)
  {
    String entryId = http.getString();  // "0" means the update was rejected

    Serial.printf("[%s | ID %s] HTTP %d, entry id: %s\n",
                  FULL_NAME, UNIVERSITY_ID, code, entryId.c_str());

    if (entryId == "0")
    {
      Serial.println("ThingSpeak rejected the update (check API key / 15 s limit).");
    }
  }
  else
  {
    Serial.printf("HTTP request failed: %s\n", http.errorToString(code).c_str());
  }

  http.end();
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  Serial.printf("ESP32 -> ThingSpeak | %s | University ID: %s\n", FULL_NAME, UNIVERSITY_ID);
  connectWiFi();
}

void loop()
{
  if (millis() - lastSend >= SEND_INTERVAL_MS || lastSend == 0)
  {
    lastSend = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();  // Celsius

    if (isnan(h) || isnan(t))
    {
      Serial.println("Failed to read from DHT sensor, skipping this cycle.");
      return;
    }

    Serial.printf("Temperature: %.1f C | Humidity: %.1f %%\n", t, h);
    sendToThingSpeak(t, h);
  }
}
