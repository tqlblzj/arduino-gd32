/*
 * WiFiSTAConfig.ino
 *
 * WiFi Static IP Configuration Example
 *
 * Description:
 * - Demonstrates WiFi.config() function for static IP configuration
 * - Shows how to set static IP, gateway, subnet mask, and DNS servers
 * - Demonstrates switching between static IP and DHCP modes
 *
 * Features:
 * - Static IP configuration with WiFi.config()
 * - Current IP configuration retrieval with WiFi.getConfig()
 * - DHCP mode enable with WiFi.config(INADDR_NONE)
 * - Connection test with both static and DHCP modes
 *
 * Configuration:
 * - Static IP: 192.168.4.100
 * - Gateway: 192.168.4.1
 * - Subnet: 255.255.255.0
 * - DNS: 0.0.0.0 (use gateway)
 *
 * Test Sequence:
 * 1. Configure static IP
 * 2. Connect to WiFi and verify
 * 3. Disconnect
 * 4. Enable DHCP mode
 * 5. Reconnect and verify
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>
#include "WiFi.h"

// WiFi credentials
char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";
IPAddress current_local_ip, current_gateway, current_subnet, current_dns;

// Static IP configuration
IPAddress local_ip(192, 168, 4, 100);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(0, 0, 0, 0);
IPAddress dns2(0, 0, 0, 0);

void setup() {

    Serial.begin(115200);
    delay(100);

    if (!WiFi.config(local_ip, gateway, subnet, dns1, dns2)) {
        Serial.println("Failed to configure static IP!");
        return;
    }

    Serial.println("Static IP configured successfully!");

    Serial.println("Getting current IP configuration...");
    if (WiFi.getConfig(&current_local_ip, &current_gateway, &current_subnet, &current_dns)) {
        Serial.println("Current IP configuration:");
        Serial.print("IP Address: ");
        Serial.println(current_local_ip.toString());
        Serial.print("Gateway: ");
        Serial.println(current_gateway.toString());
        Serial.print("Subnet Mask: ");
        Serial.println(current_subnet.toString());
        Serial.print("DNS Server: ");
        Serial.println(current_dns.toString());
    } else {
        Serial.println("Failed to get IP configuration!");
    }
    Serial.println();

    Serial.println("Connecting to WiFi...");
    if (WiFi.begin(ssid, password) != WL_CONNECTED) {
        Serial.println("WiFi connection failed!");
        return;
    }

    Serial.println("WiFi connected successfully!");
    Serial.print("Device MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Device IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    delay(5000);

    if (WiFi.disconnect()) {
        Serial.println("WiFi disconnected.");
    }

    delay(5000);
    Serial.println();
    Serial.println("Resetting to DHCP mode...");

    // TODO, always print Failed to enable DHCP, even when it works
    if (WiFi.config(INADDR_NONE)) {
        Serial.println("DHCP enabled successfully!");
    } else {
        Serial.println("Failed to enable DHCP!");
    }

    Serial.println("Connecting to WiFi...");
    if (WiFi.begin(ssid, password) != WL_CONNECTED) {
        Serial.println("WiFi connection failed!");
        return;
    }

    Serial.println("Getting current IP configuration...");
    if (WiFi.getConfig(&current_local_ip, &current_gateway, &current_subnet, &current_dns)) {
        Serial.println("Current IP configuration:");
        Serial.print("IP Address: ");
        Serial.println(current_local_ip.toString());
        Serial.print("Gateway: ");
        Serial.println(current_gateway.toString());
        Serial.print("Subnet Mask: ");
        Serial.println(current_subnet.toString());
        Serial.print("DNS Server: ");
        Serial.println(current_dns.toString());
    } else {
        Serial.println("Failed to get IP configuration!");
    }

    Serial.println();
    IPAddress local_ip = WiFi.localIP();
    Serial.print("WiFi.localIP(): ");
    Serial.println(local_ip.toString());

    IPAddress gateway = WiFi.gatewayIP();
    Serial.print("gatewayIP(): ");
    Serial.println(gateway.toString());

    IPAddress subnet = WiFi.subnetMask();
    Serial.print("subnetMask: ");
    Serial.println(subnet.toString());

    if (WiFi.disconnect()) {
        Serial.println("WiFi disconnected.");
    }
}

void loop() {
    delay(50);
}