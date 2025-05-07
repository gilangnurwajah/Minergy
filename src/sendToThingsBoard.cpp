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
    // StaticJsonDocument<512> doc;  
    StaticJsonDocument<1024> doc;

    doc["volt1"] = round(tegangan1 * 100) / 100.0;  // 2 desimal
    doc["volt2"] = round(tegangan2 * 100) / 100.0;  // 2 desimal
    doc["volt3"] = round(tegangan3 * 100) / 100.0;  // 2 desimal
    doc["ampere1"] = round(arus1 * 100) / 100.0;    // 2 desimal
    doc["ampere2"] = round(arus2 * 100) / 100.0;    // 2 desimal
    doc["ampere3"] = round(arus3 * 100) / 100.0;    // 2 desimal
    doc["power1"] = round(daya1 * 100) / 100.0;     // 2 desimal
    doc["power2"] = round(daya2 * 100) / 100.0;     // 2 desimal
    doc["power3"] = round(daya3 * 100) / 100.0;     // 2 desimal
    doc["energy1"] = round(energi1 * 100) / 100.0;  // 2 desimal
    doc["energy2"] = round(energi2 * 100) / 100.0;  // 2 desimal
    doc["energy3"] = round(energi3 * 100) / 100.0;  // 2 desimal
    doc["totalBiaya"] = round(totalBiaya * 100) / 100.0;  // 2 desimal
    doc["totalEnergy"] = round(totalEnergy * 100) / 100.0;  // 2 desimal
    
    // **Konversi JSON ke String**
    // char payload[512];
    char payload[1024];
    serializeJson(doc, payload);

    Serial.print("📤 Payload JSON: ");
    Serial.println(payload);

    // **Kirim data ke ThingsBoard**
    // client.publish("v1/devices/me/telemetry", payload);
    if (client.publish("v1/devices/me/telemetry", payload)) {
        Serial.println("✅ Berhasil mengirim telemetry!");
    } else {
        Serial.println("❌ Gagal mengirim telemetry ke ThingsBoard!");
        Serial.print("MQTT State: ");
        Serial.println(client.state());

    }
    
}
