/*
 WiFiSTA.cpp - WiFi library for esp32

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

#include "WiFiSTA.h"
#include "wifi_management.h"
#include "wifi_net_ip.h"
#include "wifi_vif.h"
#include "WiFi.h"
#include "NetworkEvents.h"
#include "netif.h"
#include "NetworkInterface.h"
#include "NetworkEvents.h"
#include "NetworkManager.h"
#include "IPAddress.h"

STAClass::STAClass()
    :status(WL_STOPPED), _sta_initialized(false)
{
}

STAClass::~STAClass()
{
}

bool STAClass::begin()
{
    // Initialize WiFi hardware if not already initialized
    if(!_sta_initialized){
        if(WiFi.open() != 0) {
            return false;
        }
        arduino_event_t arduino_event;
        arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_INIT;
        Network.postEvent(&arduino_event);

        // Start STA mode in WiFi management layer
        if(wifi_management_sta_start() != 0){
            return false;
        }
        _sta_initialized = true;
        printf("WiFi STA initialized.\r\n");
    }
    else{
        printf("WiFi STA is already initialized.\r\n");
    }
    status = WL_IDLE_STATUS;
    return true;
}

bool STAClass::end(bool wifi_off)
{
    // Turn off WiFi hardware if requested
    if(wifi_off){
        if(status != WL_STOPPED && status != WL_NO_SHIELD)
        {
            if(WiFi.close() != 0) {
                return false;
            }
        }
    }

    _sta_initialized = false;
    status = WL_STOPPED;
    return true;
}

static void _sta_event_cb(void *arg, void *eloop_data, void *user_ctx) {

}

void STAClass::_onStaEvent(void *eloop_data, void *user_ctx) {
}

bool STAClass::onEnable()
{

}

WiFiSTAClass::WiFiSTAClass()
{
}

WiFiSTAClass::~WiFiSTAClass()
{
}

static void cb_connect_success(void *eloop_data, void *user_ctx)
{
    WiFi.handleConnectResults(true);
    eloop_event_unregister(WIFI_MGMT_EVENT_CONNECT_SUCCESS);
    eloop_event_unregister(WIFI_MGMT_EVENT_CONNECT_FAIL);
}

static void cb_connect_fail(void *eloop_data, void *user_ctx){
    WiFi.handleConnectResults(false);
    eloop_event_unregister(WIFI_MGMT_EVENT_CONNECT_SUCCESS);
    eloop_event_unregister(WIFI_MGMT_EVENT_CONNECT_FAIL);
}

wl_status_t WiFiSTAClass::begin(char* ssid, char* password, uint8_t blocked)
{
    // Initialize WiFi STA mode if not already initialized
    if(STA.begin() != true){
        STA.status = WL_CONNECT_FAILED;
        return WL_CONNECT_FAILED;
    }

    sys_ms_sleep(10);
    // printf("Connecting to %s\n", ssid);

    // Initiate connection to WiFi network
    if(wifi_management_connect(ssid, password, blocked) != 0){
        // printf("WiFi connect failed.\n");
        STA.status = WL_CONNECT_FAILED;
        return WL_CONNECT_FAILED;
    }

    // Handle non-blocking mode
    if(!blocked)
    {
        // Register callbacks for connection success/failure events
        eloop_event_register(WIFI_MGMT_EVENT_CONNECT_SUCCESS, cb_connect_success, NULL, NULL);
        eloop_event_register(WIFI_MGMT_EVENT_CONNECT_FAIL, cb_connect_fail, NULL, NULL);
        STA.status = WL_CONNECT_ONGOING;
        return WL_CONNECT_ONGOING;
    }

    // Handle blocking mode - process results immediately
    _processConnectResults(true);

    return WL_CONNECTED;
}

wl_status_t WiFiSTAClass::status()
{
    return STA.status;
}

bool WiFiSTAClass::disconnect()
{
    arduino_event_t arduino_event;
    // Request disconnection from WiFi management layer
    if(wifi_management_disconnect() != 0){
        STA.status = WL_CONNECT_FAILED;
        return false;
    }
    STA.status = WL_DISCONNECTED;

    arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_DISCONNECT;
    Network.postEvent(&arduino_event);

    return true;
}

void WiFiSTAClass::_processConnectResults(bool success)
{
    arduino_event_t arduino_event;
    // Handle connection failure
    if(success == false){
        STA.status = WL_CONNECT_FAILED;
        arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_CONNECT_FAIL;
        Network.postEvent(&arduino_event);
        return;
    }
    // Handle connection success
    arduino_event.event_id = ARDUINO_WIFI_MGMT_EVENT_CONNECT_SUCCESS;
    Network.postEvent(&arduino_event);
    STA.status = WL_CONNECTED;
    return;
}

String WiFiSTAClass::macAddress()
{
    int vif_idx = WIFI_VIF_INDEX_DEFAULT;
    return STA.macAddress(vif_idx);
}

uint8_t* WiFiSTAClass::macAddress(uint8_t* mac)
{
    return STA.macAddress(mac, WIFI_VIF_INDEX_DEFAULT);
}

bool WiFiSTAClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {

    // Handle case where local_ip is not specified
    if (local_ip == (IPAddress)INADDR_NONE) {
        return STA.begin() && STA.config(local_ip, gateway, subnet, dns1, dns2, WIFI_VIF_INDEX_DEFAULT);
    }

    // Handle legacy parameter order (local_ip, dns, gateway, subnet)
    if (local_ip != (IPAddress)INADDR_NONE && subnet[0] != 255) {
        IPAddress tmp = dns1;
        dns1 = gateway;
        gateway = subnet;
        subnet = (tmp != (IPAddress)INADDR_NONE) ? tmp : IPAddress(255, 255, 255, 0);
    }

    return STA.begin() && STA.config(local_ip, gateway, subnet, dns1, dns2, WIFI_VIF_INDEX_DEFAULT);
}

bool WiFiSTAClass::config(IPAddress local_ip, IPAddress dns) {

    // Use local_ip with .1 as gateway
    IPAddress gw(local_ip);
    gw[3] = 1;
    if (dns == (IPAddress)INADDR_NONE) {
        dns = gw;
    }

    return config(local_ip, gw, IPAddress(255, 255, 255, 0), dns, dns);
}

bool WiFiSTAClass::getConfig(IPAddress* local_ip, IPAddress* gateway, IPAddress* subnet, IPAddress* dns) {
    return STA.getConfig(local_ip, gateway, subnet, dns, WIFI_VIF_INDEX_DEFAULT);
}

bool WiFiSTAClass::setDNS(IPAddress dns){
    return STA.setDNS(dns, WIFI_VIF_INDEX_DEFAULT);
}

bool WiFiSTAClass::getDNS(IPAddress* dns){
    return STA.getDNS(dns, WIFI_VIF_INDEX_DEFAULT);
}

int WiFiSTAClass::getSocket(uint16_t ethertype) {
    return STA.getSocket(ethertype, WIFI_VIF_INDEX_DEFAULT);
}

IPAddress WiFiSTAClass::localIP(void) {
    return STA.localIP(WIFI_VIF_INDEX_DEFAULT);
}

IPAddress WiFiSTAClass::gatewayIP(void) {
    return STA.gatewayIP(WIFI_VIF_INDEX_DEFAULT);
}

IPAddress WiFiSTAClass::subnetMask(void) {
    return STA.subnetMask(WIFI_VIF_INDEX_DEFAULT);
}
