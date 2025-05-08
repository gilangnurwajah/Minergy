#ifndef NEXTION_H
#define NEXTION_H

#include <Arduino.h>

String getCommand(const String& command);

void initNextion(HardwareSerial &serial);
void updateNextion();  // Kirim semua data global ke Nextion

void updateMonitor1();
void updateMonitor2();

extern int currentPage;
extern bool requestApMode;

void sendCommand(const String &cmd);

#endif
