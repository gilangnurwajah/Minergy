#include <ModbusMaster.h>
#include <Wire.h>
#include "Preferences.h"
#include "globals.h"
#include "rtc_reader.h"

// Definisi alamat untuk setiap fase
#define SET_ADDRESS_1 0x01
#define SET_ADDRESS_2 0x02
#define SET_ADDRESS_3 0x03

// Inisialisasi objek ModbusMaster
ModbusMaster node1, node2, node3;
// float biayaWbp = 0.0;
// float biayaLwbp = 0.0;

void pzemInit() { // Fungsi inisialisasi
    // Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    // Inisialisasi PZEM
    node1.begin(SET_ADDRESS_1, Serial2);
    node2.begin(SET_ADDRESS_2, Serial2);
    node3.begin(SET_ADDRESS_3, Serial2);
}

float totalBiaya = 0.0;
// float totalEnergyPrev = 0.0;  // Menyimpan total energi sebelumnya
// float energyWbp = 0.0;        // Energi yang dikonsumsi saat jam WBP
// float energyLwbp = 0.0;       // Energi yang dikonsumsi saat jam LWBP

void readPzemData() { // Fungsi membaca data
    static unsigned long lastReadTime = 0; // Timer untuk membaca data
    if (millis() - lastReadTime < 1000) return; // Baca setiap 1 detik
    lastReadTime = millis();

    energi1 = 0.0, energi2 = 0.0, energi3 = 0.0; // Variabel energi tiap fase
    uint8_t result;

    // Baca data dari fase 1
    result = node1.readInputRegisters(0x0000, 10);
    if (result == node1.ku8MBSuccess) {
        tegangan1 = node1.getResponseBuffer(0x00) / 10.0f;
        arus1 = node1.getResponseBuffer(0x01) / 1000.0f;
        daya1 = node1.getResponseBuffer(0x03) / 10.0f;
        energi1 = node1.getResponseBuffer(0x05) / 1000.0f;
        frekuensi1 = node1.getResponseBuffer(0x07) / 10.0f;
        faktorDaya1 = node1.getResponseBuffer(0x08) / 100.0f;    
        
        Serial.printf("Fase 1: V=%.2fV, A=%.2fA, W=%.1fW, kWh=%.3f, Hz=%.1f, PF=%.2f\n",
            tegangan1, arus1, daya1, energi1, frekuensi1, faktorDaya1);

    } else {
        Serial.println("Gagal membaca data fase 1");
        tegangan1 = arus1 = daya1 = energi1 = frekuensi1 = faktorDaya1 = 0.0;
    }

    // Baca data dari fase 2
    result = node2.readInputRegisters(0x0000, 10);
    if (result == node2.ku8MBSuccess) {
        tegangan2 = node2.getResponseBuffer(0x00) / 10.0f;
        arus2 = node2.getResponseBuffer(0x01) / 1000.0f;
        daya2 = node2.getResponseBuffer(0x03) / 10.0f;
        energi2 = node2.getResponseBuffer(0x05) / 1000.0f;  // Diperbaiki
        frekuensi2 = node2.getResponseBuffer(0x07) / 10.0f;
        faktorDaya2 = node2.getResponseBuffer(0x08) / 100.0f;

        Serial.printf("Fase 2: V=%.2fV, A=%.2fA, W=%.1fW, kWh=%.3f, Hz=%.1f, PF=%.2f\n",
                      tegangan2, arus2, daya2, energi2, frekuensi2, faktorDaya2);
    } else {
        Serial.println("Gagal membaca data fase 2");
        tegangan2 = arus2 = daya2 = energi2 = frekuensi2 = faktorDaya2 = 0.0;
    }

    // Baca data dari fase 3
    result = node3.readInputRegisters(0x0000, 10);
    if (result == node3.ku8MBSuccess) {
        tegangan3 = node3.getResponseBuffer(0x00) / 10.0f;
        arus3 = node3.getResponseBuffer(0x01) / 1000.0f;
        daya3 = node3.getResponseBuffer(0x03) / 10.0f;
        energi3 = node3.getResponseBuffer(0x05) / 1000.0f;  // Diperbaiki
        frekuensi3 = node3.getResponseBuffer(0x07) / 10.0f;
        faktorDaya3 = node3.getResponseBuffer(0x08) / 100.0f;

        Serial.printf("Fase 3: V=%.2fV, A=%.2fA, W=%.1fW, kWh=%.3f, Hz=%.1f, PF=%.2f\n",
                      tegangan3, arus3, daya3, energi3, frekuensi3, faktorDaya3);
    } else {
        Serial.println("Gagal membaca data fase 3");
        tegangan3 = arus3 = daya3 = energi3 = frekuensi3 = faktorDaya3 = 0.0;
    }

    // **Menjumlahkan total energi hanya dari fase yang terbaca**
    totalEnergy = energi1 + energi2 + energi3;
    totalBiaya = totalEnergy * hargaListrik;
    Serial.printf("Total Biaya: Rp %.2f\n", totalBiaya);
    Serial.printf("Total Energy: %.3f kWh\n", totalEnergy);
    }

    // Fungsi untuk mendapatkan total energi
    float getTotalEnergy() {
    return totalEnergy;
    }

    // if (now.hour() >= 18 && now.hour() < 23) {
    void hitungBiaya() {
        DateTime now = getRTCNow();
    
        // Hitung selisih energi sejak pembacaan terakhir
        float deltaEnergy = totalEnergy - totalEnergyPrev;
    
        // Jika ada tambahan energi sejak pembacaan terakhir
        if (deltaEnergy > 0) {
            bool isWBP = false;
            // if (now.hour() >= 18 && now.hour() < 24) {
            if ((now.hour() == 18 && now.minute() >= 30) ||  // 18:30 - 18:59
                (now.hour() >= 19 && now.hour() < 24) ||     // 19:00 - 23:59
                (now.hour() == 0 && now.minute() <= 28)) {   // 00:00 - 00:15
                isWBP = true;
            }
            if (isWBP) {
                energyWbp += deltaEnergy;  // Tambahkan energi ke kategori WBP
                Serial.printf("Waktu Wbp");
            } else {
                energyLwbp += deltaEnergy;  // Tambahkan energi ke kategori LWBP
                Serial.printf("Waktu Lwbp");
            }
        }
    
        // Hitung biaya berdasarkan energi yang digunakan dalam masing-masing periode
        biayaWbp = energyWbp * hargaWbp;
        biayaLwbp = energyLwbp * hargaLwbp;
        
        BiayaTWP = biayaWbp + biayaLwbp;
        TotalPower = daya1 + daya2 + daya3;
        // Simpan total energi saat ini untuk pembacaan berikutnya
        totalEnergyPrev = totalEnergy;
    
        // Cetak hasil
        Serial.printf(" Biaya WBP: Rp %.2f | Biaya LWBP: Rp %.2f\n", biayaWbp, biayaLwbp);
        Serial.printf(" Energi WBP: %.3f kWh | Energi LWBP: %.3f kWh\n", energyWbp, energyLwbp);
    }

    uint16_t calculateCRC(uint8_t *buffer, uint8_t length) {
        uint16_t crc = 0xFFFF;
        for (uint8_t i = 0; i < length; i++) {
            crc ^= buffer[i];
            for (uint8_t j = 0; j < 8; j++) {
                if (crc & 0x0001) {
                    crc >>= 1;
                    crc ^= 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }    

    void resetEnergy() {
        Serial.println("🔴 Mengirim perintah reset energi ke semua PZEM...");
    
        uint8_t resetCommand[] = {0x01, 0x42, 0x00, 0x00}; // Contoh untuk PZEM alamat 0x01
        uint16_t crc = calculateCRC(resetCommand, 2); // Hitung CRC untuk 2 byte pertama
        
        resetCommand[2] = crc & 0xFF;  // CRC Low Byte
        resetCommand[3] = (crc >> 8) & 0xFF;  // CRC High Byte
    
        Serial2.write(resetCommand, sizeof(resetCommand));
        delay(100);
    
        resetCommand[0] = 0x02; // Ganti alamat untuk PZEM kedua
        crc = calculateCRC(resetCommand, 2);
        resetCommand[2] = crc & 0xFF;
        resetCommand[3] = (crc >> 8) & 0xFF;
        
        Serial2.write(resetCommand, sizeof(resetCommand));
        delay(100);
    
        resetCommand[0] = 0x03; // Ganti alamat untuk PZEM ketiga
        crc = calculateCRC(resetCommand, 2);
        resetCommand[2] = crc & 0xFF;
        resetCommand[3] = (crc >> 8) & 0xFF;
        
        Serial2.write(resetCommand, sizeof(resetCommand));
        delay(100);
    
        Serial.println("✅ Perintah reset energi dikirim ke semua PZEM!");
        
    }

    
    