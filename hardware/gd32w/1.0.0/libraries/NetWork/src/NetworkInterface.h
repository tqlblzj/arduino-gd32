/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2025 GigaDevice Semiconductor Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H
#include "Arduino.h"
#include "wifi_vif.h"
#include "netif.h"

class NetworkInterface
{
private:

public:
    NetworkInterface();
    ~NetworkInterface();

    // netif* netif;
    int vif_idx;

    uint8_t * macAddress(uint8_t *mac, uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);
    String macAddress(uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);

    bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0x00000000, IPAddress dns2 = (uint32_t)0x00000000, uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);
    bool getConfig(IPAddress* local_ip, IPAddress* gateway, IPAddress* subnet, IPAddress* dns, uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);
    bool setDNS(IPAddress dns, uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);
    bool getDNS(IPAddress* dns, uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);
    int getSocket(uint16_t ethertype, uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);
    IPAddress localIP(uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);
    IPAddress subnetMask(uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);
    IPAddress gatewayIP(uint8_t vif_idx = WIFI_VIF_INDEX_DEFAULT);
protected:

};


#endif // NETWORK_INTERFACE_H