/*
 ESP8266WiFiGeneric.cpp - WiFi library for esp8266

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

#include "gd32vw55x_platform.h"
#include "wifi_netlink.h"
#include "wifi_init.h"
#include "wifi_vif.h"
#include "macif_vif.h"
#include "co_bit.h"
#include "lwip/ip_addr.h"
#include "dhcpd.h"
#include "wifi_management.h"
#include "dhcpd.h"

#include "WiFiGeneric.h"
#include "NetworkManager.h"
WiFiGenericClass::WiFiGenericClass()
    :_wifi_initialized(false)
{
}

WiFiGenericClass::~WiFiGenericClass()
{
}

int WiFiGenericClass::open()
{
    int ret = wifi_netlink_wifi_open();
    return ret;
}

int WiFiGenericClass::close()
{
    wifi_netlink_wifi_close();
    return 0;
}

bool WiFiGenericClass::setPSMode(int vif_index, bool ps_mode)
{

    if(wifi_netlink_ps_mode_set(vif_index, ps_mode ? 1 : 0) != 0){
        return false;
    }

    return true;
}


/**
 * @brief Retrieves the status information of a specified Virtual Interface (VIF)
 *
 * This function gets detailed status information for a specific virtual interface,
 * including MAC address, interface type, connection status, etc., and fills in
 * corresponding specific information according to the interface type (STA or AP).
 *
 * @param vif_info Pointer to a WiFiVifInfo structure where the retrieved status information will be stored
 * @param vif_index The virtual interface index used to specify which interface to query
 * @return int Returns operation result: 0 for success, -1 for failure
 */
