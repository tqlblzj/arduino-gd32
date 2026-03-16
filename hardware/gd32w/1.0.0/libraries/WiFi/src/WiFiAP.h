/*
 ESP8266WiFiAP.h - esp8266 Wifi support.
 Based on WiFi.h from Arduino WiFi shield library.
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

#ifndef WIFIAP_H
#define WIFIAP_H

#include <stdint.h>
#include <WString.h>
#include "IPAddress.h"
#include "WiFiType.h"
#include "NetworkInterface.h"
#include "wifi_management.h"

// Forward declaration
class WiFiGenericClass;

/**
 * APClass - WiFi Access Point class for GD32VW55x
 */
class APClass : public NetworkInterface {
public:
    APClass();
    ~APClass();

    bool begin();
    bool end();

    // Configure AP settings
    bool create(
        char *ssid,
        char *passphrase,
        int channel,
        int auth_mode,
        int ssid_hidden
    );

    bool clear();

    // Get AP information
    String SSID(void) const;
    uint8_t stationCount();

protected:
    bool _ap_initialized;
};

/**
 * WiFiAPClass - WiFi AP interface class
 */
class WiFiAPClass {
public:
    APClass AP;

    WiFiAPClass();
    ~WiFiAPClass();

    // Create SoftAP
    bool softAP(
        char *ssid,
        char *passphrase,
        int channel,
        int auth_mode,
        int ssid_hidden
    );

    bool softAP(
        String &ssid,
        String &passphrase,
        int channel,
        int auth_mode,
        int ssid_hidden
    ) {
        return softAP((char*)ssid.c_str(), (char*)passphrase.c_str(), channel, auth_mode, ssid_hidden);
    }

    // Configure AP IP settings
    bool softAPConfig(
        IPAddress local_ip,
        IPAddress gateway,
        IPAddress subnet,
        IPAddress dhcp_lease_start = (uint32_t)0,
        IPAddress dns = (uint32_t)0
    );

    // Disconnect from AP
    bool softAPdisconnect(bool wifioff = false);

    // Get AP information
    uint8_t softAPgetStationNum();
    String softAPSSID(void) const;
    IPAddress softAPIP();
    IPAddress softAPBroadcastIP();
    IPAddress softAPNetworkID();
    IPAddress softAPSubnetMask();
    uint8_t softAPSubnetCIDR();

    // Get MAC address
    uint8_t *softAPmacAddress(uint8_t *mac);
    String softAPmacAddress(void);

protected:
    void _onApEvent(int32_t event_id, void *event_data);
};

#endif // WIFIAP_H
