/*
 ESP8266WiFiScan.h - esp8266 Wifi support.
 Based on WiFi.h from Ardiono WiFi shield library.
 Copyright (c) 2011-2014 Arduino.  All right reserved.
 Modified by Ivan Grokhotkov, December 2014
 Reworked by Markus Sattler, December 2015
 Modified 12 Feb 2026 by GigaDevice Semiconductor Inc (adapt to the first version of gd32 arduino sdk).

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef WIFISCAN_H
#define WIFISCAN_H

#include <stdint.h>
#include <functional>
#include "WString.h"
// #include "WiFiType.h"
// #include "NetworkEvent.h"


#define WIFI_SCAN_RUNNING (-1)
#define WIFI_SCAN_FAILED  (-2)

// Scan result structure
typedef struct {
    char    ssid[33];
    char    ap_power;
    char    bssid[6];
    char    channel;
    uint8_t security;
} wifi_ap_info_t;

typedef struct {
    uint16_t ap_num;
    wifi_ap_info_t *ap_list;
} wifi_scan_result_t;

// Scan callback function type
typedef std::function<void(int count)> WiFiScanDoneCallback;

class WiFiScanClass {
public:
    WiFiScanClass();
    ~WiFiScanClass();

    void setScanTimeout(uint32_t ms);
    // void setScanActiveMinTime(uint32_t ms);

    int16_t scanNetworks(bool async = false, bool show_hidden = false, bool passive = false, uint32_t max_ms_per_chan = 300, uint8_t channel = 0, const char *ssid = nullptr, const uint8_t *bssid = nullptr);

    int16_t scanComplete();

    void scanDelete();

    // Get scan result info by index
    String SSID(uint8_t i);
    String BSSID(uint8_t i);
    int32_t RSSI(uint8_t i);
    uint8_t encryptionType(uint8_t i);
    int32_t channel(uint8_t i);

    wifi_scan_result_t* getScanResults();

    void handleScanResults();

protected:
    static int _rssi_compare(const void *a, const void *b);
    void _processScanResults();

    // Scan state
    bool _scanRunning;
    bool _scanComplete;

    int32_t _scanStartTime;

    // Scan results storage
    wifi_scan_result_t _scanResults;
    int16_t _maxResults;

    // Scan configuration
    uint32_t _scanTimeout;
    uint32_t _scanActiveMinTime;

    // Callback for async scan
    WiFiScanDoneCallback _scanCompleteCallback;

    friend class WiFiSTAClass;
    friend class WiFiGenericClass;
};

#endif // WIFISCAN_H
