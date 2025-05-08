#include <Arduino.h>
#include "rtc_reader.h"
#include "pzem_reader.h"
#include <Wire.h>
#include <WiFiManager.h>
#include "globals.h"
#include "sendToThingsBoard.h"
#include "storage_manager.h"
#include "nextion.h"
#include "barcode.h"  // Tambahkan ini di bagian atas
#include "tarif.h"

#define BUTTON_WIFI 13  // Tombol untuk masuk ke mode WiFi Manager
#define BUTTON_RESET 32  // Tombol reset energi dan biaya
#define LED_RED 12       // LED Merah (untuk koneksi gagal)
#define LED_GREEN 14     // LED Hijau (untuk koneksi berhasil)
#define NEXTION_RX 26
#define NEXTION_TX 27



WiFiClient espClient;
PubSubClient client(espClient);

Preferences preferences;

bool wifiManagerActive = false;
const char* mqtt_server = "34.232.35.38";  // Pakai IP langsung
// const char* mqtt_server = "thingsboard.cloud";  // Pakai IP langsung
const char *thingsboardToken = "MINERGYYY";  // Ganti dengan token asli
const int THINGSBOARD_PORT = 1883;


void configModeCallback(WiFiManager *myWiFiManager);
void saveConfig();
void updateStatusWiFi();
void blinkLED(int pin, int times, int delayMs);
void handleNextionInput();

HardwareSerial NextionSerial(1);

WiFiManager wm;


void setup() {
    Serial.begin(115200);

    //untuk input
    storageInit(); 
    loadTarifFromStorage();

    rtcInit();
    NextionSerial.begin(9600, SERIAL_8N1, 26, 27);  // RX=GPIO26, TX=GPIO27
    initNextion(NextionSerial);
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
    static unsigned long lastSensorRead = 0;
    static unsigned long lastWiFiCheck = 0;
    static unsigned long lastThingsBoardSend = 0;
    static unsigned long lastNextionUpdate = 0;  // ⬅️ Tambahan ini

    static const unsigned long sensorInterval = 6000;       // 6 detik
    static const unsigned long wifiCheckInterval = 10000;   // 10 detik
    static const unsigned long tbSendInterval = 15000;      // 15 detik
    static const unsigned long nextionUpdateInterval = 1000; // ⏱️ 1 detik

    // ✅ Selalu tangani input dari Nextion secepat mungkin
    handleNextionInput();


    // Update Nextion tiap 1 detik
    if (millis() - lastNextionUpdate > nextionUpdateInterval) {
        if (currentPage == 1 || currentPage == 2) {
            updateNextion();
        }
        lastNextionUpdate = millis();
    }
    
    // Periksa koneksi WiFi berkala
    if (millis() - lastWiFiCheck > wifiCheckInterval) {
        updateStatusWiFi();
        lastWiFiCheck = millis();
    }

    // ✅ Baca sensor & update Nextion berkala
    if (millis() - lastSensorRead > sensorInterval) {
        printDateTime();
        readPzemData();
        hitungBiaya();
        // updateNextion();
        saveTotalEnergyPrev(totalEnergyPrev);
        saveEnergyData(energyWbp, energyLwbp, biayaWbp, biayaLwbp);
        lastSensorRead = millis();
    }

    // ✅ Kirim ke ThingsBoard jika WiFi tersedia
    if (WiFi.status() == WL_CONNECTED && millis() - lastThingsBoardSend > tbSendInterval) {
        if (!client.connected()) {
            Serial.println("⚠️ MQTT terputus! Mencoba reconnect...");
            reconnectMQTT();
        }
        sendToThingsBoard();
        client.loop();
        lastThingsBoardSend = millis();
    } else {
        client.loop(); // Tetap jalankan loop MQTT meski tidak kirim
    }

    // ✅ Tangani tombol RESET ENERGI
    static unsigned long lastButtonPress = 0;
    if (digitalRead(BUTTON_RESET) == LOW && millis() - lastButtonPress > 1000) {
        lastButtonPress = millis();
        if (digitalRead(BUTTON_RESET) == LOW) {
            unsigned long pressStart = millis();
            while (digitalRead(BUTTON_RESET) == LOW) {
                if (millis() - pressStart > 3000) {
                    resetEnergy();
                    energyWbp = 0.0;
                    energyLwbp = 0.0;
                    saveEnergyData(energyWbp, energyLwbp, biayaWbp, biayaLwbp);
                    Serial.println("⚡ Energi direset dari tombol fisik!");
                    delay(500);  // optional: kasih delay pendek agar tidak langsung trigger ulang
                    readPzemData();
                    break;
                }
            }
        }
    }

    // ✅ Tangani tombol WiFi Manager
    if (digitalRead(BUTTON_WIFI) == LOW && millis() - lastButtonPress > 1000) {
        lastButtonPress = millis();
        if (digitalRead(BUTTON_WIFI) == LOW) {
            unsigned long pressStart = millis();
            while (digitalRead(BUTTON_WIFI) == LOW) {
                if (millis() - pressStart > 3000) {
                    Serial.println("🔵 Masuk Mode WiFi Manager...");
                    if (wm.startConfigPortal("ESP_Config", "1Def23G5")) {
                        Serial.println("✅ WiFi dikonfigurasi.");
                    } else {
                        Serial.println("⚠️ Timeout WiFi config, reconnect terakhir...");
                        WiFi.reconnect();
                    }
                    updateStatusWiFi();
                    break;
                }
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
        delay(7000);
    }
}

    void configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("Masuk mode konfigurasi WiFi!");
    Serial.print("SSID AP: "); Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.print("Password AP: ");
    Serial.println("1Def23G5");
    Serial.print("IP AP ESP32: "); Serial.println(WiFi.softAPIP());

    // Tampilkan halaman ModeAp di Nextion
    sendCommand("page ModeAp");
    delay(500);

    // Tampilkan informasi SSID, password dan IP
    sendCommand("t0.txt=\"" + String(myWiFiManager->getConfigPortalSSID()) + "\"");
    sendCommand("t1.txt=\"1Def23G5\"");
    sendCommand("t2.txt=\"" + WiFi.softAPIP().toString() + "\"");

    // Generate QR code dengan SSID, password, dan IP
    // ✅ Buat QR code untuk koneksi WiFi
    String ip = WiFi.softAPIP().toString();
    generateQRCode("ESP_Config", "1Def23G5", ip);
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