#ifndef PZEM_READER_H
#define PZEM_READER_H

#include <Arduino.h>
#include <ModbusMaster.h>
#include <Wire.h>
#include <RTClib.h>

// Deklarasi objek ModbusMaster untuk komunikasi dengan PZEM
extern ModbusMaster node1;
extern ModbusMaster node2;
extern ModbusMaster node3;

// Deklarasi objek RTC
extern RTC_DS3231 rtc;

// Definisi alamat untuk setiap fase PZEM
#define SET_ADDRESS_1 0x01
#define SET_ADDRESS_2 0x02
#define SET_ADDRESS_3 0x03

// Deklarasi variabel global yang digunakan di pzem_reader.cpp
extern float totalEnergy;
extern float previousEnergy;
extern float biayaWBP;
extern float biayaLWBP;

// Fungsi untuk inisialisasi PZEM
void pzemInit();

// Fungsi untuk membaca data dari PZEM
void readPzemData();  // Disesuaikan dengan implementasi di .cpp

// Fungsi untuk mendapatkan total energi yang telah terakumulasi
float getTotalEnergy();

#endif
