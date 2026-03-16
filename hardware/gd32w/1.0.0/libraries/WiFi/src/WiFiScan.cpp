/*
 ESP8266WiFiScan.cpp - WiFi library for esp8266

 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

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

 Reworked on 28 Dec 2015 by Markus Sattler
 Modified 12 Feb 2026 by GigaDevice Semiconductor Inc (adapt to the first version of gd32 arduino sdk).
 */

#include "WiFiScan.h"
#include "WiFiSTA.h"
#include "wifi_management.h"
#include "wifi_netlink.h"
#include "macif_api.h"
#include "mac_types.h"
#include <string.h>
#include <Arduino.h>
#include "wifi_export.h"
#include "wifi_eloop.h"
#include "WiFi.h"
#include "NetworkEvents.h"

// Global instance
WiFiScanClass WiFiScan;

// Default scan timeout (10 seconds)
#define DEFAULT_SCAN_TIMEOUT_MS 10000

WiFiScanClass::WiFiScanClass()
    :_scanRunning(false), _scanComplete(false), _maxResults(SCANU_MAX_RESULTS),
     _scanTimeout(DEFAULT_SCAN_TIMEOUT_MS), _scanActiveMinTime(0), _scanCompleteCallback(nullptr),
     _scanStartTime(0)
{

}


WiFiScanClass::~WiFiScanClass()
{

}

static void scan_done_cb(void *eloop_data, void *user_ctx)
{
    WiFi.handleScanResults();
    eloop_event_unregister(WIFI_MGMT_EVENT_SCAN_DONE);
    eloop_event_unregister(WIFI_MGMT_EVENT_SCAN_FAIL);
    arduino_event_t arduino_event;
    arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_SCAN_DONE;
    Network.postEvent(&arduino_event);
}

static void scan_fail_cb(void *eloop_data, void *user_ctx)
{
    eloop_event_unregister(WIFI_MGMT_EVENT_SCAN_DONE);
    eloop_event_unregister(WIFI_MGMT_EVENT_SCAN_FAIL);

    arduino_event_t arduino_event;
    arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_SCAN_FAIL;
    Network.postEvent(&arduino_event);
}

int16_t WiFiScanClass::scanNetworks(bool async, bool show_hidden, bool passive, uint32_t max_ms_per_chan, uint8_t channel, const char *ssid, const uint8_t *bssid)
{
    _scanStartTime = millis();
    scanDelete();
    _scanComplete = false;
    _scanRunning = true;

    // // Start scan
    int result = wifi_management_scan(async ? 0 : 1, ssid);
    if (result < 0) {
        return -1;
    }
    if(async){
        eloop_event_register(WIFI_MGMT_EVENT_SCAN_DONE, scan_done_cb, NULL, NULL);
        eloop_event_register(WIFI_MGMT_EVENT_SCAN_FAIL, scan_fail_cb, NULL, NULL);
        return 0;
    }

    handleScanResults();

    return _scanResults.ap_num;
}

void WiFiScanClass::scanDelete()
{
    if (_scanResults.ap_list) {
        sys_mfree(_scanResults.ap_list);
        _scanResults.ap_list = nullptr;
        _scanResults.ap_num = 0;
    }
}

int WiFiScanClass::_rssi_compare(const void *a, const void *b)
{
    struct mac_scan_result *resultA = (struct mac_scan_result *)a;
    struct mac_scan_result *resultB = (struct mac_scan_result *)b;

    return resultB->rssi - resultA->rssi;
}

/**
 * @brief Processes the WiFi scan results, organizing and storing AP information into internal structures.
 *
 * This function retrieves WiFi scan results, sorts them by signal strength (RSSI),
 * and parses each AP's information (such as BSSID, SSID, channel, security mode, etc.)
 * into the [_scanResults] structure. Finally, it frees temporary memory and updates
 * the scan status flags.
 *
 * @note This function takes no parameters and returns no value. All processing results
 *       are stored in class member variables.
 */
