/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2025 GigaDevice Semiconductor Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "NetworkManager.h"
#include "IPAddress.h"
#include "lwip/dns.h"
#include "netdb.h"

NetworkManager::NetworkManager() {}

bool NetworkManager::begin() {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        return initNetworkEvents();
    }
    return initialized;
}

// TODO
int NetworkManager::hostByName(const char *aHostname, IPAddress &aResult) {
    return 1;
}

uint8_t *NetworkManager::macAddress(uint8_t *mac) {
    struct mac_addr mac_addr;
    macif_ctl_base_addr_get(&mac_addr);
    memcpy(mac, mac_addr.array, 6);
    return mac;
}

String NetworkManager::macAddress(void) {
    uint8_t mac[6];
    char macStr[18] = {0};
    macAddress(mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

// TODO
static char default_hostname[32] = {
  0,
};

const char *NetworkManager::getHostname() {
    return (const char *)default_hostname;
}

bool NetworkManager::setHostname(const char *name) {
    if (name) {
        snprintf(default_hostname, 32, "%s", name);
    }
    return true;
}

NetworkManager Network;
