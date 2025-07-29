#include "barcode.h"
#include "Nextion.h"

void generateQRCode(const String& ssid, const String& password, const String& ip) {
    String qrData;
    qrData.reserve(100);  // Mencegah fragmentasi memori

    // Format standar QR code WiFi (tanpa IP)
    qrData = "WIFI:T:WPA;S:" + ssid + ";P:" + password + ";;";

    Serial.print("📡 QR WiFi: ");
    Serial.println(qrData);

    sendCommand("qr0.txt=\"" + qrData + "\"");


}






