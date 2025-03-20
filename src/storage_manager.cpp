#include "storage_manager.h"

void storageInit() {
    preferences.begin("energyData", false); // Namespace "energyData"
}

void loadEnergyData(float &energyWbp, float &energyLwbp, float &biayaWbp, float &biayaLwbp) {
    preferences.begin("energyData", true);
    
    float prevEnergyWbp = preferences.getFloat("energyWbp", 0.0);
    float prevEnergyLwbp = preferences.getFloat("energyLwbp", 0.0);

    energyWbp = prevEnergyWbp;  
    energyLwbp = prevEnergyLwbp; 

    biayaWbp = preferences.getFloat("biayaWbp", 0.0);
    biayaLwbp = preferences.getFloat("biayaLwbp", 0.0);

    Serial.printf("📤 Memuat Data: WBP=%.3f kWh, LWBP=%.3f kWh\n", energyWbp, energyLwbp);

    preferences.end();
}

void saveEnergyData(float energyWbp, float energyLwbp, float biayaWbp, float biayaLwbp) {
    preferences.begin("energyData", false);

    float prevEnergyWbp = preferences.getFloat("energyWbp", 0.0);
    float prevEnergyLwbp = preferences.getFloat("energyLwbp", 0.0);

    Serial.printf("🔍 Sebelum Menyimpan: WBP=%.3f kWh, LWBP=%.3f kWh (sebelumnya WBP=%.3f, LWBP=%.3f)\n",
                  energyWbp, energyLwbp, prevEnergyWbp, prevEnergyLwbp);

    if (energyWbp != prevEnergyWbp || energyLwbp != prevEnergyLwbp) {
        Serial.println("💾 Data berubah, menyimpan ulang...");
        preferences.putFloat("energyWbp", energyWbp);
        preferences.putFloat("energyLwbp", energyLwbp);
    } else {
        Serial.println("✅ Data tidak berubah, tidak disimpan ulang.");
    }

    preferences.putFloat("biayaWbp", biayaWbp);
    preferences.putFloat("biayaLwbp", biayaLwbp);

    preferences.end();
}

void saveTotalEnergyPrev(float totalEnergyPrev) {
    Preferences preferences;
    preferences.begin("energy", false);
    preferences.putFloat("totalEnergyPrev", totalEnergyPrev);
    preferences.end();
}

float loadTotalEnergyPrev() {
    Preferences preferences;
    preferences.begin("energy", true);
    float value = preferences.getFloat("totalEnergyPrev", 0.0);
    preferences.end();
    return value;
}
