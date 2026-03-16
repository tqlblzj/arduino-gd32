/*
 * WiFiStatus.ino
 *
 * WiFi Status Information Example
 *
 * Description:
 * - Demonstrates how to retrieve and display WiFi VIF status information
 * - Shows detailed STA and AP interface status
 * - Displays connection state, configuration, and statistics
 *
 * Features:
 * - WiFi VIF (Virtual Interface) information retrieval
 * - STA mode status display:
 *   - Connection state (Idle, WPS, Scanning, Connecting, etc.)
 *   - SSID, BSSID, Channel, Bandwidth
 *   - RSSI, Link quality, TX/RX rate
 *   - IP configuration (IP, Gateway, Subnet, DNS)
 * - AP mode status display:
 *   - AP configuration (SSID, Channel, Auth mode)
 *   - Connected stations list
 * - Network statistics (TX/RX packets, bytes, errors)
 *
 * VIF Types:
 * - WVIF_STA: Station mode
 * - WVIF_AP: Access Point mode
 * - WVIF_MONITOR: Monitor mode
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>
#include "WiFi.h"
#include "WiFiGeneric.h"

void printVifInfo(WiFiVifInfo* info) {
    Serial.println("========================================");
    Serial.print("VIF Index: ");
    Serial.println(info->index);
    Serial.print("Active: ");
    Serial.println(info->active ? "Yes" : "No");

    if (!info->active) {
        Serial.println("VIF is not active");
        return;
    }

    // Print MAC address
    Serial.print("MAC Address: ");
    for (int i = 0; i < 6; i++) {
        if (info->mac[i] < 16) Serial.print("0");
        Serial.print(info->mac[i], HEX);
        if (i < 5) Serial.print(":");
    }
    Serial.println();

    // Print VIF type
    Serial.print("VIF Type: ");
    switch (info->type) {
        case WVIF_STA:
            Serial.println("STA");
            break;
        case WVIF_AP:
            Serial.println("AP");
            break;
        case WVIF_MONITOR:
            Serial.println("Monitor");
            break;
        default:
            Serial.println("Unknown");
            break;
    }

    // Print STA info
    if (info->type == WVIF_STA) {
        Serial.println("--- STA Information ---");

        // Print state
        Serial.print("State: ");
        switch (info->sta.state) {
            case WIFI_STA_STATE_IDLE:
                Serial.println("Idle");
                break;
            case WIFI_STA_STATE_WPS:
                Serial.println("WPS");
                break;
            case WIFI_STA_STATE_SCAN:
                Serial.println("Scanning");
                break;
            case WIFI_STA_STATE_CONNECT:
                Serial.println("Connecting");
                break;
            case WIFI_STA_STATE_HANDSHAKE:
                Serial.println("Handshaking");
                break;
            case WIFI_STA_STATE_IP_GETTING:
                Serial.println("Getting IP");
                break;
            case WIFI_STA_STATE_CONNECTED:
                Serial.println("Connected");
                break;
            default:
                Serial.println("Unknown");
                break;
        }

        // Print configuration if connected
        if (info->sta.state >= WIFI_STA_STATE_CONNECT) {
            Serial.print("SSID: ");
            Serial.println(info->sta.config.ssid);

            Serial.print("BSSID: ");
            for (int i = 0; i < 6; i++) {
                if (info->sta.config.bssid[i] < 16) Serial.print("0");
                Serial.print(info->sta.config.bssid[i], HEX);
                if (i < 5) Serial.print(":");
            }
            Serial.println();

            Serial.print("Channel: ");
            Serial.println(info->sta.config.channel);

            Serial.print("Bandwidth: ");
            switch (info->sta.config.bw) {
                case PHY_CHNL_BW_20:
                    Serial.println("20MHz");
                    break;
                case PHY_CHNL_BW_40:
                    Serial.println("40MHz");
                    break;
                default:
                    Serial.println("Unknown");
                    break;
            }

            Serial.print("Security: ");
            switch (info->sta.config.security) {
                case AUTH_MODE_OPEN:
                    Serial.println("OPEN");
                    break;
                case AUTH_MODE_WEP:
                    Serial.println("WEP");
                    break;
                case AUTH_MODE_WPA:
                    Serial.println("WPA");
                    break;
                case AUTH_MODE_WPA2:
                    Serial.println("WPA2");
                    break;
                case AUTH_MODE_WPA_WPA2:
                    Serial.println("WPA/WPA2");
                    break;
                case AUTH_MODE_WPA2_WPA3:
                    Serial.println("WPA2/WPA3");
                    break;
                case AUTH_MODE_WPA3:
                    Serial.println("WPA3");
                    break;
                default:
                    Serial.println("Unknown");
                    break;
            }

            Serial.print("Wireless Mode: ");
            switch (info->sta.mode) {
                case WIRELESS_MODE_11B:
                    Serial.println("11b");
                    break;
                case WIRELESS_MODE_11G:
                    Serial.println("11g");
                    break;
                case WIRELESS_MODE_11BG:
                    Serial.println("11bg");
                    break;
                case WIRELESS_MODE_11N:
                    Serial.println("11n");
                    break;
                case WIRELESS_MODE_11GN:
                    Serial.println("11gn");
                    break;
                case WIRELESS_MODE_11BGN:
                    Serial.println("11bgn");
                    break;
                case WIRELESS_MODE_11GN_AX:
                    Serial.println("11gn/ax");
                    break;
                case WIRELESS_MODE_11BGN_AX:
                    Serial.println("11bgn/ax");
                    break;
                default:
                    Serial.println("Unknown");
                    break;
            }

            Serial.print("RSSI: ");
            Serial.print(info->sta.rssi);
            Serial.println(" dBm");

            // Print IP info
            if (info->sta.state >= WIFI_STA_STATE_IP_GETTING) {
                if (info->sta.ip_info.valid) {
                    Serial.print("IP Address: ");
                    Serial.println(IPAddress(info->sta.ip_info.addr));
                    Serial.print("Netmask: ");
                    Serial.println(IPAddress(info->sta.ip_info.mask));
                    Serial.print("Gateway: ");
                    Serial.println(IPAddress(info->sta.ip_info.gw));
                }

#ifdef CONFIG_IPV6_SUPPORT
                if (info->sta.ipv6_info.valid) {
                    Serial.print("IPv6 Local: ");
                    Serial.println(info->sta.ipv6_info.local);
                    Serial.print("IPv6 Unique: ");
                    Serial.println(info->sta.ipv6_info.unique);
                }
#endif
            }
        }
    }

    // Print AP info
    if (info->type == WVIF_AP) {
        Serial.println("--- AP Information ---");

        // Print state
        Serial.print("State: ");
        switch (info->ap.state) {
            case WIFI_AP_STATE_INIT:
                Serial.println("Not Started");
                break;
            case WIFI_AP_STATE_STARTED:
                Serial.println("Started");
                break;
            default:
                Serial.println("Unknown");
                break;
        }

        // Print configuration if started
        if (info->ap.state == WIFI_AP_STATE_STARTED) {
            Serial.print("SSID: ");
            Serial.println(info->ap.config.ssid);

            Serial.print("Channel: ");
            Serial.println(info->ap.config.channel);

            Serial.print("Security: ");
            switch (info->ap.config.security) {
                case AUTH_MODE_OPEN:
                    Serial.println("OPEN");
                    break;
                case AUTH_MODE_WPA:
                    Serial.println("WPA");
                    break;
                case AUTH_MODE_WPA2:
                    Serial.println("WPA2");
                    break;
                case AUTH_MODE_WPA_WPA2:
                    Serial.println("WPA/WPA2");
                    break;
                case AUTH_MODE_WPA2_WPA3:
                    Serial.println("WPA2/WPA3");
                    break;
                case AUTH_MODE_WPA3:
                    Serial.println("WPA3");
                    break;
                default:
                    Serial.println("Unknown");
                    break;
            }

            Serial.print("Wireless Mode: ");
            switch (info->ap.mode) {
                case WIRELESS_MODE_11BGN:
                    Serial.println("11bgn");
                    break;
                case WIRELESS_MODE_11BGN_AX:
                    Serial.println("11bgn/ax");
                    break;
                default:
                    Serial.println("Unknown");
                    break;
            }

            // Print IP info
            if (info->ap.ip_info.valid) {
                Serial.print("IP Address: ");
                Serial.println(IPAddress(info->ap.ip_info.addr));
                Serial.print("Netmask: ");
                Serial.println(IPAddress(info->ap.ip_info.mask));
                Serial.print("Gateway: ");
                Serial.println(IPAddress(info->ap.ip_info.gw));
            }

#ifdef CONFIG_IPV6_SUPPORT
            if (info->ap.ipv6_info.valid) {
                Serial.print("IPv6 Local: ");
                Serial.println(info->ap.ipv6_info.local);
                Serial.print("IPv6 Unique: ");
                Serial.println(info->ap.ipv6_info.unique);
            }
#endif

            // Print connected clients
            Serial.print("Connected Clients: ");
            Serial.println(info->ap.client_count);
            for (int i = 0; i < info->ap.client_count; i++) {
                Serial.print("    Client[");
                Serial.print(i);
                Serial.print("] MAC: ");
                for(int k=0; k < 6; k++){
                    Serial.print(info->ap.clients[i].mac[k], HEX);
                    if(k < 5) {
                        Serial.print(":");
                    }
                }
                // Serial.print(info->ap.clients[i].mac[0], HEX);
                // Serial.print(":");
                // Serial.print(info->ap.clients[i].mac[1], HEX);
                // Serial.print(":");
                // Serial.print(info->ap.clients[i].mac[2], HEX);
                // Serial.print(":");
                // Serial.print(info->ap.clients[i].mac[3], HEX);
                // Serial.print(":");
                // Serial.print(info->ap.clients[i].mac[4], HEX);
                // Serial.print(":");
                // Serial.print(info->ap.clients[i].mac[5], HEX);


                if (info->ap.clients[i].ip_known) {
                    Serial.print(" IP: ");
                    Serial.print(info->ap.clients[i].ip & 0xFF, DEC);
                    Serial.print(".");
                    Serial.print(info->ap.clients[i].ip >> 8 & 0xFF, DEC);
                    Serial.print(".");
                    Serial.print(info->ap.clients[i].ip >> 16 & 0xFF, DEC);
                    Serial.print(".");
                    Serial.println(info->ap.clients[i].ip >> 24 & 0xFF, DEC);
                } else {
                    Serial.println(" IP: unknown");
                }
            }
        }
    }

    // Print Monitor info
    if (info->type == WVIF_MONITOR) {
        Serial.println("--- Monitor Information ---");
        Serial.println("Monitor mode is active");
    }
}

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

char apssid[] = "gd_test_ap";
char appassword[] = "12345678";
void setup() {
    Serial.begin(115200);
    Serial.println("\r\n");
    Serial.println("WiFi Status Test");
    Serial.println("==============================\r\n");

    WiFiVifInfo vif_info;
    sys_memset(&vif_info, 0, sizeof(WiFiVifInfo));

    // Get VIF 0 status
    if (WiFi.getStatus(&vif_info, 0) == 0) {
        printVifInfo(&vif_info);
    } else {
        Serial.println("Failed to get VIF 0 status");
    }

    Serial.println("\r\n");

    delay(3000);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("Connected");
    if (WiFi.getStatus(&vif_info, 0) == 0) {
        printVifInfo(&vif_info);
    } else {
        Serial.println("Failed to get VIF 0 status");
    }

    delay(3000);
    WiFi.disconnect();

    delay(3000);

    // use WiFi.STA.end(true) to turn off WiFi after STA end
    WiFi.STA.end();
    Serial.println("\r\n");

    WiFi.softAP(apssid, appassword, 1, 4, 0);
    if (WiFi.getStatus(&vif_info, 0) == 0) {
        printVifInfo(&vif_info);
    } else {
        Serial.println("Failed to get VIF 0 status");
    }

    delay(20000);
    if (WiFi.getStatus(&vif_info, 0) == 0) {
        printVifInfo(&vif_info);
    } else {
        Serial.println("Failed to get VIF 0 status");
    }
    WiFi.freeVifInfo(&vif_info);
    WiFi.AP.end();
}

void loop() {
    delay(50);
}