int WiFiGenericClass::getStatus(WiFiVifInfo* vif_info, uint8_t vif_index)
{
    if (vif_info == nullptr) {
        return -1;
    }

    // Free any previously allocated client memory
    if (vif_info->ap.clients != nullptr) {
        free(vif_info->ap.clients);
        vif_info->ap.clients = nullptr;
    }

    // Initialize the structure
    memset(vif_info, 0, sizeof(WiFiVifInfo));
    vif_info->index = vif_index;

    // Check if VIF is valid
    if (vif_index >= CFG_VIF_NUM) {
        return -1;
    }

    struct wifi_vif_tag *wvif = &wifi_vif_tab[vif_index];

    if (wvif->mac_vif == nullptr) {
        vif_info->active = false;
        return 0;
    }

    vif_info->active = true;

    // Copy MAC address
    sys_memcpy(vif_info->mac, wvif->mac_addr.array, 6);

    // Get VIF type
    switch (wvif->wvif_type) {
        case WVIF_STA:
            vif_info->type = WVIF_STA;
            break;
        case WVIF_AP:
            vif_info->type = WVIF_AP;
            break;
        case WVIF_MONITOR:
            vif_info->type = WVIF_MONITOR;
            break;
        default:
            vif_info->type = WVIF_UNKNOWN;
            break;
    }

    // Get STA info
    if (vif_info->type == WVIF_STA) {
        // Get STA state
        vif_info->sta.state = wvif->sta.state;

        // Get STA configuration
        if (wvif->sta.state >= WIFI_STA_STATE_CONNECT) {
            strncpy(vif_info->sta.config.ssid, wvif->sta.cfg.ssid, 32);
            vif_info->sta.config.ssid[32] = '\0';
            sys_memcpy(vif_info->sta.config.bssid, wvif->sta.cfg.bssid, 6);
            vif_info->sta.config.channel = wvif->sta.cfg.channel;

            // Get bandwidth
            switch (wvif->sta.cfg.bw) {
                case PHY_CHNL_BW_20:
                    vif_info->sta.config.bw = PHY_CHNL_BW_20;
                    break;
                case PHY_CHNL_BW_40:
                    vif_info->sta.config.bw = PHY_CHNL_BW_40;
                    break;
                case PHY_CHNL_BW_80:
                    vif_info->sta.config.bw = PHY_CHNL_BW_80;
                    break;
                case PHY_CHNL_BW_160:
                    vif_info->sta.config.bw = PHY_CHNL_BW_160;
                    break;
                case PHY_CHNL_BW_80P80:
                    vif_info->sta.config.bw = PHY_CHNL_BW_80P80;
                    break;
                default:
                    vif_info->sta.config.bw = PHY_CHNL_BW_OTHER;
                    break;
            }

            // Get security type
            if (wvif->sta.cfg.akm & CO_BIT(MAC_AKM_SAE)) {
                vif_info->sta.config.security = AUTH_MODE_WPA3;
            } else if (wvif->sta.cfg.akm == CO_BIT(MAC_AKM_PRE_RSN)) {
                vif_info->sta.config.security = AUTH_MODE_WEP;
            } else if (wvif->sta.cfg.akm == (CO_BIT(MAC_AKM_PSK) | CO_BIT(MAC_AKM_PRE_RSN))) {
                vif_info->sta.config.security = AUTH_MODE_WPA;
            } else if (wvif->sta.cfg.akm == CO_BIT(MAC_AKM_PSK)) {
                vif_info->sta.config.security = AUTH_MODE_WPA2;
            } else if (wvif->sta.cfg.akm == CO_BIT(MAC_AKM_PSK_SHA256)) {
                vif_info->sta.config.security = AUTH_MODE_WPA2;
            } else if (wvif->sta.cfg.akm == CO_BIT(MAC_AKM_NONE)) {
                vif_info->sta.config.security = AUTH_MODE_OPEN;
            } else if (wvif->sta.cfg.akm == CO_BIT(MAC_AKM_8021X)) {
                vif_info->sta.config.security = AUTH_MODE_WPA2;
            } else if (wvif->sta.cfg.akm == CO_BIT(MAC_AKM_8021X_SHA256)) {
                vif_info->sta.config.security = AUTH_MODE_WPA2;
            } else if (wvif->sta.cfg.akm == CO_BIT(MAC_AKM_8021X_SUITE_B)) {
                vif_info->sta.config.security = AUTH_MODE_WPA2;
            } else if (wvif->sta.cfg.akm == CO_BIT(MAC_AKM_8021X_SUITE_B_192)) {
                vif_info->sta.config.security = AUTH_MODE_WPA2;
            } else if (wvif->sta.cfg.akm == CO_BIT(MAC_AKM_OWE)) {
                vif_info->sta.config.security = AUTH_MODE_WPA3;
            } else {
                vif_info->sta.config.security = AUTH_MODE_UNKNOWN;
            }

            // Get wireless mode and RSSI
            vif_info->sta.mode = macif_vif_wireless_mode_get(vif_index);
            vif_info->sta.rssi = macif_vif_sta_rssi_get(vif_index);
        }

        // Get IP info
        if (wvif->sta.state >= WIFI_STA_STATE_IP_GETTING) {
            struct wifi_ip_addr_cfg ip_cfg;
            if (wifi_get_vif_ip(vif_index, &ip_cfg) == 0) {
                vif_info->sta.ip_info.addr = ip_cfg.ipv4.addr;
                vif_info->sta.ip_info.mask = ip_cfg.ipv4.mask;
                vif_info->sta.ip_info.gw = ip_cfg.ipv4.gw;
                vif_info->sta.ip_info.valid = true;
            }

#ifdef CONFIG_IPV6_SUPPORT
            if (wifi_get_vif_ip6(vif_index, vif_info->sta.ipv6_info.local,
                                   vif_info->sta.ipv6_info.unique) == 0) {
                vif_info->sta.ipv6_info.valid = true;
            }
#endif
        }
    }

    // Get AP info
    if (vif_info->type == WVIF_AP) {
        // Get AP state
        vif_info->ap.state = wvif->ap.ap_state;

        if (vif_info->ap.state != WIFI_AP_STATE_STARTED) {
            return 0;
        }

        // Get AP configuration
        strncpy(vif_info->ap.config.ssid, wvif->ap.cfg.ssid, 32);
        vif_info->ap.config.ssid[32] = '\0';
        vif_info->ap.config.channel = wvif->ap.cfg.channel;

        // Get security type
        if (wvif->ap.cfg.akm == CO_BIT(MAC_AKM_NONE)) {
            vif_info->ap.config.security = AUTH_MODE_OPEN;
        } else if (wvif->ap.cfg.akm == (CO_BIT(MAC_AKM_PSK) | CO_BIT(MAC_AKM_PRE_RSN))) {
            vif_info->ap.config.security = AUTH_MODE_WPA;
        } else if (wvif->ap.cfg.akm == CO_BIT(MAC_AKM_PSK)) {
            vif_info->ap.config.security = AUTH_MODE_WPA2;
        } else if (wvif->ap.cfg.akm == CO_BIT(MAC_AKM_SAE)) {
            vif_info->ap.config.security = AUTH_MODE_WPA3;
        } else if (wvif->ap.cfg.akm == (CO_BIT(MAC_AKM_PSK) | CO_BIT(MAC_AKM_SAE))) {
            vif_info->ap.config.security = AUTH_MODE_WPA2_WPA3;
        } else {
            vif_info->ap.config.security = AUTH_MODE_UNKNOWN;
        }

        vif_info->ap.config.he_disabled = wvif->ap.cfg.he_disabled;

        // Get wireless mode
        if (wvif->ap.cfg.he_disabled) {
            vif_info->ap.mode = WIRELESS_MODE_11BGN;
        } else {
            vif_info->ap.mode = WIRELESS_MODE_11BGN_AX;
        }

        // Get IP info
        struct wifi_ip_addr_cfg ip_cfg;
        if (wifi_get_vif_ip(vif_index, &ip_cfg) == 0) {
            vif_info->ap.ip_info.addr = ip_cfg.ipv4.addr;
            vif_info->ap.ip_info.mask = ip_cfg.ipv4.mask;
            vif_info->ap.ip_info.gw = ip_cfg.ipv4.gw;
            vif_info->ap.ip_info.valid = true;
        }

#ifdef CONFIG_IPV6_SUPPORT
        if (wifi_get_vif_ip6(vif_index, vif_info->ap.ipv6_info.local,
                               vif_info->ap.ipv6_info.unique) == 0) {
            vif_info->ap.ipv6_info.valid = true;
        }
#endif

        // Get associated clients
        struct mac_addr cli_mac[CFG_STA_NUM];
        int cli_num = macif_vif_ap_assoc_info_get(vif_index, (uint16_t *)&cli_mac);
        vif_info->ap.client_count = cli_num;

        // Allocate memory for clients array
        if (cli_num > 0) {
            vif_info->ap.clients = (WiFiApClient *)malloc(cli_num * sizeof(WiFiApClient));
            if (vif_info->ap.clients == nullptr) {
                vif_info->ap.client_count = 0;
                return 0;
            }
        }

        for (int i = 0; i < vif_info->ap.client_count; i++) {
            // Copy MAC address byte by byte (cli_mac.array is uint16_t[3])
            vif_info->ap.clients[i].mac[0] = cli_mac[i].array[0] & 0xFF;
            vif_info->ap.clients[i].mac[1] = cli_mac[i].array[0] >> 8;
            vif_info->ap.clients[i].mac[2] = cli_mac[i].array[1] & 0xFF;
            vif_info->ap.clients[i].mac[3] = cli_mac[i].array[1] >> 8;
            vif_info->ap.clients[i].mac[4] = cli_mac[i].array[2] & 0xFF;
            vif_info->ap.clients[i].mac[5] = cli_mac[i].array[2] >> 8;

            uint32_t cli_ipaddr = dhcpd_find_ipaddr_by_macaddr((uint8_t *)cli_mac[i].array);
            vif_info->ap.clients[i].ip = cli_ipaddr;
            vif_info->ap.clients[i].ip_known = (cli_ipaddr != 0);
        }
    }

    return 0;
}

