#include "RTC_reader.h"

RTC_DS3231 rtc;

void rtcInit() {
    Wire.begin(21, 22);  // Pastikan ini sesuai dengan pin I2C ESP32

    if (!rtc.begin()) {
        Serial.println("⚠ RTC tidak ditemukan!");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("🔄 RTC kehilangan daya, menyetel ulang waktu...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set waktu saat kompilasi
    }
}

void printDateTime() {
    DateTime now = rtc.now();
    Serial.printf("🕒 Waktu: %02d:%02d:%02d %02d/%02d/%04d\n",
                  now.hour(), now.minute(), now.second(),
                  now.day(), now.month(), now.year());
}

DateTime getRTCNow() {
    return rtc.now();
}