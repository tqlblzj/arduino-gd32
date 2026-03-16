/*
 ESP8266WiFiSTA.cpp - WiFi library for esp8266

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

#include "WiFiAP.h"
#include "wifi_management.h"
#include "wifi_net_ip.h"
#include "wifi_vif.h"
#include "wifi_eloop.h"
#include "WiFi.h"
#include "NetworkEvents.h"
#include "NetworkManager.h"

static void cb_client_added(void *eloop_data, void *user_ctx);
static void cb_client_removed(void *eloop_data, void *user_ctx);

APClass::APClass()
    : _ap_initialized(false)
{
}

APClass::~APClass()
{
}

bool APClass::begin()
{
    if (WiFi.open() != 0) {
        printf("[WiFiAP] Failed to open WiFi.\r\n");
        return false;
    }

    eloop_event_register(WIFI_MGMT_EVENT_CLIENT_DHCP_OFFER, cb_client_added, NULL, NULL);
    eloop_event_register(WIFI_MGMT_EVENT_CLIENT_REMOVED, cb_client_removed, NULL, NULL);

    arduino_event_t arduino_event;
    arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_INIT;
    Network.postEvent(&arduino_event);

    return true;
}

bool APClass::end()
{
    eloop_event_unregister(WIFI_MGMT_EVENT_CLIENT_ADDED);
    eloop_event_unregister(WIFI_MGMT_EVENT_CLIENT_REMOVED);

    if (wifi_management_ap_stop() != 0) {
        printf("[WiFiAP] Failed to stop WiFi AP.\r\n");
        return false;
    }

    arduino_event_t arduino_event;
    arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_STOP_AP_CMD;
    Network.postEvent(&arduino_event);

    return true;
}

/**
 * @brief Create WiFi AP with specified settings
 *
 * Starts a WiFi access point with the specified SSID, password, channel,
 * authentication mode, and SSID visibility.
 * Posts ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD event.
 *
 * @param ssid: SSID of the AP
 * @param passphrase: Password for the AP (empty for open network)
 * @param channel: WiFi channel (1-14)
 * @param auth_mode: Authentication mode (0=OPEN, 1=WEP, 2=WPA_PSK, 3=WPA2_PSK, 4=.....)
 * @param ssid_hidden: 1 to hide SSID, 0 to broadcast
 * @return true if AP started successfully, false on failure
 */
bool APClass::create(
    char *ssid,
    char *passphrase,
    int channel,
    int auth_mode,
    int ssid_hidden)
{
    if (wifi_management_ap_start(ssid, passphrase, (uint32_t)channel, (wifi_ap_auth_mode_t)auth_mode, (uint32_t)ssid_hidden) != 0) {
        printf("[WiFiAP] Failed to start WiFi AP.\r\n");
        return false;
    }

    printf("[WiFiAP] WiFi AP started: SSID=%s, Channel=%d\r\n", ssid, channel);

    arduino_event_t arduino_event;
    arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD;
    Network.postEvent(&arduino_event);

    return true;
}


bool APClass::clear()
{
    return end();
}

String APClass::SSID(void) const
{
    // Get VIF (Virtual Interface) structure
    uint8_t vif_index = WIFI_VIF_INDEX_DEFAULT;
    if(wifi_management_concurrent_get()){
        vif_index = WIFI_VIF_INDEX_SOFTAP_MODE;
    }

    struct wifi_vif_tag *vif = (struct wifi_vif_tag *)vif_idx_to_wvif(vif_index);
    if (!vif || vif->wvif_type != WVIF_AP) {
        return String("");
    }

    // Return SSID from AP configuration
    struct wifi_ap *ap = &vif->ap;
    return String(ap->cfg.ssid);
}

uint8_t APClass::stationCount()
{
    uint8_t vif_index = WIFI_VIF_INDEX_DEFAULT;
    if(wifi_management_concurrent_get()){
        vif_index = WIFI_VIF_INDEX_SOFTAP_MODE;
    }

    // Get VIF (Virtual Interface) structure
    struct wifi_vif_tag *vif = (struct wifi_vif_tag *)vif_idx_to_wvif(vif_index);
    if (!vif || vif->wvif_type != WVIF_AP) {
        return 0;
    }

    struct wifi_ap *ap = &vif->ap;
    struct mac_addr cli_mac[CFG_STA_NUM];
    uint8_t count = macif_vif_ap_assoc_info_get(vif_index, (uint16_t *)&cli_mac);
    return count;
}

WiFiAPClass::WiFiAPClass()
{
}

WiFiAPClass::~WiFiAPClass()
{
}

/**
 * @brief Start soft AP (access point) mode
 *
 * Initializes and starts a WiFi access point with the specified settings.
 *
 * @param ssid: SSID of the AP
 * @param passphrase: Password for the AP (empty for open network)
 * @param channel: WiFi channel (1-14)
 * @param auth_mode: Authentication mode (0=OPEN, 1=WEP, 2=WPA_PSK, 3=WPA2_PSK, 4=.....)
 * @param ssid_hidden: 1 to hide SSID, 0 to broadcast
 * @return true if AP started successfully, false on failure
 */
bool WiFiAPClass::softAP(
    char *ssid,
    char *passphrase,
    int channel,
    int auth_mode,
    int ssid_hidden)
{
    // Initialize AP mode
    if (!AP.begin()) {
        return false;
    }

    sys_ms_sleep(5);

    // Create AP with specified configuration
    if(AP.create(ssid, passphrase, channel, auth_mode, ssid_hidden) != true){
        return false;
    }

    return true;
}

