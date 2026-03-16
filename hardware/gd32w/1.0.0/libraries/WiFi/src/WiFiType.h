/*
 ESP8266WiFiType.h - esp8266 Wifi support.
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

#ifndef WIFITYPE_H
#define WIFITYPE_H

#include "wifi_netlink.h"

#define WiFiEvent_t   arduino_event_id_t

#define wifi_status_t wifi_work_status_t

typedef enum {
    WL_NO_SHIELD = 255,
    WL_STOPPED = 254,
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL = 1,
    WL_SCAN_COMPLETED = 2,
    WL_CONNECTED = 3,
    WL_CONNECT_FAILED = 4,
    WL_CONNECTION_LOST = 5,
    WL_DISCONNECTED = 6,
    WL_CONNECT_ONGOING
} wl_status_t;

typedef enum {
    WIFI_OFF = 0,
    WIFI_STA = 1,
    WIFI_AP = 2,
    WIFI_AP_STA = 3,
    WIFI_MAX_MODE
} wifi_mode_t;

#endif /* SOC_WIFI_SUPPORTED */