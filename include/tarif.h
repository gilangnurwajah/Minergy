#ifndef TARIF_H
#define TARIF_H

#include <Arduino.h>

void prosesInputTarif(const String& input);
void loadTarifFromStorage(); // untuk dipanggil saat setup/init
void prosesInputTarifTWP(const String& input);  // Tanpa parameter
#endif
