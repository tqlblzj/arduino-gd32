/*
 * WiFiNTPTimeSync.ino
 *
 * WiFi NTP Time Sync Example
 *
 * Description:
 * - Demonstrates NTP (Network Time Protocol) time synchronization
 * - Syncs RTC with NTP server via WiFi connection
 * - Supports timezone offset configuration
 *
 * Features:
 * - NTP packet construction and parsing
 * - RTC time synchronization
 * - Timezone offset support (UTC+8 default)
 * - Automatic periodic time sync
 *
 * Configuration:
 * - NTP server: configurable (default: 172.20.10.3)
 * - NTP port: 123 (standard)
 * - Timezone offset: UTC+8 (China)
 *
 * Usage:
 * 1. Update WiFi credentials below
 * 2. Optionally change NTP server to a public one like "pool.ntp.org"
 * 3. Upload and monitor serial output for time sync status
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
#include "WiFi.h"
#include "HardwareRTC.h"
#include <time.h>

// NTP server configuration
#define NTP_SERVER  "172.20.10.3" // You can use a public NTP server like "pool.ntp.org" or your local NTP server IP address
#define NTP_PORT    123           // User should not change this port, NTP always uses 123
#define NTP_PACKET_SIZE 48        // NTP time packet size is always 48 bytes

// WiFi configuration
char *ssid = "WIFI_SSID";
char *password = "WIFI_PASSWORD";

// NTP timestamp offset (seconds from 1900-01-01 to 1970-01-01)
#define NTP_OFFSET 2208988800UL

// UTC timezone offset (China UTC+8)
#define TIMEZONE_OFFSET 8 * 3600

// HWRTC rtc;
WiFiUDP udp;

// NTP packet buffer
byte ntpPacketBuffer[NTP_PACKET_SIZE];

UTCTimeStruct currentTime;

// Send NTP request
void sendNTPpacket() {
    // Initialize NTP packet
    memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);

    // Set NTP protocol version and mode
    ntpPacketBuffer[0] = 0b11100011;  // LI=0, VN=4, Mode=3 (Client)
    ntpPacketBuffer[1] = 0;           // Stratum
    ntpPacketBuffer[2] = 6;           // Polling Interval
    ntpPacketBuffer[3] = 0xEC;        // Peer Clock Precision

    // 8 bytes zero, root delay and root dispersion
    ntpPacketBuffer[12] = 49;
    ntpPacketBuffer[13] = 0x4E;
    ntpPacketBuffer[14] = 49;
    ntpPacketBuffer[15] = 52;

    // Send NTP packet
    udp.beginPacket(NTP_SERVER, NTP_PORT);
    udp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

// Parse timestamp from NTP response
unsigned long parseNTPTime() {
    // Check if enough data received
    udp.parsePacket();
    int avail = udp.available();
    if (avail >= NTP_PACKET_SIZE) {
        // Read NTP packet
        udp.read(ntpPacketBuffer, NTP_PACKET_SIZE);

        // Timestamp starts at byte 40 (4 bytes)
        unsigned long highWord = word(ntpPacketBuffer[40], ntpPacketBuffer[41]);
        unsigned long lowWord = word(ntpPacketBuffer[42], ntpPacketBuffer[43]);

        // Combine to NTP timestamp (seconds since 1900-01-01)
        unsigned long secsSince1900 = highWord << 16 | lowWord;

        // Convert to Unix timestamp (seconds since 1970-01-01)
        unsigned long secsSince1970 = secsSince1900 - NTP_OFFSET;

        return secsSince1970;
    }
    return 0;
}

// Sync RTC with NTP time
bool syncRTCWithNTP() {
    Serial.println("Connecting to NTP server...");

    // Start UDP connection
    udp.begin(NTP_PORT);

    // Send NTP request
    sendNTPpacket();

    // Wait for NTP response
    unsigned long startTime = millis();
    while (millis() - startTime < 10000) {  // 10 second timeout
        udp.parsePacket();
        int avail = udp.available();
        if (avail) {
            unsigned long unixTime = parseNTPTime();
            if (unixTime > 0) {
                // Add timezone offset
                unsigned long localTime = unixTime + TIMEZONE_OFFSET;

                // Manually decompose Unix timestamp
                unsigned long days = localTime / 86400;
                unsigned long seconds = localTime % 86400;

                // Calculate hour, minute, second
                uint8_t hour = seconds / 3600;
                uint8_t minute = (seconds % 3600) / 60;
                uint8_t second = seconds % 60;

                // Calculate year, month, day (simplified algorithm for 1970-2099)
                uint16_t year = 1970;
                uint8_t month = 1;
                uint8_t day = 1;

                unsigned long remainingDays = days;
                while (remainingDays > 0) {
                    // Check if leap year
                    bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
                    unsigned long daysInYear = isLeapYear ? 366 : 365;

                    if (remainingDays >= daysInYear) {
                        remainingDays -= daysInYear;
                        year++;
                    } else {
                        // Calculate month
                        uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                        if (isLeapYear) daysInMonth[1] = 29;

                        for (month = 0; month < 12 && remainingDays >= daysInMonth[month]; month++) {
                            remainingDays -= daysInMonth[month];
                        }
                        day = remainingDays + 1;
                        month++;  // Month starts from 1
                        break;
                    }
                }

                // Create UTCTimeStruct
                UTCTimeStruct utcTime;
                utcTime.year = year;
                utcTime.month = month;
                utcTime.day = day;
                utcTime.hour = hour;
                utcTime.minutes = minute;
                utcTime.seconds = second;

                // Update RTC
                rtc.setUTCTime(&utcTime);

                udp.stop();
                return true;
            }
        }
        delay(100);
    }

    Serial.println("NTP sync timeout");
    udp.stop();
    return false;
}

// Print current RTC time
void getCurrentTime() {
    rtc.getUTCTime(&currentTime);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("WiFi Time Sync Example");

    // Connect to WiFi
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin((char*)ssid, (char*)password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Sync RTC time
    if (syncRTCWithNTP()) {
        Serial.println("Time sync successful!");
    } else {
        Serial.println("Time sync failed!");
    }

    // Attach RTC second interrupt
    rtc.attachInterrupt(getCurrentTime, INT_SECOND_MODE);
}

void loop() {
    Serial.print("Current time: ");
    Serial.print(currentTime.year);
    Serial.print("-");
    Serial.print(currentTime.month);
    Serial.print("-");
    Serial.print(currentTime.day);
    Serial.print(" ");
    Serial.print(currentTime.hour);
    Serial.print(":");
    Serial.print(currentTime.minutes);
    Serial.print(":");
    Serial.print(currentTime.seconds);
    Serial.println();
    delay(2000);
}
