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

// Variabel untuk non-blocking WiFi reconnect
static bool wifiTrying = false;
static unsigned long wifiStartAttemptTime = 0;
const unsigned long wifiTimeout = 10000;

bool reconnectWiFiNonBlocking() {
    if (WiFi.status() == WL_CONNECTED) {
        wifiTrying = false;
        return true;
    }

    if (!wifiTrying) {
        Serial.println(" Memulai koneksi WiFi...");
        WiFi.begin();
        wifiStartAttemptTime = millis();
        wifiTrying = true;
    }

    if (millis() - wifiStartAttemptTime > wifiTimeout) {
        Serial.println("❌ Timeout koneksi WiFi.");
        wifiTrying = false;
        return false;
    }

    return false;  // Masih dalam proses koneksi
}

void reconnectMQTT() {
    if (!client.connected()) {
        Serial.print("🔄 Menghubungkan ke MQTT ThingsBoard...");
        if (client.connect("ESP32_Client", thingsboardToken, NULL)) {
            Serial.println("✅ Berhasil terhubung ke ThingsBoard!");
        } else {
            Serial.print(" Gagal, kode error: ");
            Serial.println(client.state());
        }
    }
}

void sendToThingsBoard() {
    // Cek dan coba koneksi WiFi secara non-blocking
    if (!reconnectWiFiNonBlocking()) {
        Serial.println(" WiFi belum tersambung! Menunggu koneksi...");
        return;
    }

    // Cek koneksi MQTT
    if (!client.connected()) {
        Serial.println(" MQTT tidak terhubung! Mencoba reconnect...");
        reconnectMQTT();
    }

    client.loop();

    // Cek validitas data sensor
    if (isnan(tegangan1) || isnan(arus1) || isnan(daya1) || isnan(energi1)) {
        Serial.println(" Data mengandung NaN, tidak dikirim!");
        return;
    }

    // Cek interval pengiriman data
    if (millis() - lastSendTime < sendInterval) {
        return;
    }
    lastSendTime = millis();

    // Buat JSON payload
    StaticJsonDocument<1024> doc;
    doc["R"] = round(tegangan1 * 100) / 100.0;
    doc["S"] = round(tegangan2 * 100) / 100.0;
    doc["T"] = round(tegangan3 * 100) / 100.0;
    doc["1"] = round(arus1 * 100) / 100.0;
    doc["2"] = round(arus2 * 100) / 100.0;
    doc["3"] = round(arus3 * 100) / 100.0;
    doc["4"] = round(daya1 * 100) / 100.0;
    doc["5"] = round(daya2 * 100) / 100.0;
    doc["6"] = round(daya3 * 100) / 100.0;
    doc["7"] = round(energi1 * 100) / 100.0;
    doc["8"] = round(energi2 * 100) / 100.0;
    doc["9"] = round(energi3 * 100) / 100.0;
    doc["Reg"] = round(totalBiaya * 100) / 100.0;
    doc["Te"] = round(totalEnergy * 100) / 100.0;
    doc["Twp"] = round(BiayaTWP * 100) / 100.0;
    doc["Tp"] = round(TotalPower * 100) / 100.0;

    size_t jsonSize = measureJson(doc);
    Serial.print("JSON size: ");
    Serial.println(jsonSize);
    
    char payload[1024];
    serializeJson(doc, payload);

    Serial.print(" Payload JSON: ");
    Serial.println(payload);

    if (client.publish("v1/devices/me/telemetry", payload)) {
        Serial.println("✅ Berhasil mengirim telemetry!");
    } else {
        Serial.println(" Gagal mengirim telemetry ke ThingsBoard!");
        Serial.print("MQTT State: ");
        Serial.println(client.state());
    }
}
