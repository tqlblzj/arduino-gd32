/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2025 GigaDevice Semiconductor Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "NetworkInterface.h"
#include "netif.h"
#include "wifi_net_ip.h"
#include "wifi_netif.h"

NetworkInterface::NetworkInterface()
{
}

NetworkInterface::~NetworkInterface()
{
}

uint8_t* NetworkInterface::macAddress(uint8_t* mac, uint8_t vif_idx)
{
    struct netif *netif = (struct netif *)vif_idx_to_net_if(vif_idx);
    if (netif == nullptr) {
        return nullptr;
    }

    if (netif != nullptr) {
        sys_memcpy(mac, netif->hwaddr, MAC_ADDR_LEN);
    } else {
        memset(mac, 0, MAC_ADDR_LEN);
    }

    return mac;
}

String NetworkInterface::macAddress(uint8_t vif_idx)
{
    uint8_t mac_raw[MAC_ADDR_LEN] = {0};
    char mac_str[18];  // "AA:BB:CC:DD:EE:FF" + '\0'

    struct netif *netif = (struct netif *)vif_idx_to_net_if(vif_idx);
    if (netif != nullptr) {
        sys_memcpy(mac_raw, netif->hwaddr, MAC_ADDR_LEN);
    }

    snprintf(mac_str, sizeof(mac_str),
            "%02X:%02X:%02X:%02X:%02X:%02X",
            mac_raw[0], mac_raw[1], mac_raw[2],
            mac_raw[3], mac_raw[4], mac_raw[5]);

    return String(mac_str);
}

bool NetworkInterface::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2, uint8_t vif_idx)
{
    if (local_ip == INADDR_NONE) {
        struct wifi_ip_addr_cfg cfg;

        net_if_use_static_ip(false);
        cfg.mode = IP_ADDR_NONE;
        wifi_set_vif_ip(vif_idx, &cfg);

        cfg.mode = IP_ADDR_DHCP_CLIENT;
        cfg.dhcp.to_ms = 10000;
        cfg.default_output = true;

        if (wifi_set_vif_ip(vif_idx, &cfg) != 0) {
            return false;
        }

        return true;
    }

    if (local_ip != INADDR_NONE && subnet[0] != 255) {
        IPAddress tmp = dns1;
        dns1 = gateway;
        gateway = subnet;
        subnet = (tmp != INADDR_NONE) ? tmp : IPAddress(255, 255, 255, 0);
    }

    struct wifi_ip_addr_cfg cfg;
    cfg.mode = IP_ADDR_STATIC_IPV4;
    cfg.ipv4.addr = local_ip;
    cfg.ipv4.mask = subnet;
    cfg.ipv4.gw = gateway;
    cfg.ipv4.dns = dns1;
    cfg.default_output = false;

    net_if_use_static_ip(true);

    if (wifi_set_vif_ip(vif_idx, &cfg) != 0) {
        return false;
    }

    return true;

}

bool NetworkInterface::getConfig(IPAddress* local_ip, IPAddress* gateway, IPAddress* subnet, IPAddress* dns, uint8_t vif_idx){

    if (!local_ip || !gateway || !subnet || !dns) {
        return false;
    }

    struct wifi_ip_addr_cfg cfg;
    if (wifi_get_vif_ip(vif_idx, &cfg) != 0) {
        return false;
    }

    *local_ip = IPAddress(cfg.ipv4.addr);
    *gateway = IPAddress(cfg.ipv4.gw);
    *subnet = IPAddress(cfg.ipv4.mask);
    *dns = IPAddress(cfg.ipv4.dns);

    return true;
}

bool NetworkInterface::setDNS(IPAddress dns, uint8_t vif_idx)
{
    if (dns == INADDR_NONE) {
        return false;
    }

    if (net_set_dns((uint32_t)dns) != 0) {
        return false;
    }

    return true;
}

bool NetworkInterface::getDNS(IPAddress* dns, uint8_t vif_idx)
{
    if (!dns) {
        return false;
    }

    uint32_t dns_server;
    if (net_get_dns(&dns_server) != 0) {
        return false;
    }

    *dns = IPAddress(dns_server);

    return true;
}

// TODO
int NetworkInterface::getSocket(uint16_t ethertype, uint8_t vif_idx)
{

}

IPAddress NetworkInterface::localIP(uint8_t vif_idx)
{
    struct netif *netif = (struct netif *)vif_idx_to_net_if(vif_idx);
    if (netif == nullptr) {
        return IPAddress((uint32_t)0);
    }

    uint32_t ip, mask, gw;
    if (net_if_get_ip(netif, &ip, &mask, &gw) != 0) {
        return IPAddress((uint32_t)0);
    }

    return IPAddress(ip);
}

IPAddress NetworkInterface::subnetMask(uint8_t vif_idx)
{
    struct netif *netif = (struct netif *)vif_idx_to_net_if(vif_idx);
    if (netif == nullptr) {
        return IPAddress((uint32_t)0);
    }

    uint32_t ip, mask, gw;
    if (net_if_get_ip(netif, &ip, &mask, &gw) != 0) {
        return IPAddress((uint32_t)0);
    }

    return IPAddress(mask);
}

IPAddress NetworkInterface::gatewayIP(uint8_t vif_idx)
{
    struct netif *netif = (struct netif *)vif_idx_to_net_if(vif_idx);
    if (netif == nullptr) {
        return IPAddress((uint32_t)0);
    }

    uint32_t ip, mask, gw;
    if (net_if_get_ip(netif, &ip, &mask, &gw) != 0) {
        return IPAddress((uint32_t)0);
    }

    return IPAddress(gw);
}