/**
 * @brief Configure soft AP with static IP settings
 *
 * Sets static IP configuration for the soft AP interface, including
 * local IP, gateway, subnet mask, and DNS server.
 *
 * @param local_ip: Local IP address to set
 * @param gateway: Gateway IP address
 * @param subnet: Subnet mask
 * @param dhcp_lease_start: DHCP lease start address
 * @param dns: DNS server address
 * @return true if configuration succeeded, false on failure
 */
bool WiFiAPClass::softAPConfig(
    IPAddress local_ip,
    IPAddress gateway,
    IPAddress subnet,
    IPAddress dhcp_lease_start,  // not used currently
    IPAddress dns)
{
    if (!AP.begin()) {
        return false;
    }

    uint8_t vif_index = WIFI_VIF_INDEX_DEFAULT;
    if(wifi_management_concurrent_get()){
        vif_index = WIFI_VIF_INDEX_SOFTAP_MODE;
    }

    struct wifi_ip_addr_cfg cfg;
    cfg.mode = IP_ADDR_DHCP_SERVER;
    cfg.ipv4.addr = local_ip;
    cfg.ipv4.mask = subnet;
    cfg.ipv4.gw = gateway;
    cfg.ipv4.dns = dns;
    cfg.default_output = true;

    if (wifi_set_vif_ip(vif_index, &cfg) != 0) {
        printf("[WiFiAP] Failed to set AP static IP configuration.\r\n");
        return false;
    }

    printf("[WiFiAP] AP Static IP configured: IP=%s, GW=%s, Mask=%s, DNS=%s\r\n",
           local_ip.toString().c_str(),
           gateway.toString().c_str(),
           subnet.toString().c_str(),
           dns.toString().c_str());

    return true;
}

bool WiFiAPClass::softAPdisconnect(bool wifioff)
{
    if (wifioff) {
        return AP.end();
    }
    return AP.clear();
}

uint8_t WiFiAPClass::softAPgetStationNum()
{
    return AP.stationCount();
}

String WiFiAPClass::softAPSSID(void) const
{
    return AP.SSID();
}

IPAddress WiFiAPClass::softAPIP()
{
    int vif_idx = WIFI_VIF_INDEX_DEFAULT;
    if(wifi_management_concurrent_get()){
        vif_idx = WIFI_VIF_INDEX_SOFTAP_MODE;
    }
    return AP.localIP(vif_idx);
}

IPAddress WiFiAPClass::softAPBroadcastIP()
{
    int vif_idx = WIFI_VIF_INDEX_DEFAULT;
    if(wifi_management_concurrent_get()){
        vif_idx = WIFI_VIF_INDEX_SOFTAP_MODE;
    }

    struct wifi_vif_tag *vif = (struct wifi_vif_tag *)vif_idx_to_wvif(vif_idx);
    if (!vif || vif->wvif_type != WVIF_AP) {
        return IPAddress(0, 0, 0, 0);
    }

    struct netif *netif = (struct netif *)vif_idx_to_net_if(vif_idx);
    if (!netif) {
        return IPAddress(0, 0, 0, 0);
    }

    return IPAddress(netif->ip_addr.addr | netif->netmask.addr);
}

IPAddress WiFiAPClass::softAPNetworkID()
{
    int vif_idx = WIFI_VIF_INDEX_DEFAULT;
    if(wifi_management_concurrent_get()){
        vif_idx = WIFI_VIF_INDEX_SOFTAP_MODE;
    }

    struct wifi_vif_tag *vif = (struct wifi_vif_tag *)vif_idx_to_wvif(vif_idx);
    if (!vif || vif->wvif_type != WVIF_AP) {
        return IPAddress(0, 0, 0, 0);
    }

    struct netif *netif = (struct netif *)vif_idx_to_net_if(vif_idx);
    if (!netif) {
        return IPAddress(0, 0, 0, 0);
    }

    return IPAddress(netif->ip_addr.addr & netif->netmask.addr);
}

IPAddress WiFiAPClass::softAPSubnetMask()
{
    int vif_idx = WIFI_VIF_INDEX_DEFAULT;
    if(wifi_management_concurrent_get()){
        vif_idx = WIFI_VIF_INDEX_SOFTAP_MODE;
    }

    struct wifi_vif_tag *vif = (struct wifi_vif_tag *)vif_idx_to_wvif(vif_idx);
    if (!vif || vif->wvif_type != WVIF_AP) {
        return IPAddress(0, 0, 0, 0);
    }

    struct netif *netif = (struct netif *)vif_idx_to_net_if(vif_idx);
    if (!netif) {
        return IPAddress(0, 0, 0, 0);
    }

    return AP.subnetMask(vif_idx);
}

uint8_t WiFiAPClass::softAPSubnetCIDR()
{

}

uint8_t *WiFiAPClass::softAPmacAddress(uint8_t *mac)
{
    int vif_idx = WIFI_VIF_INDEX_DEFAULT;
    if(wifi_management_concurrent_get()){
        vif_idx = WIFI_VIF_INDEX_SOFTAP_MODE;
    }
    return AP.macAddress(mac, vif_idx);
}

String WiFiAPClass::softAPmacAddress(void)
{
    int vif_idx = WIFI_VIF_INDEX_DEFAULT;
    if(wifi_management_concurrent_get()){
        vif_idx = WIFI_VIF_INDEX_SOFTAP_MODE;
    }
    return AP.macAddress(vif_idx);
}

static void cb_client_added(void *eloop_data, void *user_ctx)
{
    arduino_event_t arduino_event;
    arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_CLIENT_DHCP_OFFER;
    Network.postEvent(&arduino_event);
}

static void cb_client_removed(void *eloop_data, void *user_ctx)
{
    arduino_event_t arduino_event;
    arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED;
    Network.postEvent(&arduino_event);
}

void WiFiAPClass::_onApEvent(int32_t event_id, void *event_data)
{
    // Handle AP events
    // TODO: Implement event handling

}
