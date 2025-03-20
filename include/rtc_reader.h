#ifndef RTC_READER_H
#define RTC_READER_H

#include <Wire.h>
#include <RTClib.h>

void rtcInit();
void printDateTime();
DateTime getRTCNow();

#endif
