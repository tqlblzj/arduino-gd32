/*
 ESP8266WiFiGeneric.h - esp8266 Wifi support.
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


#ifndef WIFIGENERIC_H
#define WIFIGENERIC_H

#include "stdint.h"
#include "stdio.h"
#include "wifi_netlink.h"
#include "wifi_net_ip.h"
#include "macif_vif.h"
#include "wifi_vif.h"
#include "wifi_management.h"
#include "WiFiType.h"
#include "WString.h"
#include "NetworkEvents.h"
#include "NetworkManager.h"
#include "_build_date.h"
#include "_build_version.h"

// IPv6 address info
struct WiFiIPv6Info {
    char local[40];
    char unique[40];
    bool valid;
};

// IP address info
struct WiFiIPInfo {
    uint32_t addr;
    uint32_t mask;
    uint32_t gw;
    bool valid;
};

// AP client info
struct WiFiApClient {
    uint8_t mac[6];
    uint32_t ip;
    bool ip_known;
};

// STA configuration info
struct WiFiStaConfig {
    char ssid[33];
    uint8_t bssid[6];
    uint8_t channel;
    mac_chan_bandwidth bw;
    wifi_ap_auth_mode_t security;
};

// STA info
struct WiFiStaInfo {
    wvif_sta_state state;
    WiFiStaConfig config;
    int8_t rssi;
    wifi_wireless_mode mode;
    WiFiIPInfo ip_info;
    WiFiIPv6Info ipv6_info;
};

// AP configuration info
struct WiFiApConfig {
    char ssid[33];
    uint8_t channel;
    wifi_ap_auth_mode_t security;
    bool he_disabled;
};

// AP info
struct WiFiApInfo {
    wvif_ap_state state;
    WiFiApConfig config;
    wifi_wireless_mode mode;
    WiFiIPInfo ip_info;
    WiFiIPv6Info ipv6_info;
    uint8_t client_count;
    WiFiApClient *clients;
};

// VIF info
struct WiFiVifInfo {
    uint8_t index;
    bool active;
    wifi_vif_type type;
    uint8_t mac[6];
    union {
        WiFiStaInfo sta;
        WiFiApInfo ap;
    };
};

#define WiFiEventCb     NetworkEventCb
#define WiFiEventFuncCb NetworkEventFuncCb
#define WiFiEventSysCb  NetworkEventSysCb
#define wifi_event_id_t network_event_handle_t

class WiFiGenericClass
{
private:

public:
    WiFiGenericClass();
    ~WiFiGenericClass();

    wifi_event_id_t onEvent(WiFiEventCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);
    wifi_event_id_t onEvent(WiFiEventFuncCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);
    wifi_event_id_t onEvent(WiFiEventSysCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);
    void removeEvent(WiFiEventCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);
    void removeEvent(WiFiEventSysCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);
    void removeEvent(wifi_event_id_t id);

    int open();
    int close();

    // Get WiFi status info
    int getStatus(WiFiVifInfo* vif_info, uint8_t vif_index = 0);

    // Free memory allocated in WiFiVifInfo
    static void freeVifInfo(WiFiVifInfo* vif_info) {
        if (vif_info != nullptr && vif_info->ap.clients != nullptr) {
            sys_mfree(vif_info->ap.clients);
            vif_info->ap.clients = nullptr;
        }
    }

    int get_ap_status(WiFiVifInfo* vif_info){
        int vif_idx = WIFI_VIF_INDEX_DEFAULT;
        if (wifi_management_concurrent_get())
        {
            vif_idx = WIFI_VIF_INDEX_SOFTAP_MODE;
        }

        return getStatus(vif_info, vif_idx);
    }

    int get_sta_status(WiFiVifInfo* vif_info){
        return getStatus(vif_info, WIFI_VIF_INDEX_DEFAULT);
    }

    bool setPSMode(int vif_index = WIFI_VIF_INDEX_DEFAULT, bool ps_mode = true);
    // static bool mode(wifi_mode_t);
    static wifi_mode_t getMode();
    String modeName(wifi_mode_t mode);
    String eventName(arduino_event_id_t id){
        return Network.eventName(id);
    }

    String getSDKVersion(){
        return WIFI_GIT_REVISION;
    }

    String getSDKBuildDate(){
        return SDK_BUILD_DATE;
    }

private:
    bool _wifi_initialized;
};

#endif // WIFIGENERIC_H