/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2025 GigaDevice Semiconductor Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "NetworkEvents.h"
#include "wrapper_os.h"
#include "stdio.h"
#include <algorithm>

#ifndef ARDUINO_NETWORK_EVENT_TASK_STACK_SIZE
#define ARDUINO_NETWORK_EVENT_TASK_STACK_SIZE 1024
#endif

NetworkEvents::NetworkEvents(): _arduino_event_group(nullptr), _arduino_event_queue(nullptr), _arduino_event_task_handle(nullptr)
{

}

NetworkEvents::~NetworkEvents()
{
    // Free event group if allocated
    if(_arduino_event_group != nullptr){
        sys_event_group_free(&_arduino_event_group);
        _arduino_event_group = nullptr;
    }
    // Free event queue and all remaining events
    if(_arduino_event_queue != nullptr){
        arduino_event_t *event = NULL;
        // Read and free all remaining events in the queue
        while(sys_queue_read(&_arduino_event_queue, &event, 0, false)){
            sys_mfree(event);
        }
        sys_queue_free(&_arduino_event_queue);
        _arduino_event_queue = nullptr;
    }
    // Delete event task if running
    if(_arduino_event_task_handle != nullptr){
        sys_task_delete(&_arduino_event_task_handle);
        _arduino_event_task_handle = nullptr;
    }
}

bool NetworkEvents::initNetworkEvents()
{
    // Initialize event group if not already created
    if(_arduino_event_group == nullptr){
        if(sys_event_group_init(&_arduino_event_group) != 0){
            printf("[NetworkEvent] Failed to init event group\n");
            return false;
        }
    }

    // Initialize event queue if not already created (queue depth: 16)
    if(_arduino_event_queue == nullptr){
        if(sys_queue_init(&_arduino_event_queue, 16, sizeof(os_queue_t)) != 0){
            printf("[NetworkEvent] Failed to init event queue\n");
            return false;
        }
    }

    // Create event task if not already running
    if(_arduino_event_task_handle == nullptr){
        _arduino_event_task_handle = sys_task_create_dynamic(
                "arduino_event_task",                                   // Task name
                ARDUINO_NETWORK_EVENT_TASK_STACK_SIZE,                  // Stack size
                OS_TASK_PRIORITY(3),                                    // Task priority
                [](void *self) {                                        // Task function
                    static_cast<NetworkEvents *>(self)->_checkForEvent();
                },
                this);                                                  // Task parameter
        if(_arduino_event_task_handle == NULL)
        {
            printf("[NetworkEvent] Failed to create arduino_event_task\n");
            return false;
        }
    }
    return true;
}

int NetworkEvents::getStatusBits() const
{
    if(_arduino_event_group == nullptr){
        return -1;
    }
    uint32_t bits = 0;
    return sys_event_group_get_bits((os_event_group_t *)(&_arduino_event_group), false);
}

int NetworkEvents::waitStatusBits(int bits, uint32_t timeout_ms)
{
    if(_arduino_event_group == nullptr){
        return -1;
    }
    return sys_event_group_wait_bits(&_arduino_event_group, bits, timeout_ms, false, false);
}

int NetworkEvents::setStatusBits(int bits)
{
    if(_arduino_event_group == nullptr){
        return -1;
    }
    return sys_event_group_set_bits(&_arduino_event_group, bits, false);
}

int NetworkEvents::clearStatusBits(int bits)
{
    if(_arduino_event_group == nullptr){
        return -1;
    }
    return sys_event_group_clear_bits(&_arduino_event_group, bits, false);
}

network_event_handle_t NetworkEvents::onEvent(NetworkEventCb cbEvent, arduino_event_id_t event) {
    if (!cbEvent) {
        return 0;
    }
    _cbEventList.emplace_back(++_current_id, cbEvent, nullptr, nullptr, event);
    return _cbEventList.back().id;
}

network_event_handle_t NetworkEvents::onEvent(NetworkEventFuncCb cbEvent, arduino_event_id_t event) {
    if (!cbEvent) {
        return 0;
    }
    _cbEventList.emplace_back(++_current_id, nullptr, cbEvent, nullptr, event);
    return _cbEventList.back().id;
}

network_event_handle_t NetworkEvents::onEvent(NetworkEventSysCb cbEvent, arduino_event_id_t event) {
    if (!cbEvent) {
        return 0;
    }
    _cbEventList.emplace_back(++_current_id, nullptr, nullptr, cbEvent, event);
    return _cbEventList.back().id;
}

