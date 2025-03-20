#include "sendToThingsBoard.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "globals.h"
#include "pzem_reader.h"
#include <ArduinoJson.h>

// MQTT Client
extern WiFiClient espClient;
extern PubSubClient client;
extern const char *thingsboardToken;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000;

void reconnectWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("🔄 Menghubungkan ke WiFi...");
        WiFi.begin();  
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            Serial.print(".");
            delay(500);
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("✅ WiFi Terhubung!");
        } else {
            Serial.println("❌ Gagal terhubung ke WiFi!");
        }
    }
}

void reconnectMQTT() {
    if (!client.connected()) {
        Serial.print("🔄 Menghubungkan ke MQTT ThingsBoard...");
        if (client.connect("ESP32_Client", thingsboardToken, NULL)) {
            Serial.println("✅ Berhasil terhubung ke ThingsBoard!");
        } else {
            Serial.print("❌ Gagal, kode error: ");
            Serial.println(client.state());
        }
    }
}

void sendToThingsBoard() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ WiFi belum tersambung! Membaca data sensor saja...");
        reconnectWiFi();
        return;
    }

    if (!client.connected()) {
        Serial.println("⚠️ MQTT tidak terhubung! Mencoba reconnect...");
        reconnectMQTT();
    }

    client.loop();

    if (isnan(tegangan1) || isnan(arus1) || isnan(daya1) || isnan(energi1)) {
        Serial.println("⚠️ Data mengandung NaN, tidak dikirim!");
        return;
    }

    if (millis() - lastSendTime < sendInterval) {
        return;
    }
    lastSendTime = millis();

    // **Gunakan ArduinoJson untuk membuat JSON**
    StaticJsonDocument<512> doc;  

    doc["volt1"] = tegangan1;
    doc["volt2"] = tegangan2;
    doc["volt3"] = tegangan3;
    doc["ampere1"] = arus1;
    doc["ampere2"] = arus2;
    doc["ampere3"] = arus3;
    doc["power1"] = daya1;
    doc["power2"] = daya2;
    doc["power3"] = daya3;
    doc["energy1"] = energi1;
    doc["energy2"] = energi2;
    doc["energy3"] = energi3;
    doc["totalBiaya"] = totalBiaya;
    doc["totalEnergy"] = totalEnergy;
    // **Konversi JSON ke String**
    char payload[512];
    serializeJson(doc, payload);

    Serial.print("📤 Payload JSON: ");
    Serial.println(payload);

    // **Kirim data ke ThingsBoard**
    client.publish("v1/devices/me/telemetry", payload);
}
