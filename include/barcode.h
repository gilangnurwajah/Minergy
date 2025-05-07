#ifndef BARCODE_H
#define BARCODE_H

#include <Arduino.h>

// Fungsi untuk menghasilkan dan menampilkan QR code ke Nextion
void generateQRCode(const String& apSSID, const String& apPassword, const String& ip);

#endif


