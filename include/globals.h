#ifndef GLOBALS_H
#define GLOBALS_H

#include <Preferences.h>
#include <Arduino.h>


// extern float totalEnergy;
extern float previousEnergy;

extern float tegangan1, tegangan2, tegangan3;
extern float arus1, arus2, arus3;
extern float daya1, daya2, daya3;
extern float energi1, energi2, energi3;
extern float frekuensi1, frekuensi2, frekuensi3;
extern float faktorDaya1, faktorDaya2, faktorDaya3;

extern float totalEnergy;
extern float totalBiaya; //untuk pengiriman ke TB
const float hargaListrik = 1444.70;  // Sesuaikan dengan tarif listrik per kWh

extern float biayaWbp;
extern float biayaLwbp;
const float hargaLwbp = 1444.70;  // Tarif LWBP per kWh
const float hargaWbp = 1670.00;   // Tarif WBP per kWh

extern float energyWbp;
extern float energyLwbp;
extern float totalEnergyPrev;



#endif
