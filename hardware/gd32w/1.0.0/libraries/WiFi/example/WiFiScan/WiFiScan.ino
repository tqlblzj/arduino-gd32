/*
 * WiFiScan.ino
 *
 * WiFi Network Scan Example
 *
 * Description:
 * - Demonstrates WiFi network scanning functionality
 * - Displays available WiFi networks with detailed information
 * - Shows SSID, RSSI, channel, and encryption type
 *
 * Features:
 * - Synchronous WiFi network scanning
 * - Network information display:
 *   - SSID (network name)
 *   - RSSI (signal strength)
 *   - Channel (frequency channel)
 *   - Encryption type (OPEN, WEP, WPA, WPA2, WPA3, etc.)
 * - Hidden network detection
 * - Automatic scan every 10 seconds
 *
 * Output Format:
 * - Number | SSID | RSSI | Channel | Encryption
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>
#include "WiFi.h"
void setup() {
    Serial.begin(115200);
    if(WiFi.STA.begin() != true) {
        Serial.println("WiFi STA failed!");
        return;
    }
    Serial.println("WiFi STA open successfully!");
}

void scanWiFi() {
    Serial.println("Scan start");
    // WiFi.scanNetworks will return the number of networks found.
    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
        Serial.println("-- | -------------------------------- | ---- | -- | ----------");
        for (int i = 0; i < n; ++i) {

            /* Nr (2 chars) */
            if (i + 1 < 10) Serial.print(" ");
            Serial.print(i + 1);
            Serial.print(" | ");

            String ssid = WiFi.SSID(i);
            if(ssid==""){
                Serial.println("This AP is hidden");
                delay(10);
                continue;
            }
            Serial.print(ssid);
            for (int s = ssid.length(); s < 32; s++) {
                Serial.print(" ");
            }
            Serial.print(" | ");

            int rssi = WiFi.RSSI(i);
            if (rssi > -100) Serial.print(" ");
            if (rssi > -10)  Serial.print(" ");
            Serial.print(rssi);
            Serial.print(" | ");

            int ch = WiFi.channel(i);
            if (ch < 10) Serial.print(" ");
            Serial.print(ch);
            Serial.print(" | ");

            /* Encryption */
            switch (WiFi.encryptionType(i)) {
                case AUTH_MODE_OPEN:        Serial.print("OPEN"); break;
                case AUTH_MODE_WEP:         Serial.print("WEP"); break;
                case AUTH_MODE_WPA:         Serial.print("WPA"); break;
                case AUTH_MODE_WPA2:        Serial.print("WPA2"); break;
                case AUTH_MODE_WPA_WPA2:    Serial.print("WPA+WPA2"); break;
                case AUTH_MODE_WPA3:        Serial.print("WPA3");break;
                case AUTH_MODE_WPA2_WPA3:   Serial.print("WPA2+WPA3"); break;
                default:                    Serial.print("unknown"); break;
        }

            Serial.println();
            delay(10);
        }

    }

    WiFi.scanDelete();
    Serial.println("-------------------------------------");
}

void loop() {
    scanWiFi();
    delay(10000);
}