wifi_mode_t WiFiGenericClass::getMode() {
    wifi_mode_t mode;
    if(wifi_vif_tab[WIFI_VIF_INDEX_DEFAULT].wvif_type ==  WVIF_UNKNOWN &&
       wifi_vif_tab[WIFI_VIF_INDEX_SOFTAP_MODE].wvif_type == WVIF_UNKNOWN) {
        mode = WIFI_OFF;
    } else if(wifi_vif_tab[WIFI_VIF_INDEX_DEFAULT].wvif_type == WVIF_AP) {
        mode = WIFI_AP;
    } else if(wifi_vif_tab[WIFI_VIF_INDEX_DEFAULT].wvif_type == WVIF_STA) {
        mode = WIFI_STA;
    } else if(wifi_vif_tab[WIFI_VIF_INDEX_STA_MODE].wvif_type == WVIF_STA &&
              wifi_vif_tab[WIFI_VIF_INDEX_SOFTAP_MODE].wvif_type == WVIF_AP) {
        mode = WIFI_AP_STA;
    } else {
        mode = WIFI_OFF;
    }
    return mode;
}

wifi_event_id_t WiFiGenericClass::onEvent(WiFiEventCb cbEvent, arduino_event_id_t event) {
    return Network.onEvent(cbEvent, event);
}

wifi_event_id_t WiFiGenericClass::onEvent(WiFiEventFuncCb cbEvent, arduino_event_id_t event) {
    return Network.onEvent(cbEvent, event);
}

wifi_event_id_t WiFiGenericClass::onEvent(WiFiEventSysCb cbEvent, arduino_event_id_t event) {
    return Network.onEvent(cbEvent, event);
}

void WiFiGenericClass::removeEvent(WiFiEventCb cbEvent, arduino_event_id_t event) {
    Network.removeEvent(cbEvent, event);
}

void WiFiGenericClass::removeEvent(WiFiEventSysCb cbEvent, arduino_event_id_t event) {
    Network.removeEvent(cbEvent, event);
}

void WiFiGenericClass::removeEvent(wifi_event_id_t id) {
    Network.removeEvent(id);
}

String WiFiGenericClass::modeName(wifi_mode_t mode)
{
    switch(mode) {
        case WIFI_OFF:
            return "WIFI_OFF";
        case WIFI_STA:
            return "WIFI_STA";
        case WIFI_AP:
            return "WIFI_AP";
        case WIFI_AP_STA:
            return "WIFI_AP_STA";
        default:
            return "WIFI_UNKNOWN";
    }
}