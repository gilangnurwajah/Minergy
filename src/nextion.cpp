#include "nextion.h"
#include "globals.h"
#include "pzem_reader.h"
#include "RTC_reader.h"
#include "storage_manager.h"   // untuk saveEnergyData()
#include <WiFi.h>
#include "barcode.h"  // Pastikan ditambahkan di nextion.cpp
#include <WiFiManager.h>
#include "tarif.h"

//untuk input
String tarifInputBuffer = "";


extern WiFiManager wm;
extern void updateStatusWiFi();

HardwareSerial* nextionSerial;
int currentPage = 1;  // Default ke halaman Monitor1 saat awal

void initNextion(HardwareSerial &serial) {
    nextionSerial = &serial;
    nextionSerial->begin(9600);
}

void sendCommand(const String& cmd) {
    nextionSerial->print(cmd);
    nextionSerial->write(0xFF);
    nextionSerial->write(0xFF);
    nextionSerial->write(0xFF);
}

// Fungsi getCommand untuk mengambil data dari Nextion
String getCommand(const String& command) {
    String response = "";
    sendCommand(command);  // Mengirim perintah ke Nextion
    
    // Menunggu respon dari Nextion
    delay(100);  // Pastikan delay cukup untuk Nextion mengirimkan respon
    
    while (nextionSerial->available()) {
        char c = nextionSerial->read();  // Membaca karakter dari serial
        if (c != 0xFF) {  // Hanya ambil data yang valid
            response += c;
        }
    }
    return response;  // Mengembalikan nilai yang diterima
}

void handleNextionInput() {
    while (nextionSerial->available()) {
        String input = "";
        int ffCount = 0;

        while (nextionSerial->available()) {
            char c = nextionSerial->read();

            if (c == 0xFF) {
                ffCount++;
                if (ffCount >= 3) break;
            } else {
                input += c;
                ffCount = 0;
            }
        }

        // Deteksi input tarif dari Nextion
        if (input.startsWith("REG:")) {
            Serial.println("📜 Input tarif REG diterima: " + input); 
            prosesInputTarif(input);
        }

        // Deteksi halaman aktif
        if (input.indexOf("Monitor1") >= 0) {
            currentPage = 1;
            Serial.println("📄 Halaman aktif: Monitor1");
            delay(300);
            updateMonitor1();
        } else if (input.indexOf("Monitor2") >= 0) {
            currentPage = 2;
            Serial.println("📄 Halaman aktif: Monitor2");
            delay(300);
            updateMonitor2();  
        } else if (input.indexOf("Reset") >= 0) {
            currentPage = 3;
            Serial.println("📄 Halaman aktif: Reset");
        } else if (input.indexOf("TarifReguler") >= 0) {
            currentPage = 5;
            Serial.println("📄 Halaman aktif: TarifReguler");
        } else if (input.indexOf("TWP") >= 0) {
            currentPage = 6;
            Serial.println("📄 Halaman aktif: TWP");

            sendCommand("TWP.t0.txt=\"" + String(hargaWbp, 2) + "\"");
            sendCommand("TWP.t1.txt=\"" + String(hargaLwbp, 2) + "\"");
        } else if (input.indexOf("Configuration") >= 0) {
            currentPage = 3;
            Serial.println("📄 Halaman aktif: Configuration");
        } else if (input.indexOf("ModeAp") >= 0) {
            currentPage = 4;
            Serial.println("📄 Halaman aktif: ModeAp");

            String ssid = WiFi.SSID();
            String password = "1Def23G5";
            IPAddress ip = WiFi.softAPIP();
            String ipStr = ip.toString();

            delay(300);
            sendCommand("t0.txt=\"" + ssid + "\"");
            sendCommand("t1.txt=\"" + password + "\"");
            sendCommand("t2.txt=\"" + ipStr + "\"");

            generateQRCode(ssid, password, ipStr);
        }

        // Perintah WiFi AP Mode
        if (input.indexOf("APMODE") >= 0) {
            Serial.println("🌐 Perintah APMODE diterima dari Nextion");

            WiFi.disconnect(true);   // Tambahan agar AP Mode bisa dimulai
            delay(1000);

            if (wm.startConfigPortal("ESP_Config", "1Def23G5")) {
                Serial.println("✅ WiFi dikonfigurasi");

                String ssid = WiFi.SSID();
                IPAddress ip = WiFi.softAPIP();
                String ipStr = ip.toString();

                sendCommand("page ModeAp");
                delay(500);
                sendCommand("t0.txt=\"" + ssid + "\"");
                sendCommand("t1.txt=\"1Def23G5\"");
                sendCommand("t2.txt=\"" + ipStr + "\"");
            } else {
                Serial.println("⚠️ Timeout, mencoba ulang koneksi...");
                WiFi.reconnect();
            }

            updateStatusWiFi();
            sendCommand("Configuration.t0.txt=\"AP Mode Aktif\"");
        }

        // Tombol reset energi
        if (input.indexOf("RST") >= 0) {
            Serial.println("🧹 Reset energi dari Nextion ditekan!");
            resetEnergy();
            energyWbp = 0.0;
            energyLwbp = 0.0;
            biayaWbp = 0.0;
            biayaLwbp = 0.0;
            totalEnergy = 0.0;
            totalBiaya = 0.0;
            saveEnergyData(energyWbp, energyLwbp, biayaWbp, biayaLwbp);
            delay(2000);
            readPzemData();
            updateNextion();
        }

        // Perintah restart ESP
        if (input.indexOf("ESP_RESET") >= 0) {
            Serial.println("🔁 Restart dari Nextion!");
            sendCommand("t0.txt=\"Restarting...\"");
            Serial.flush();
            nextionSerial->flush();
            delay(300);
            ESP.restart();
        }
    }
}


