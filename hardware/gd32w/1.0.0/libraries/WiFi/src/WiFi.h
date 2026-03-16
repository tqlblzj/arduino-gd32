/*
 WiFi.h - esp32 Wifi support.
 Based on WiFi.h from Arduino WiFi shield library.
 Copyright (c) 2011-2014 Arduino.  All right reserved.
 Modified by Ivan Grokhotkov, December 2014

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

 Modified 12 Feb 2026 by GigaDevice Semiconductor Inc (adapt to the first version of gd32 arduino sdk).
 */

#ifndef WIFI_H
#define WIFI_H

#include "Arduino.h"
#include "WiFiGeneric.h"
#include "WiFiUDP.h"
#include "WiFiSTA.h"
#include "WiFiScan.h"
#include "wifi_management.h"
#include "WiFiAP.h"
#include "NetworkServer.h"
#include "IPAddress.h"


class WiFiClass: public WiFiGenericClass, public WiFiScanClass, public WiFiSTAClass, public WiFiAPClass {
public:
    WiFiClass();
    ~WiFiClass();
};

extern WiFiClass WiFi;

#endif // WIFI_H

