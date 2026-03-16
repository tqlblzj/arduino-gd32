/*
 * WiFiSTAConnect.ino
 *
 * WiFi STA Connection Example
 *
 * Description:
 * - Demonstrates WiFi STA (Station) mode connection
 * - Shows both synchronous and asynchronous connection methods
 * - Displays MAC address in different formats
 *
 * Features:
 * - Synchronous WiFi connection (blocking)
 * - Asynchronous WiFi connection (non-blocking)
 * - Connection status monitoring with WiFi.status()
 * - MAC address retrieval (String and byte array formats)
 * - Disconnect functionality
 *
 * Connection States:
 * - WL_CONNECTED: Connected to AP
 * - WL_DISCONNECTED: Disconnected from AP
 * - WL_CONNECT_ONGOING: Connection in progress (async mode)
 *
 * Test Sequence:
 * 1. Synchronous connection test
 * 2. Disconnect
 * 3. Asynchronous connection test
 * 4. Disconnect
 * 5. Repeat every 20 seconds
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>
#include "WiFi.h"

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

void setup() {
    Serial.begin(115200);
}

void testWiFiConnect()
{
    Serial.println("Connect AP synchronously:");
    if(WiFi.begin(ssid, password) != WL_CONNECTED) {
        Serial.println("WiFi STA connection failed!");
        return;
    }
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi STA connected successfully!");
    }

    String mac = WiFi.macAddress();
    Serial.print("Device MAC Address: ");
    Serial.println(mac);

    delay(5000);

    if(WiFi.disconnect() && WiFi.status() == WL_DISCONNECTED) {
        Serial.println("WiFi STA disconnected successfully!");
    }else {
        Serial.println("WiFi STA disconnection failed!");
    }

    Serial.println("Connect AP asynchronously:");
    if(WiFi.begin(ssid, password, false) != WL_CONNECT_ONGOING) {
        Serial.println("Async WiFi STA connection failed!");
    }

    while (WiFi.status() == WL_CONNECT_ONGOING)
    {
        Serial.print("Waiting for connection...\n");
        delay(1000);
    }

    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("Async WiFi STA connected successfully!");
    } else {
        Serial.println("Async WiFi STA connection failed!");
    }

    uint8_t* macAddr = new uint8_t[6];
    WiFi.macAddress(macAddr);
    Serial.print("Device MAC Address: ");
    for(int i = 0; i < 6; i++) {
        Serial.print(macAddr[i], HEX);
        if(i < 5) Serial.print(":");
    }
    Serial.println();

    delay(5000);

    if(WiFi.disconnect() && WiFi.status() == WL_DISCONNECTED) {
        Serial.println("WiFi STA disconnected successfully!");
    }
    else {
        Serial.println("WiFi STA disconnection failed!");
    }
}


void loop()
{
    testWiFiConnect();
    delay(20000);
}