void NetworkEvents::removeEvent(NetworkEventCb cbEvent, arduino_event_id_t event) {
    if (!cbEvent) {
        return;
    }
    _cbEventList.erase(
        std::remove_if(
            _cbEventList.begin(), _cbEventList.end(),
            [cbEvent, event](const NetworkEventCbList_t &e) {
                return e.cb == cbEvent && e.event == event;
            }
        ),
        _cbEventList.end()
    );
}

void NetworkEvents::removeEvent(NetworkEventSysCb cbEvent, arduino_event_id_t event) {
    if (!cbEvent) {
        return;
    }
    _cbEventList.erase(
        std::remove_if(
            _cbEventList.begin(), _cbEventList.end(),
            [cbEvent, event](const NetworkEventCbList_t &e) {
                return e.scb == cbEvent && e.event == event;
            }
        ),
        _cbEventList.end()
    );
}

void NetworkEvents::removeEvent(network_event_handle_t id) {
    _cbEventList.erase(
        std::remove_if(
            _cbEventList.begin(), _cbEventList.end(),
            [id](const NetworkEventCbList_t &e) {
                return e.id == id;
            }
        ),
        _cbEventList.end()
    );
}

bool NetworkEvents::postEvent(const arduino_event_t *data) {
    if (data == NULL || _arduino_event_queue == nullptr) {
        return false;
    }
    arduino_event_t *event = (arduino_event_t *)sys_malloc(sizeof(arduino_event_t));
    if (event == NULL) {
        printf("[NetworkEvent] Arduino Event Malloc Failed!\n");
        return false;
    }

    sys_memcpy(event, data, sizeof(arduino_event_t));
    if (sys_queue_write(&_arduino_event_queue, &event, 0, false) != 0) {
        printf("[NetworkEvent] Arduino Event Send Failed!\n");
        sys_mfree(event);
        return false;
    }
    return true;
}

void NetworkEvents::_checkForEvent() {
    // Check if queue is valid
    if (_arduino_event_queue == nullptr) {
        _arduino_event_task_handle = nullptr;
        sys_task_delete(NULL);
        return;
    }

    // Main event loop - runs forever
    for (;;) {
        arduino_event_t *event = NULL;
        // Wait for event (infinite timeout)
        if (sys_queue_fetch(&_arduino_event_queue, &event, -1, 1) != 0) {
            continue;
        }
        if (event == NULL) {
            continue;
        }
        printf("[NetworkEvent] Network Event: %d - %s\n", event->event_id, eventName(event->event_id).c_str());

        // Iterate through all registered callback functions
        for (auto &it : _cbEventList) {
            if (it.cb || it.fcb || it.scb) {
                // Check if event ID matches (or listening to all events)
                if (it.event == (arduino_event_id_t)event->event_id || it.event == ARDUINO_WIFI_MGMT_EVENT_MAX) {
                    // Call C function callback
                    if (it.cb) {
                        it.cb((arduino_event_id_t)event->event_id);
                        continue;
                    }

                    // Call function object callback with event info
                    if (it.fcb) {
                        it.fcb((arduino_event_id_t)event->event_id, (arduino_event_info_t)event->event_info);
                        continue;
                    }

                    // Call system callback with full event structure
                    it.scb(event);
                }
            }
        }
        // Free event memory after processing
        sys_mfree(event);
    }

    // Should never reach here
    sys_task_delete(NULL);
}

