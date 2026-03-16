/*
 ESP8266WiFiSTA.h - esp8266 Wifi support.
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

#ifndef WIFISTA_H
#define WIFISTA_H

#include <stdint.h>
#include <WString.h>
#include "IPAddress.h"

#include "WiFiType.h"
#include "NetworkInterface.h"

class STAClass: public NetworkInterface {
public:
    STAClass();
    ~STAClass();

    bool begin();
    bool end(bool wifi_off = false);

    wl_status_t status;

protected:
    bool _sta_initialized;
    bool onEnable();
    bool onDisable();

private:
    void _setStatus(wl_status_t status);
    void _onStaEvent(void *eloop_data, void *user_ctx);
};

class WiFiSTAClass {
public:
    STAClass STA;
    WiFiSTAClass();
    ~WiFiSTAClass();

    wl_status_t begin(char* ssid, char* password, uint8_t blocked = 1);

    bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0x00000000, IPAddress dns2 = (uint32_t)0x00000000);
    bool config(IPAddress local_ip, IPAddress dns = (uint32_t)0x00000000);

    bool setDNS(IPAddress dns1);
    bool getDNS(IPAddress* dns);
    int getSocket(uint16_t ethertype);
    IPAddress localIP(void);
    IPAddress subnetMask(void);
    IPAddress gatewayIP(void);

    bool disconnect();
    wl_status_t status();

    bool getConfig(IPAddress* local_ip, IPAddress* gateway, IPAddress* subnet, IPAddress* dns);

    String macAddress();
    uint8_t* macAddress(uint8_t* mac);

    void handleConnectResults(bool success = false){
        _processConnectResults(success);
    }

protected:
    void _processConnectResults(bool success = false);
};

#endif // WIFISTA_H
