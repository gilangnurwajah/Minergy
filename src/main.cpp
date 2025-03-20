#include <Arduino.h>
#include "rtc_reader.h"
#include "pzem_reader.h"
#include <Wire.h>
#include <WiFiManager.h>
#include "globals.h"
#include "sendToThingsBoard.h"
#include "storage_manager.h"

#define BUTTON_WIFI 13  // Tombol untuk masuk ke mode WiFi Manager
#define BUTTON_RESET 32  // Tombol reset energi dan biaya
#define LED_RED 12       // LED Merah (untuk koneksi gagal)
#define LED_GREEN 14     // LED Hijau (untuk koneksi berhasil)

WiFiManager wm;
WiFiClient espClient;
PubSubClient client(espClient);

Preferences preferences;

bool wifiManagerActive = false;
const char* mqtt_server = "34.232.35.38";  // Pakai IP langsung
const char *thingsboardToken = "MINERGIoT";  // Ganti dengan token asli
const int THINGSBOARD_PORT = 1883;


void configModeCallback(WiFiManager *myWiFiManager);
void saveConfig();
void updateStatusWiFi();
void blinkLED(int pin, int times, int delayMs);

void setup() {
    Serial.begin(115200);
    rtcInit();
    client.setServer(mqtt_server, THINGSBOARD_PORT);

    pinMode(BUTTON_WIFI, INPUT_PULLUP);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(BUTTON_RESET, INPUT_PULLUP);

    digitalWrite(LED_GREEN, LOW);

    wm.setAPCallback(configModeCallback);
    wm.setBreakAfterConfig(true);
    wm.setConfigPortalTimeout(180);

    WiFi.begin();
    updateStatusWiFi();
    pzemInit();

    // Inisialisasi penyimpanan data
    storageInit();

    // Load data dari penyimpanan
    loadEnergyData(energyWbp, energyLwbp, biayaWbp, biayaLwbp);
    Serial.printf("✅ Data awal: WBP=%.3f kWh, LWBP=%.3f kWh\n", energyWbp, energyLwbp);

    totalEnergyPrev = loadTotalEnergyPrev();
    Serial.printf("🔄 Memuat totalEnergyPrev: %.3f kWh\n", totalEnergyPrev);

}

void loop() {
    updateStatusWiFi();
    client.loop();
    delay(5000);

    printDateTime();
    delay(1000);
    readPzemData();
    hitungBiaya();
    saveTotalEnergyPrev(totalEnergyPrev);


    // Simpan data ke Preferences setiap siklus
    saveEnergyData(energyWbp, energyLwbp, biayaWbp, biayaLwbp);

    static unsigned long lastButtonPress = 0; // Untuk debounce tombol

        // **Tombol Reset Energi**
    if (digitalRead(BUTTON_RESET) == LOW && millis() - lastButtonPress > 1000) {
        lastButtonPress = millis();
        delay(50); // Debounce
        if (digitalRead(BUTTON_RESET) == LOW) {
            delay(3000); // Tahan 3 detik untuk reset
            if (digitalRead(BUTTON_RESET) == LOW) {
                resetEnergy();
                Serial.println("⚡ Total energi direset ke 0 kWh!");

                // **Set nilai energi ke nol**
                energyWbp = 0.0;
                energyLwbp = 0.0;

                // **Simpan kembali ke Preferences**
                saveEnergyData(energyWbp, energyLwbp, biayaWbp, biayaLwbp);

                Serial.println("🔄 Membaca ulang data PZEM setelah reset...");
                delay(2000);  // Tunggu sebelum membaca ulang
                readPzemData();  // Baca ulang data setelah reset            
            }
        }
    }


    // **Tombol WiFi Manager**
    if (digitalRead(BUTTON_WIFI) == LOW && millis() - lastButtonPress > 1000) {
        lastButtonPress = millis();  
        delay(50);  // Debounce
        if (digitalRead(BUTTON_WIFI) == LOW) {
            delay(3000);
            if (digitalRead(BUTTON_WIFI) == LOW) {
                Serial.println("🔵 Masuk Mode WiFi Manager...");
                if (wm.startConfigPortal("ESP_Config", "1Def23G5#")) {
                    Serial.println("✅ WiFi dikonfigurasi, keluar dari mode AP.");
                } else {
                    Serial.println("⚠️ Timeout, mencoba ulang koneksi terakhir...");
                    WiFi.reconnect(); // **Lebih baik dari WiFi.begin()**
                }
                updateStatusWiFi();
            }
        }
    }

    // Cek WiFi Sebelum Kirim Data
    if (WiFi.status() == WL_CONNECTED) {
        if (!client.connected()) {
            Serial.println("⚠️ MQTT terputus! Mencoba reconnect...");
            reconnectMQTT();
        }
        sendToThingsBoard();
        client.loop();
    } else {
        Serial.println("⚠️ WiFi tidak terhubung, hanya membaca data sensor...");
    }
}

    void configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("Masuk mode konfigurasi WiFi!");
    Serial.print("SSID AP: "); Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.print("Password AP: ");
    Serial.println("1Def23G5#");
    Serial.print("IP AP ESP32: "); Serial.println(WiFi.softAPIP());

    // Ambil nilai parameter dari WiFiManager
    Serial.println("Token ThingsBoard: ");
}


void updateStatusWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        blinkLED(LED_GREEN, 3, 300);
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, LOW);
        Serial.println("✅ WiFi Terhubung!");
    } else {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, HIGH);
        Serial.println("❌ WiFi Tidak Terhubung!");
    }
}

// Fungsi untuk kedip LED
void blinkLED(int pin, int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        digitalWrite(pin, HIGH);
        delay(delayMs);
        digitalWrite(pin, LOW);
        delay(delayMs);
    }
}