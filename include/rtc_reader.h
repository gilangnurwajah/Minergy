#ifndef RTC_READER_H
#define RTC_READER_H

#include <Wire.h>
#include <RTClib.h>

void rtcInit();
void printDateTime();
DateTime getRTCNow();
extern RTC_DS3231 rtc;  // <-- Tambahkan ini agar rtc dikenali di main.cpp

#endif
