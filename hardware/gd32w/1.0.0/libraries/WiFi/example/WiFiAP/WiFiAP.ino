/*
 * WiFiAP.ino
 *
 * WiFi SoftAP Mode Example
 *
 * Description:
 * - Demonstrates WiFi SoftAP (Access Point) mode configuration
 * - Creates a WiFi network that other devices can connect to
 * - Displays AP configuration and connection information
 *
 * Features:
 * - SoftAP configuration with SSID, password, and channel
 * - Static IP configuration for AP
 * - Multiple authentication modes support (OPEN, WEP, WPA, WPA2, WPA3)
 * - AP information display (IP, MAC, subnet, gateway)
 *
 * Configuration:
 * - AP SSID: GD_Arduino_TestAP
 * - AP Password: 12345678
 * - Channel: 1
 * - Auth Mode: WPA_WPA2
 *
 * Authentication Modes:
 * - 0 = AUTH_MODE_OPEN (no password)
 * - 1 = AUTH_MODE_WEP
 * - 2 = AUTH_MODE_WPA
 * - 3 = AUTH_MODE_WPA2
 * - 4 = AUTH_MODE_WPA_WPA2
 * - 5 = AUTH_MODE_WPA2_WPA3
 * - 6 = AUTH_MODE_WPA3
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>
#include "WiFi.h"

// AP configuration
const char* ap_ssid = "GD_Arduino_TestAP";
const char* ap_password = "12345678";

// AP IP configuration (optional - defaults to 192.168.1.1 if not set)
IPAddress ap_local_ip(192, 168, 1, 1);
IPAddress ap_gateway(192, 168, 1, 1);
IPAddress ap_subnet(255, 255, 255, 0);

// AP settings
const int ap_channel = 1;              // WiFi channel (1-13)
const int ap_auth_mode = 4;            // AUTH_MODE_WPA_WPA2
const int ap_ssid_hidden = 0;          // 0 = visible, 1 = hidden

// Authentication mode options:
// 0 = AUTH_MODE_OPEN (no password)
// 1 = AUTH_MODE_WEP
// 2 = AUTH_MODE_WPA
// 3 = AUTH_MODE_WPA2
// 4 = AUTH_MODE_WPA_WPA2
// 5 = AUTH_MODE_WPA2_WPA3
// 6 = AUTH_MODE_WPA3

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  GD32VW55x WiFi AP Example");
    Serial.println("========================================");
    Serial.println();

    // Configure AP static IP (optional)
    Serial.println("Configuring AP IP settings...");
    if (WiFi.softAPConfig(ap_local_ip, ap_gateway, ap_subnet)) {
        Serial.println("AP IP configuration successful!");
        Serial.print("  IP Address: ");
        Serial.println(ap_local_ip.toString());
        Serial.print("  Gateway: ");
        Serial.println(ap_gateway.toString());
        Serial.print("  Subnet Mask: ");
        Serial.println(ap_subnet.toString());
    } else {
        Serial.println("Failed to configure AP IP settings!");
    }
    Serial.println();

    // Start SoftAP
    Serial.println("Starting SoftAP...");
    Serial.print("  SSID: ");
    Serial.println(ap_ssid);
    Serial.print("  Password: ");
    Serial.println(ap_password);
    Serial.print("  Channel: ");
    Serial.println(ap_channel);
    Serial.print("  Auth Mode: ");
    Serial.println(ap_auth_mode);
    Serial.println();

    if (WiFi.softAP((char*)ap_ssid, (char*)ap_password, ap_channel, ap_auth_mode, ap_ssid_hidden)) {
        Serial.println("SoftAP started successfully!");
        Serial.println();

        // Print AP information
        Serial.println("AP Information:");
        Serial.print("  SSID: ");
        Serial.println(WiFi.softAPSSID());
        Serial.print("  IP Address: ");
        Serial.println(WiFi.softAPIP().toString());
        Serial.print("  MAC Address: ");
        Serial.println(WiFi.softAPmacAddress());
        Serial.print("  Subnet Mask: ");
        Serial.println(WiFi.softAPSubnetMask().toString());
        Serial.print("  Broadcast IP: ");
        Serial.println(WiFi.softAPBroadcastIP().toString());
        Serial.print("  Network ID: ");
        Serial.println(WiFi.softAPNetworkID().toString());
        Serial.println();

        Serial.println("========================================");
        Serial.println("  AP is ready!");
        Serial.println("  Connect your device to: " + String(ap_ssid));
        Serial.println("  Password: " + String(ap_password));
        Serial.println("========================================");
        Serial.println();
    } else {
        Serial.println("Failed to start SoftAP!");
        Serial.println("Please check your configuration and try again.");
    }

    Serial.println("Monitoring connected stations...");
    Serial.println();
}

void loop() {
    // Print connected station count every 5 seconds
    static unsigned long last_print = 0;
    unsigned long current_time = millis();

    if (current_time - last_print >= 5000) {
        last_print = current_time;

        uint8_t station_num = WiFi.softAPgetStationNum();
        Serial.print("Connected stations: ");
        Serial.println(station_num);

        if (station_num > 0) {
            Serial.println("  Devices are connected to the AP!");
        } else {
            Serial.println("  No devices connected.");
        }
        Serial.println();
    }

    delay(100);
}