void WiFiScanClass::_processScanResults()
{
    uint32_t result_cnt = 0;
    struct macif_scan_results *results;
    uint32_t akm;
    uint32_t group_cipher;
    uint32_t idx = 0;

    sys_memset(&_scanResults, 0, sizeof(_scanResults));

    results = (struct macif_scan_results *)sys_malloc(sizeof(struct macif_scan_results));

    if (NULL == results) {
        goto exit;
    }

    if (wifi_netlink_scan_results_get(WIFI_VIF_INDEX_DEFAULT, results)) {
        goto exit;
    }

    if (results->result_cnt == 0) {
        goto exit;
    }

    _scanResults.ap_num = results->result_cnt;

    _scanResults.ap_list =
        (wifi_ap_info_t *)
        sys_malloc(sizeof(wifi_ap_info_t) * results->result_cnt);

    if (_scanResults.ap_list == NULL) {
        goto exit;
    }

    sys_memset(_scanResults.ap_list, 0, sizeof(*_scanResults.ap_list) * results->result_cnt);

    qsort(results->result, results->result_cnt, sizeof(struct mac_scan_result), _rssi_compare);

    result_cnt = results->result_cnt;
    for (idx = 0; idx < result_cnt; idx++) {
        akm = results->result[idx].akm;
        group_cipher = results->result[idx].group_cipher;

        sys_memcpy(_scanResults.ap_list[idx].bssid, results->result[idx].bssid.array, 6);

        _scanResults.ap_list[idx].channel = wifi_freq_to_channel(results->result[idx].chan->freq);
        _scanResults.ap_list[idx].ap_power = results->result[idx].rssi;

        // Determine security mode based on AKM and group cipher types
        if (akm & CO_BIT(MAC_AKM_NONE))
            _scanResults.ap_list[idx].security = AUTH_MODE_OPEN;
        else if (akm == CO_BIT(MAC_AKM_PRE_RSN))
            _scanResults.ap_list[idx].security = AUTH_MODE_WEP;
        else if ((akm & CO_BIT(MAC_AKM_SAE)) && (akm & CO_BIT(MAC_AKM_PSK)))
            _scanResults.ap_list[idx].security = AUTH_MODE_WPA2_WPA3;
        else if (akm & CO_BIT(MAC_AKM_SAE))
            _scanResults.ap_list[idx].security = AUTH_MODE_WPA3;
        else if ((akm & CO_BIT(MAC_AKM_PRE_RSN)) && (akm & CO_BIT(MAC_AKM_PSK))) { // "WPA/WPA2"
            if ((group_cipher & CO_BIT(MAC_CIPHER_CCMP)) && (group_cipher & CO_BIT(MAC_CIPHER_TKIP)))
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA_WPA2;
            else if ((group_cipher & CO_BIT(MAC_CIPHER_CCMP)))
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA_WPA2;
            else if ((group_cipher & CO_BIT(MAC_CIPHER_TKIP)))
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA_WPA2;
            else
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA_WPA2;
        }
        else if (akm & CO_BIT(MAC_AKM_PRE_RSN)) { // "WPA"
            if ((group_cipher & CO_BIT(MAC_CIPHER_CCMP)))
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA;
            else if ((group_cipher & CO_BIT(MAC_CIPHER_TKIP)))
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA;
            else
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA;
        } else { // "WPA2"
            if ((group_cipher & CO_BIT(MAC_CIPHER_CCMP)) && (group_cipher & CO_BIT(MAC_CIPHER_TKIP)))
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA2;
            else if ((group_cipher & CO_BIT(MAC_CIPHER_CCMP)))
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA2;
            else if ((group_cipher & CO_BIT(MAC_CIPHER_TKIP)))
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA2;
            else
                _scanResults.ap_list[idx].security = AUTH_MODE_WPA2;
        }

        sys_memcpy(_scanResults.ap_list[idx].ssid, results->result[idx].ssid.array, results->result[idx].ssid.length);
        _scanResults.ap_list[idx].ssid[results->result[idx].ssid.length] = '\0';
    }

exit:
    // Free temporarily allocated memory
    if (results)
        sys_mfree(results);

    // Update scan completion and running status
    _scanComplete = true;
    _scanRunning = false;

    // Commented-out code: Free AP list memory if needed
    // if (_scanResults.ap_list) {
    //     sys_mfree(_scanResults.ap_list);
    //     _scanResults.ap_list = nullptr;
    //     _scanResults.ap_num = 0;
    // }
}

String WiFiScanClass::SSID(uint8_t i)
{
    if (i >= _scanResults.ap_num) {
        return String();
    }
    return String(_scanResults.ap_list[i].ssid);
}

String WiFiScanClass::BSSID(uint8_t i)
{
    if (i >= _scanResults.ap_num) {
        return String();
    }
    char bssid_str[18];
    snprintf(bssid_str, sizeof(bssid_str), "%02x:%02x:%02x:%02x:%02x:%02x",
             _scanResults.ap_list[i].bssid[0], _scanResults.ap_list[i].bssid[1],
             _scanResults.ap_list[i].bssid[2], _scanResults.ap_list[i].bssid[3],
             _scanResults.ap_list[i].bssid[4], _scanResults.ap_list[i].bssid[5]);
    return String(bssid_str);
}

int32_t WiFiScanClass::RSSI(uint8_t i)
{
    if (i >= _scanResults.ap_num) {
        return 0;
    }
    return (int32_t)_scanResults.ap_list[i].ap_power;
}

uint8_t WiFiScanClass::encryptionType(uint8_t i)
{
    if (i >= _scanResults.ap_num) {
        return 0;
    }
    return _scanResults.ap_list[i].security;
}

int32_t WiFiScanClass::channel(uint8_t i)
{
    if (i >= _scanResults.ap_num) {
        return 0;
    }
    return (int32_t)_scanResults.ap_list[i].channel;
}

void WiFiScanClass::handleScanResults(){
    _processScanResults();
}

int16_t WiFiScanClass::scanComplete(){
    if (_scanComplete)
    {
        return _scanResults.ap_num;
    }

    if (_scanRunning)
    {
        if ((millis() - WiFiScanClass::_scanStartTime) > WiFiScanClass::_scanTimeout) {
            return WIFI_SCAN_FAILED;
        }
        return WIFI_SCAN_RUNNING;
    }

    return WIFI_SCAN_FAILED;
}

void WiFiScanClass::setScanTimeout(uint32_t ms){
    _scanTimeout = ms;
}
