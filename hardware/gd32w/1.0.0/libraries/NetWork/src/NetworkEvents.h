/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2025 GigaDevice Semiconductor Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NETWORKEVENT_H
#define NETWORKEVENT_H

#include <functional>
#include "wrapper_os.h"
#include <WString.h>

typedef enum {
    ARDUINO_WIFI_MGMT_EVENT_START = 4,

    /* For both STA and SoftAP */
    ARDUINO_WIFI_MGMT_EVENT_INIT,  //5
    ARDUINO_WIFI_MGMT_EVENT_SWITCH_MODE_CMD,
    ARDUINO_WIFI_MGMT_EVENT_RX_MGMT,
    ARDUINO_WIFI_MGMT_EVENT_RX_EAPOL,

    /* For STA only */
    ARDUINO_WIFI_MGMT_EVENT_SCAN_CMD,
    ARDUINO_WIFI_MGMT_EVENT_CONNECT_CMD,  //10
    ARDUINO_WIFI_MGMT_EVENT_DISCONNECT_CMD,
    ARDUINO_WIFI_MGMT_EVENT_AUTO_CONNECT_CMD,
    ARDUINO_WIFI_MGMT_EVENT_WPS_CMD,

    ARDUINO_WIFI_MGMT_EVENT_SCAN_DONE,
    ARDUINO_WIFI_MGMT_EVENT_SCAN_FAIL,
    ARDUINO_WIFI_MGMT_EVENT_SCAN_RESULT,  //16
    ARDUINO_WIFI_MGMT_EVENT_EXTERNAL_AUTH_REQUIRED,  //17

    ARDUINO_WIFI_MGMT_EVENT_ASSOC_SUCCESS,  //18

    ARDUINO_WIFI_MGMT_EVENT_DHCP_START,
    ARDUINO_WIFI_MGMT_EVENT_DHCP_SUCCESS,
    ARDUINO_WIFI_MGMT_EVENT_DHCP_FAIL, //21

    ARDUINO_WIFI_MGMT_EVENT_CONNECT_SUCCESS,
    ARDUINO_WIFI_MGMT_EVENT_CONNECT_FAIL,

    ARDUINO_WIFI_MGMT_EVENT_DISCONNECT,
    ARDUINO_WIFI_MGMT_EVENT_ROAMING_START,
    ARDUINO_WIFI_MGMT_EVENT_RX_UNPROT_DEAUTH, //26
    ARDUINO_WIFI_MGMT_EVENT_RX_ACTION,

    /* For STA WPS */
    ARDUINO_WIFI_MGMT_EVENT_WPS_SUCCESS, //28
    ARDUINO_WIFI_MGMT_EVENT_WPS_FAIL,
    ARDUINO_WIFI_MGMT_EVENT_WPS_CRED,

    /* For SoftAP only */
    ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD,  //31
    ARDUINO_WIFI_MGMT_EVENT_STOP_AP_CMD,
    ARDUINO_WIFI_MGMT_EVENT_AP_SWITCH_CHNL_CMD,

    ARDUINO_WIFI_MGMT_EVENT_TX_MGMT_DONE, //34
    ARDUINO_WIFI_MGMT_EVENT_CLIENT_ADDED,
    ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED, //36
    ARDUINO_WIFI_MGMT_EVENT_CLIENT_DHCP_OFFER,
    /* For Monitor only */
    ARDUINO_WIFI_MGMT_EVENT_MONITOR_START_CMD,

    /* For STA 802.1x EAP */
    ARDUINO_WIFI_MGMT_EVENT_EAP_SUCCESS,

    ARDUINO_WIFI_MGMT_EVENT_MAX,
    ARDUINO_WIFI_MGMT_EVENT_NUM = ARDUINO_WIFI_MGMT_EVENT_MAX - ARDUINO_WIFI_MGMT_EVENT_START - 1,
} arduino_event_id_t;

// Not used currently, but defined for future use
typedef union {

} arduino_event_info_t;

struct arduino_event_t {
  arduino_event_id_t event_id;
  arduino_event_info_t event_info;
};

using NetworkEventCb = void (*)(arduino_event_id_t event);
using NetworkEventFuncCb = std::function<void(arduino_event_id_t event, arduino_event_info_t info)>;
using NetworkEventSysCb = void (*)(arduino_event_t *event);
using network_event_handle_t = size_t;

class NetworkEvents {
public:
    NetworkEvents();
    ~NetworkEvents();
    network_event_handle_t onEvent(NetworkEventCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);
    network_event_handle_t onEvent(NetworkEventFuncCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);
    network_event_handle_t onEvent(NetworkEventSysCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);

    void removeEvent(NetworkEventCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);
    void removeEvent(NetworkEventSysCb cbEvent, arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX);
    void removeEvent(network_event_handle_t id);

    bool postEvent(const arduino_event_t *event);

    int getStatusBits() const;
    int waitStatusBits(int bits, uint32_t timeout_ms);
    int setStatusBits(int bits);
    int clearStatusBits(int bits);
    String eventName(arduino_event_id_t id);

protected:
    bool initNetworkEvents();

private:

    struct NetworkEventCbList_t {
        network_event_handle_t id;
        NetworkEventCb cb;
        NetworkEventFuncCb fcb;
        NetworkEventSysCb scb;
        arduino_event_id_t event;

        explicit NetworkEventCbList_t(
            network_event_handle_t id, NetworkEventCb cb = nullptr, NetworkEventFuncCb fcb = nullptr, NetworkEventSysCb scb = nullptr,
            arduino_event_id_t event = ARDUINO_WIFI_MGMT_EVENT_MAX
        )
            : id(id), cb(cb), fcb(fcb), scb(scb), event(event) {}
    };

    network_event_handle_t _current_id{0};
    os_event_group_t _arduino_event_group;
    os_queue_t _arduino_event_queue;
    os_task_t _arduino_event_task_handle;

    // registered events callbacks container
    std::vector<NetworkEventCbList_t> _cbEventList;

    void _checkForEvent();
};

#endif // NETWORKEVENT_H