String NetworkEvents::eventName(arduino_event_id_t id) {
    switch (id) {
        case ARDUINO_WIFI_MGMT_EVENT_START: return "ARDUINO_WIFI_MGMT_EVENT_START";
        case ARDUINO_WIFI_MGMT_EVENT_INIT: return "ARDUINO_WIFI_MGMT_EVENT_INIT";
        case ARDUINO_WIFI_MGMT_EVENT_SWITCH_MODE_CMD: return "ARDUINO_WIFI_MGMT_EVENT_SWITCH_MODE_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_RX_MGMT: return "ARDUINO_WIFI_MGMT_EVENT_RX_MGMT";
        case ARDUINO_WIFI_MGMT_EVENT_RX_EAPOL: return "ARDUINO_WIFI_MGMT_EVENT_RX_EAPOL";
        case ARDUINO_WIFI_MGMT_EVENT_SCAN_CMD: return "ARDUINO_WIFI_MGMT_EVENT_SCAN_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_CONNECT_CMD: return "ARDUINO_WIFI_MGMT_EVENT_CONNECT_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_DISCONNECT_CMD: return "ARDUINO_WIFI_MGMT_EVENT_DISCONNECT_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_AUTO_CONNECT_CMD: return "ARDUINO_WIFI_MGMT_EVENT_AUTO_CONNECT_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_WPS_CMD: return "ARDUINO_WIFI_MGMT_EVENT_WPS_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_SCAN_DONE: return "ARDUINO_WIFI_MGMT_EVENT_SCAN_DONE";
        case ARDUINO_WIFI_MGMT_EVENT_SCAN_FAIL: return "ARDUINO_WIFI_MGMT_EVENT_SCAN_FAIL";
        case ARDUINO_WIFI_MGMT_EVENT_SCAN_RESULT: return "ARDUINO_WIFI_MGMT_EVENT_SCAN_RESULT";
        case ARDUINO_WIFI_MGMT_EVENT_EXTERNAL_AUTH_REQUIRED: return "ARDUINO_WIFI_MGMT_EVENT_EXTERNAL_AUTH_REQUIRED";
        case ARDUINO_WIFI_MGMT_EVENT_ASSOC_SUCCESS: return "ARDUINO_WIFI_MGMT_EVENT_ASSOC_SUCCESS";
        case ARDUINO_WIFI_MGMT_EVENT_DHCP_START: return "ARDUINO_WIFI_MGMT_EVENT_DHCP_START";
        case ARDUINO_WIFI_MGMT_EVENT_DHCP_SUCCESS: return "ARDUINO_WIFI_MGMT_EVENT_DHCP_SUCCESS";
        case ARDUINO_WIFI_MGMT_EVENT_DHCP_FAIL: return "ARDUINO_WIFI_MGMT_EVENT_DHCP_FAIL";
        case ARDUINO_WIFI_MGMT_EVENT_CONNECT_SUCCESS: return "ARDUINO_WIFI_MGMT_EVENT_CONNECT_SUCCESS";
        case ARDUINO_WIFI_MGMT_EVENT_CONNECT_FAIL: return "ARDUINO_WIFI_MGMT_EVENT_CONNECT_FAIL";
        case ARDUINO_WIFI_MGMT_EVENT_DISCONNECT: return "ARDUINO_WIFI_MGMT_EVENT_DISCONNECT";
        case ARDUINO_WIFI_MGMT_EVENT_ROAMING_START: return "ARDUINO_WIFI_MGMT_EVENT_ROAMING_START";
        case ARDUINO_WIFI_MGMT_EVENT_RX_UNPROT_DEAUTH: return "ARDUINO_WIFI_MGMT_EVENT_RX_UNPROT_DEAUTH";
        case ARDUINO_WIFI_MGMT_EVENT_RX_ACTION: return "ARDUINO_WIFI_MGMT_EVENT_RX_ACTION";
        case ARDUINO_WIFI_MGMT_EVENT_WPS_SUCCESS: return "ARDUINO_WIFI_MGMT_EVENT_WPS_SUCCESS";
        case ARDUINO_WIFI_MGMT_EVENT_WPS_FAIL: return "ARDUINO_WIFI_MGMT_EVENT_WPS_FAIL";
        case ARDUINO_WIFI_MGMT_EVENT_WPS_CRED: return "ARDUINO_WIFI_MGMT_EVENT_WPS_CRED";
        case ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD: return "ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_STOP_AP_CMD: return "ARDUINO_WIFI_MGMT_EVENT_STOP_AP_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_AP_SWITCH_CHNL_CMD: return "ARDUINO_WIFI_MGMT_EVENT_AP_SWITCH_CHNL_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_TX_MGMT_DONE: return "ARDUINO_WIFI_MGMT_EVENT_TX_MGMT_DONE";
        case ARDUINO_WIFI_MGMT_EVENT_CLIENT_ADDED: return "ARDUINO_WIFI_MGMT_EVENT_CLIENT_ADDED";
        case ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED: return "ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED";
        case ARDUINO_WIFI_MGMT_EVENT_CLIENT_DHCP_OFFER: return "ARDUINO_WIFI_MGMT_EVENT_CLIENT_DHCP_OFFER";
        case ARDUINO_WIFI_MGMT_EVENT_MONITOR_START_CMD: return "ARDUINO_WIFI_MGMT_EVENT_MONITOR_START_CMD";
        case ARDUINO_WIFI_MGMT_EVENT_EAP_SUCCESS: return "ARDUINO_WIFI_MGMT_EVENT_EAP_SUCCESS";
        default: return String("Unknown event");
    }
}
