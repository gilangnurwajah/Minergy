#ifndef PZEM_READER_H_
#define PZEM_READER_H_

#include <Arduino.h>
#include <ModbusMaster.h>
#include <Wire.h>
#include <RTClib.h>
#include "globals.h"  // Menyertakan globals.h agar variabel global dikenali

// Deklarasi objek ModbusMaster untuk komunikasi dengan PZEM
extern ModbusMaster node1;
extern ModbusMaster node2;
extern ModbusMaster node3;

// Definisi alamat untuk setiap fase PZEM
#define PZEM_ADDRESS_1 0x01
#define PZEM_ADDRESS_2 0x02
#define PZEM_ADDRESS_3 0x03

// Fungsi untuk inisialisasi PZEM
void pzemInit();

// Fungsi untuk membaca data dari PZEM
void readPzemData();  

// Fungsi untuk mendapatkan total energi yang telah terakumulasi
float getTotalEnergy();

// Fungsi untuk mereset total energi
void resetEnergy();

//fungsi untuk biaya wbp dan lwbp
void hitungBiaya();

#endif  // PZEM_READER_H_
