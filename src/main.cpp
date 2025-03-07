#include <Arduino.h>
#include "pzem_reader.h"
#include <Wire.h>
#include "Preferences.h"
#include <WiFiManager.h>
#include "globals.h"

#define BUTTON_WIFI 13  // Tombol untuk masuk ke mode WiFi Manager
#define BUTTON_RESET 32  // Tombol reset energi dan biaya
#define LED_RED 12      // LED Merah (untuk koneksi gagal)
#define LED_GREEN 14    // LED Hijau (untuk koneksi berhasil)

WiFiManager wm;
bool wifiManagerActive = false;

// Gunakan buffer untuk WiFiManager
char wbpPriceBuffer[10] = "1500";
char lwbpPriceBuffer[10] = "1000";
char thingsboardTokenBuffer[40] = "";

WiFiManagerParameter customWbpPrice("wbp_price", "Harga WBP", wbpPriceBuffer, 10);
WiFiManagerParameter customLwbpPrice("lwbp_price", "Harga LWBP", lwbpPriceBuffer, 10);
WiFiManagerParameter customThingsboardToken("thingsboard_token", "Token ThingsBoard", thingsboardTokenBuffer, 40);

void configModeCallback(WiFiManager *myWiFiManager);
void saveConfig();
void loadConfig();
void updateStatusWiFi();

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_WIFI, INPUT_PULLUP);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(BUTTON_RESET, INPUT_PULLUP);

    wm.setAPCallback(configModeCallback);
    wm.setBreakAfterConfig(true);
    wm.setConfigPortalTimeout(180);

    wm.addParameter(&customWbpPrice);
    wm.addParameter(&customLwbpPrice);
    wm.addParameter(&customThingsboardToken);

    // Muat konfigurasi WiFi & harga listrik
    loadConfig();

    // Coba hubungkan ke WiFi
    WiFi.begin();
    updateStatusWiFi();

    // Inisialisasi PZEM
    pzemInit();
}

void loop() {
    // Tombol WiFi Manager
    if (digitalRead(BUTTON_WIFI) == LOW) {
        delay(50);  // Debounce
        if (digitalRead(BUTTON_WIFI) == LOW) {
            delay(3000);
            if (digitalRead(BUTTON_WIFI) == LOW) {
                Serial.println("Tombol WiFi ditekan, masuk mode konfigurasi...");
                if (wm.startConfigPortal("ESP_Config", "1Def23G5#")) {
                    Serial.println("WiFi dikonfigurasi, keluar dari mode AP.");
                } else {
                    Serial.println("Timeout, kembali mencoba koneksi terakhir...");
                    WiFi.begin();
                }
                updateStatusWiFi();
            }
        }
    }

    // Baca data dari PZEM
    readPzemData();

    // Tombol Reset Data
    if (digitalRead(BUTTON_RESET) == LOW) {
        delay(50);  // Debounce
        if (digitalRead(BUTTON_RESET) == LOW) {
            delay(3000);
            if (digitalRead(BUTTON_RESET) == LOW) {
                Serial.println("Reset energi dan biaya listrik...");

                preferences.begin("config", false);
                preferences.putFloat("previousEnergy", 0.0);
                preferences.putFloat("biayaWBP", 0.0);
                preferences.putFloat("biayaLWBP", 0.0);
                preferences.putFloat("totalCost", 0.0);
                preferences.end();

                totalEnergy = 0.0;
                biayaWBP = 0.0;
                biayaLWBP = 0.0;
                previousEnergy = 0.0;

                Serial.println("Reset complete!");

                while (digitalRead(BUTTON_RESET) == LOW) {
                    delay(10);
                }
            }
        }
    }

    delay(1000);
}

void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("Masuk mode konfigurasi WiFi!");
  Serial.print("SSID AP: "); Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.print("Password AP: ");
  Serial.println("1Def23G5#"); // Sesuai dengan password yang digunakan saat startConfigPortal
  Serial.print("IP AP ESP32: "); Serial.println(WiFi.softAPIP());

  // Ambil nilai parameter dari WiFiManager
  wbpPrice = String(customWbpPrice.getValue()).toFloat();
  lwbpPrice = String(customLwbpPrice.getValue()).toFloat();
  thingsboardToken = String(customThingsboardToken.getValue());

  Serial.println("Harga WBP: Rp " + String(wbpPrice));
  Serial.println("Harga LWBP: Rp " + String(lwbpPrice));
  Serial.println("Token Thingsboard: " + thingsboardToken);

  // **Simpan ke Preferences agar tidak hilang setelah restart**
  saveConfig();
}


void updateStatusWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_GREEN, HIGH);
            delay(300);
            digitalWrite(LED_GREEN, LOW);
            delay(300);
        }
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, LOW);
        Serial.println("WiFi Terhubung!");
    } else {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, HIGH);
        Serial.println("WiFi Tidak Terhubung!");
    }
}

void saveConfig() {
  preferences.begin("config", false);
  preferences.putFloat("wbp_price", wbpPrice);
  preferences.putFloat("lwbp_price", lwbpPrice);
  preferences.putString("thingsboard_token", thingsboardToken);
  preferences.putFloat("lwbp_price", lwbpPrice);
  preferences.end();
}


void loadConfig() {
  preferences.begin("config", true);  // Mode read-only
  wbpPrice = preferences.getFloat("wbp_price", 1500.0);
  lwbpPrice = preferences.getFloat("lwbp_price", 1000.0);
  thingsboardToken = preferences.getString("thingsboard_token", "");
  preferences.end();

  // **Pastikan nilai ini masuk ke buffer agar tampil di WiFiManager**
  snprintf(wbpPriceBuffer, sizeof(wbpPriceBuffer), "%.0f", wbpPrice);
  snprintf(lwbpPriceBuffer, sizeof(lwbpPriceBuffer), "%.0f", lwbpPrice);
  strncpy(thingsboardTokenBuffer, thingsboardToken.c_str(), sizeof(thingsboardTokenBuffer) - 1);
}

