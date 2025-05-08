#include "tarif.h"
#include "globals.h"
#include <Preferences.h>
#include "nextion.h"

Preferences tarifPrefs;

void loadTarifFromStorage() {
    tarifPrefs.begin("tarif", true);  // mode read

    // Memuat nilai tarif dari Preferences
    hargaListrik = tarifPrefs.getFloat("hargaListrik", hargaListrik);  // Jika belum ada data, menggunakan nilai default yang ada di globals.cpp
    hargaWbp     = tarifPrefs.getFloat("hargaWbp", hargaWbp);
    hargaLwbp    = tarifPrefs.getFloat("hargaLwbp", hargaLwbp);

    tarifPrefs.end();  // Menutup akses ke Preferences

    // Menampilkan tarif yang dimuat ke serial monitor
    Serial.printf("📦 Tarif dimuat: REG=%.2f, WBP=%.2f, LWBP=%.2f\n", hargaListrik, hargaWbp, hargaLwbp);
}

// Fungsi untuk memproses input tarif
void prosesInputTarif(const String& input) {
    // Menghapus prefix REG:
    String tarif = input.substring(4);  // Ambil string setelah "REG:"
    Serial.println("Tarif yang diterima: " + tarif);

    // Simpan tarif ke dalam variabel
    hargaListrik = tarif.toFloat();  // Misalnya jika input berupa angka desimal
    Serial.printf("Harga Listrik Baru: %.2f\n", hargaListrik);

    // Update tampilan di Nextion (misalnya ke layar Monitor1.t11)
    sendCommand("Monitor1.t11.txt=\"" + String(hargaListrik, 2) + "\"");

    // Menyimpan tarif yang baru ke Preferences agar tetap ada meskipun restart
    tarifPrefs.begin("tarif", false);  // mode write
    tarifPrefs.putFloat("hargaListrik", hargaListrik);
    tarifPrefs.end();
}

// Fungsi untuk mengambil nilai dari t0 dan menampilkannya di Monitor1.t11
void tampilkanInputDiTarifReguler() {
    // Ambil nilai yang diinputkan pada t0 (tarif input pada halaman TarifReguler)
    String tarifInput = getCommand("TarifReguler.t0.txt");
    
    // Simpan nilai input ke Preferences agar tetap ada meskipun restart
    tarifPrefs.begin("tarif", false);  // mode write
    tarifPrefs.putFloat("hargaListrik", tarifInput.toFloat());  // Simpan nilai input ke dalam preferensi
    tarifPrefs.end();
}
