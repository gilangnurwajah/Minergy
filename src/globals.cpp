#include "globals.h"

float totalEnergy = 0.0;
float previousEnergy = 0.0;

float tegangan1 = 0.0, tegangan2 = 0.0, tegangan3 = 0.0;
float arus1 = 0.0, arus2 = 0.0, arus3 = 0.0;
float daya1 = 0.0, daya2 = 0.0, daya3 = 0.0;
float energi1 = 0.0, energi2 = 0.0, energi3 = 0.0;
float energy1 = 0.0, energy2 = 0.0, energy3 = 0.0;
float frekuensi1 = 0.0, frekuensi2 = 0.0, frekuensi3 = 0.0;
float faktorDaya1 = 0.0, faktorDaya2 = 0.0, faktorDaya3 = 0.0;


float energyWbp = 0.0;
float energyLwbp = 0.0;
float biayaWbp = 0.0;
float biayaLwbp = 0.0;
float totalEnergyPrev = 0.0;

// Tarif awal bisa diubah saat runtime
float hargaListrik = 1444.70;
float hargaWbp = 1670.00;
float hargaLwbp = 1444.70;

float BiayaTWP = 0.0;
float TotalPower = 0.0;
bool apModeRequest = false;