void updateMonitor1() {
    sendCommand("Monitor1.t0.txt=\"" + String(tegangan1, 1) + " V\"");
    sendCommand("Monitor1.t1.txt=\"" + String(arus1, 2) + " A\"");
    sendCommand("Monitor1.t2.txt=\"" + String(daya1, 1) + " W\"");

    sendCommand("Monitor1.t3.txt=\"" + String(tegangan2, 1) + " V\"");
    sendCommand("Monitor1.t4.txt=\"" + String(arus2, 2) + " A\"");
    sendCommand("Monitor1.t5.txt=\"" + String(daya2, 1) + " W\"");

    sendCommand("Monitor1.t6.txt=\"" + String(tegangan3, 1) + " V\"");
    sendCommand("Monitor1.t7.txt=\"" + String(arus3, 2) + " A\"");
    sendCommand("Monitor1.t8.txt=\"" + String(daya3, 1) + " W\"");

    sendCommand("Monitor1.t9.txt=\"Rp " + String(totalBiaya, 1) + "\"");
    sendCommand("Monitor1.t10.txt=\"" + String(totalEnergy, 2) + " kWh\"");

    sendCommand("Monitor1.t11.txt=\"" + String(hargaListrik, 2) + "\"");


     // Tambahkan bagian ini untuk waktu dan tanggal
     DateTime now = getRTCNow();

     char tanggal[20];
     char waktu[20];
 
     sprintf(tanggal, "%02d/%02d/%04d", now.day(), now.month(), now.year());
     sprintf(waktu, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
 
     sendCommand("Monitor1.t12.txt=\"" + String(tanggal) + "\"");
     sendCommand("Monitor1.t13.txt=\"" + String(waktu) + "\"");

}

void updateMonitor2() {
    sendCommand("Monitor2.t0.txt=\"" + String(tegangan1, 1) + " V\"");
    sendCommand("Monitor2.t1.txt=\"" + String(arus1, 2) + " A\"");
    sendCommand("Monitor2.t2.txt=\"" + String(daya1, 1) + " W\"");

    sendCommand("Monitor2.t3.txt=\"" + String(tegangan2, 1) + " V\"");
    sendCommand("Monitor2.t4.txt=\"" + String(arus2, 2) + " A\"");
    sendCommand("Monitor2.t5.txt=\"" + String(daya2, 1) + " W\"");

    sendCommand("Monitor2.t6.txt=\"" + String(tegangan3, 1) + " V\"");
    sendCommand("Monitor2.t7.txt=\"" + String(arus3, 2) + " A\"");
    sendCommand("Monitor2.t8.txt=\"" + String(daya3, 1) + " W\"");

    // bedanya di sini:
    sendCommand("Monitor2.t9.txt=\"Rp " + String(biayaWbp + biayaLwbp, 2) + "\"");
    sendCommand("Monitor2.t10.txt=\"" + String(totalEnergy, 2) + " kWh\"");

    sendCommand("Monitor2.t11.txt=\"WBP: Rp " + String(hargaWbp, 2) + "\"");
    sendCommand("Monitor2.t12.txt=\"LWBP: Rp " + String(hargaLwbp, 2) + "\"");

    DateTime now = getRTCNow();

     char tanggal[20];
     char waktu[20];
 
     sprintf(tanggal, "%02d/%02d/%04d", now.day(), now.month(), now.year());
     sprintf(waktu, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
 
     sendCommand("Monitor2.t13.txt=\"" + String(tanggal) + "\"");
     sendCommand("Monitor2.t14.txt=\"" + String(waktu) + "\"");

}

void updateNextion() {
    handleNextionInput();

    if (currentPage == 1) {
        updateMonitor1();
    } else if (currentPage == 2) {
        updateMonitor2();
    }
}

