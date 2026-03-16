/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2025 GigaDevice Semiconductor Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NetworkManager_h
#define NetworkManager_h


#include "NetworkEvents.h"
#include "NetworkInterface.h"
#include "IPAddress.h"
#include "WString.h"

class NetworkManager : public NetworkEvents {
public:
    NetworkManager();

    bool begin();
    int hostByName(const char *aHostname, IPAddress &aResult);
    uint8_t *macAddress(uint8_t *mac);
    String macAddress();

    static const char *getHostname();
    static bool setHostname(const char *hostname);
    static bool hostname(const String &aHostname) {
        return setHostname(aHostname.c_str());
    }
};

extern NetworkManager Network;

#endif
