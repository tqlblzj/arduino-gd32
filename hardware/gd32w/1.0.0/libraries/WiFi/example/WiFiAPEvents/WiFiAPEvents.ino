/*
 * WiFiAPEvents.ino
 *
 * WiFi AP Events Example
 *
 * Description:
 * - Demonstrates WiFi SoftAP event handling
 * - Shows how to register callbacks for AP events
 * - Tracks client connections and disconnections
 *
 * Features:
 * - WiFi AP event registration
 * - Client connect/disconnect event handling
 * - Station count tracking
 * - Event callback functions (called from separate RTOS task)
 *
 * WiFi AP Events:
 * - ARDUINO_WIFI_MGMT_EVENT_INIT (5): WiFi initialized
 * - ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD (31): Start AP command sent
 * - ARDUINO_WIFI_MGMT_EVENT_STOP_AP_CMD (32): Stop AP command sent
 * - ARDUINO_WIFI_MGMT_EVENT_CLIENT_DHCP_OFFER (35): Client connected to AP
 * - ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED (36): Client disconnected from AP
 *
 * Configuration:
 * - AP SSID: GD32_AP_Example
 * - AP Password: 12345678
 * - Channel: 1
 * - Auth Mode: WPA2_PSK
 *
 * Warning:
 * - Event callback functions are called from a separate RTOS task
 * - Use proper synchronization when accessing shared variables
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>
#include <NetworkManager.h>

const char *ap_ssid = "GD32_AP_Example";
const char *ap_password = "12345678";
const int ap_channel = 1;
const int ap_auth_mode = 3;  // 3 = WPA2_PSK

// Track number of connected clients
int clientCount = 0;

// WARNING: This function is called from a separate RTOS task (thread)!
void WiFiAPEvent(arduino_event_id_t event) {

    String eventName = WiFi.eventName(event);
    Serial.print("[WiFi AP Event] Event: ");
    Serial.println(eventName);

    switch (event) {
        case ARDUINO_WIFI_MGMT_EVENT_INIT:
            Serial.println("[WiFi AP Event] WiFi initialized");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD:
            Serial.println("[WiFi AP Event] Start AP command sent");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_STOP_AP_CMD:
            Serial.println("[WiFi AP Event] Stop AP command sent");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_CLIENT_DHCP_OFFER:
            clientCount++;
            Serial.print("[WiFi AP Event] Client connected! Total clients: ");
            Serial.println(clientCount);
            Serial.print("[WiFi AP Event] Current station count: ");
            Serial.println(WiFi.softAPgetStationNum());
            break;
        case ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED:
            clientCount--;
            Serial.print("[WiFi AP Event] Client disconnected! Total clients: ");
            Serial.println(clientCount);
            Serial.print("[WiFi AP Event] Current station count: ");
            Serial.println(WiFi.softAPgetStationNum());
            break;
        default:
            break;
    }
}

// WARNING: This function is called from a separate RTOS task (thread)!
void ClientConnectedEvent(arduino_event_id_t event, arduino_event_info_t info) {
    Serial.println("[WiFi AP Event] Client connected (lambda callback)");
}

// WARNING: This function is called from a separate RTOS task (thread)!
void ClientDisconnectedEvent(arduino_event_id_t event, arduino_event_info_t info) {
    Serial.println("[WiFi AP Event] Client disconnected (lambda callback)");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("  WiFi AP Events Example");
    Serial.println("========================================");
    Serial.println();

    // Initialize Network events system
    if (!Network.begin()) {
        Serial.println("Failed to initialize Network events!");
        return;
    }
    Serial.println("Network events initialized");

    // Register for all WiFi AP events
    WiFi.onEvent(WiFiAPEvent);
    Serial.println("Registered WiFi AP event handler");

    // Register for specific events using lambda functions
    wifi_event_id_t clientConnectedID = WiFi.onEvent(
        ClientConnectedEvent,
        ARDUINO_WIFI_MGMT_EVENT_CLIENT_DHCP_OFFER
    );

    wifi_event_id_t clientDisconnectedID = WiFi.onEvent(
        ClientDisconnectedEvent,
        ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED
    );

    Serial.print("Client connected event ID: ");
    Serial.println(clientConnectedID);
    Serial.print("Client disconnected event ID: ");
    Serial.println(clientDisconnectedID);
    Serial.println();

    // Start SoftAP
    Serial.println("Starting SoftAP...");
    Serial.print("SSID: ");
    Serial.println(ap_ssid);
    Serial.print("Password: ");
    Serial.println(ap_password);
    Serial.print("Channel: ");
    Serial.println(ap_channel);
    Serial.print("Auth Mode: ");
    Serial.println(ap_auth_mode);
    Serial.println();

    if (!WiFi.softAP((char*)ap_ssid, (char*)ap_password, ap_channel, ap_auth_mode, 0)) {
        Serial.println("Failed to start SoftAP!");
        return;
    }

    Serial.println("SoftAP started successfully!");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println();

    Serial.println("========================================");
    Serial.println("  Waiting for clients...");
    Serial.println("========================================");
    Serial.println();
}

void loop() {
    // Print AP status every 5 seconds
    static unsigned long lastPrint = 0;
    unsigned long now = millis();

    if (now - lastPrint >= 5000) {
        lastPrint = now;

        Serial.println("--- AP Status ---");
        Serial.print("SSID: ");
        Serial.println(WiFi.softAPSSID());
        Serial.print("IP Address: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("Station Count: ");
        Serial.println(WiFi.softAPgetStationNum());
        Serial.print("Tracked Client Count: ");
        Serial.println(clientCount);
        Serial.println("------------------");
        Serial.println();
    }

    delay(100);
}