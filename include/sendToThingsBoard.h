#ifndef SENDTOTB_H
#define SENDTOTB_H

#include <Arduino.h>
#include <PubSubClient.h>

// Deklarasi fungsi
void sendToThingsBoard();
void reconnectMQTT();

// Variabel eksternal dari `main.cpp`
extern PubSubClient client;
extern const char *thingsboardToken;
extern float tegangan1, tegangan2, tegangan3;
extern float arus1, arus2, arus3;
extern float daya1, daya2, daya3;
extern float energi1, energi2, energi3;
extern float frekuensi1, frekuensi2, frekuensi3;
extern float faktorDaya1, faktorDaya2, faktorDaya3;
extern float totalEnergy;

#endif
