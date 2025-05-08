#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Preferences.h>

float loadTotalEnergyPrev();
extern Preferences preferences; // Deklarasi variabel global yang ada di main.cpp

void storageInit();
void saveEnergyData(float energyWbp, float energyLwbp, float biayaWbp, float biayaLwbp);
void loadEnergyData(float &energyWbp, float &energyLwbp, float &biayaWbp, float &biayaLwbp);
void saveTotalEnergyPrev(float energy); // Tambahkan deklarasi ini


// Tambahan fungsi untuk tarif
void saveHargaListrik(float harga);
void saveHargaTWP(float wbp, float lwbp);
float loadHargaListrik();
void loadHargaTWP(float &wbp, float &lwbp);


#endif

