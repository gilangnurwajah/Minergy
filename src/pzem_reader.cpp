#include <ModbusMaster.h>
#include <Wire.h>
#include <RTClib.h>
#include "Preferences.h"
#include "globals.h"


// Definisi alamat untuk setiap fase
#define SET_ADDRESS_1 0x01
#define SET_ADDRESS_2 0x02
#define SET_ADDRESS_3 0x03

// Inisialisasi objek ModbusMaster
ModbusMaster node1, node2, node3;
RTC_DS3231 rtc;
// Preferences preferences;

// Harga WBP & LWBP dari WiFi Manager
// float wbpPrice = 1500.0;
// float lwbpPrice = 1000.0;

// Variabel energi dan biaya
// float totalEnergy = 0.0, previousEnergy = 0.0;
// float biayaWBP = 0.0, biayaLWBP = 0.0;

void pzemInit() { // Fungsi inisialisasi
    // Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    // Inisialisasi PZEM
    node1.begin(SET_ADDRESS_1, Serial2);
    node2.begin(SET_ADDRESS_2, Serial2);
    node3.begin(SET_ADDRESS_3, Serial2);

    // Inisialisasi RTC
    if (!rtc.begin()) {
        Serial.println("RTC tidak terdeteksi!");
    } else if (rtc.lostPower()) {
        Serial.println("RTC kehilangan daya, set waktu default!");
        rtc.adjust(DateTime(2025, 2, 12, 14, 30, 0));
    }

    // Ambil harga listrik dari Preferences (WiFi Manager)
    preferences.begin("config", true);
    wbpPrice = preferences.getFloat("wbp_price", 1500.0);
    lwbpPrice = preferences.getFloat("lwbp_price", 1000.0);
    previousEnergy = preferences.getFloat("previousEnergy", 0.0);
    biayaWBP = preferences.getFloat("biayaWBP", 0.0);
    biayaLWBP = preferences.getFloat("biayaLWBP", 0.0);
    preferences.end();

    Serial.printf("Harga WBP: Rp %.2f\n", wbpPrice);
    Serial.printf("Harga LWBP: Rp %.2f\n", lwbpPrice);
}

void readPzemData() { // Fungsi membaca data
    static unsigned long lastReadTime = 0; // Timer untuk membaca data
    if (millis() - lastReadTime < 1000) return; // Baca setiap 1 detik
    lastReadTime = millis();

    float energi1 = 0.0, energi2 = 0.0, energi3 = 0.0; // Variabel energi tiap fase
    uint8_t result;

    // Baca data dari fase 1
    result = node1.readInputRegisters(0x0000, 10);
    if (result == node1.ku8MBSuccess) {
        float tegangan1 = node1.getResponseBuffer(0x00) / 10.0f;
        float arus1 = node1.getResponseBuffer(0x01) / 1000.000f;
        float daya1 = node1.getResponseBuffer(0x03) / 10.0f;
        energi1 = node1.getResponseBuffer(0x05) / 1000000.0f; // Konversi ke kWh
        float frekuensi1 = node1.getResponseBuffer(0x07) / 10.0f;
        float faktorDaya1 = node1.getResponseBuffer(0x08) / 100.0f;
        
        Serial.printf("Fase 1: V=%.2fV, A=%.3fA, W=%.1fW, kWh=%.6f, Hz=%.1f, PF=%.2f\n",
                      tegangan1, arus1, daya1, energi1, frekuensi1, faktorDaya1);
    } else {
        Serial.println("Gagal membaca data fase 1");
    }

    // Baca data dari fase 2
    result = node2.readInputRegisters(0x0000, 10);
    if (result == node2.ku8MBSuccess) {
        float tegangan2 = node2.getResponseBuffer(0x00) / 10.0f;
        float arus2 = node2.getResponseBuffer(0x01) / 1000.000f;
        float daya2 = node2.getResponseBuffer(0x03) / 10.0f;
        energi2 = node2.getResponseBuffer(0x05) / 1000000.0f;
        float frekuensi2 = node2.getResponseBuffer(0x07) / 10.0f;
        float faktorDaya2 = node2.getResponseBuffer(0x08) / 100.0f;

        Serial.printf("Fase 2: V=%.2fV, A=%.3fA, W=%.1fW, kWh=%.6f, Hz=%.1f, PF=%.2f\n",
                      tegangan2, arus2, daya2, energi2, frekuensi2, faktorDaya2);
    } else {
        Serial.println("Gagal membaca data fase 2");
    }

    // Baca data dari fase 3
    result = node3.readInputRegisters(0x0000, 10);
    if (result == node3.ku8MBSuccess) {
        float tegangan3 = node3.getResponseBuffer(0x00) / 10.0f;
        float arus3 = node3.getResponseBuffer(0x01) / 1000.000f;
        float daya3 = node3.getResponseBuffer(0x03) / 10.0f;
        energi3 = node3.getResponseBuffer(0x05) / 1000000.0f;
        float frekuensi3 = node3.getResponseBuffer(0x07) / 10.0f;
        float faktorDaya3 = node3.getResponseBuffer(0x08) / 100.0f;

        Serial.printf("Fase 3: V=%.2fV, A=%.3fA, W=%.1fW, kWh=%.6f, Hz=%.1f, PF=%.2f\n",
                      tegangan3, arus3, daya3, energi3, frekuensi3, faktorDaya3);
    } else {
        Serial.println("Gagal membaca data fase 3");
    }

    // **Menjumlahkan total energi hanya dari fase yang terbaca**
    totalEnergy = energi1 + energi2 + energi3;
    Serial.printf("Total Energy: %.6f kWh\n", totalEnergy);

    // **Perhitungan Biaya Listrik**
    if (!rtc.begin()) {
        Serial.println("RTC Tidak Terbaca! Biaya tidak dihitung.");
        Serial.println("Simulasi Perkalian (tidak dihitung):");
        Serial.printf("Biaya WBP: Rp %.2f\n", totalEnergy * wbpPrice);
        Serial.printf("Biaya LWBP: Rp %.2f\n", totalEnergy * lwbpPrice);
        return;
    }

    DateTime now = rtc.now();
    int jamSekarang = now.hour();

    if (totalEnergy != previousEnergy) {
        if (jamSekarang >= 18 && jamSekarang < 24) {
            biayaWBP += (totalEnergy - previousEnergy) * wbpPrice;
            Serial.printf("Biaya WBP (dihitung): Rp %.2f\n", biayaWBP);
        } else {
            biayaLWBP += (totalEnergy - previousEnergy) * lwbpPrice;
            Serial.printf("Biaya LWBP (dihitung): Rp %.2f\n", biayaLWBP);
        }
        previousEnergy = totalEnergy;

        // Simpan data terbaru ke Preferences
        preferences.begin("config", false);
        preferences.putFloat("previousEnergy", previousEnergy);
        preferences.putFloat("biayaWBP", biayaWBP);
        preferences.putFloat("biayaLWBP", biayaLWBP);
        preferences.putFloat("totalCost", biayaWBP + biayaLWBP);
        preferences.end();

        Serial.printf("Total Biaya: Rp %.2f\n", biayaWBP + biayaLWBP);
        Serial.println("Data terbaru disimpan.");
    }
}

// Fungsi untuk mendapatkan total energi
float getTotalEnergy() {
return totalEnergy;
}