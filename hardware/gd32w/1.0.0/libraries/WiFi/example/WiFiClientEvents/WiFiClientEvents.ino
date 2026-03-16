/*
 * WiFiClientEvents.ino
 *
 * WiFi STA Events Example
 *
 * Description:
 * - Demonstrates WiFi STA (Station) event handling
 * - Shows how to register callbacks for WiFi events
 * - Displays connection status and scan results
 *
 * Features:
 * - WiFi STA event registration
 * - Connection success/failure event handling
 * - Scan completion event handling
 * - Disconnection event handling
 * - Event callback functions (called from separate RTOS task)
 *
 * WiFi Events:
 * - ARDUINO_WIFI_MGMT_EVENT_INIT (5): WiFi initialized
 * - ARDUINO_WIFI_MGMT_EVENT_SCAN_DONE (14): Scan done
 * - ARDUINO_WIFI_MGMT_EVENT_SCAN_FAIL (15): Scan failed
 * - ARDUINO_WIFI_MGMT_EVENT_CONNECT_SUCCESS (22): Connected to AP
 * - ARDUINO_WIFI_MGMT_EVENT_CONNECT_FAIL (23): Connection failed
 * - ARDUINO_WIFI_MGMT_EVENT_DISCONNECT (24): Disconnected from AP
 * - ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD (31): Start AP command
 * - ARDUINO_WIFI_MGMT_EVENT_STOP_AP_CMD (32): Stop AP command
 * - ARDUINO_WIFI_MGMT_EVENT_CLIENT_ADDED (35): Client connected to AP
 * - ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED (36): Client disconnected from AP
 *
 * Warning:
 * - Event callback functions are called from a separate RTOS task
 * - Use proper synchronization when accessing shared variables
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>
#include <NetworkManager.h>

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

// WARNING: This function is called from a separate RTOS task (thread)!
void WiFiEvent(arduino_event_id_t event) {

    String eventName = WiFi.eventName(event);
    Serial.print("[WiFi Event] eventName: ");
    Serial.println(eventName);

    switch (event) {
        case ARDUINO_WIFI_MGMT_EVENT_INIT:
            Serial.println("[WiFi Event] WiFi initialized");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_SCAN_DONE:
            Serial.println("[WiFi Event] Scan done");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_SCAN_FAIL:
            Serial.println("[WiFi Event] Scan failed");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_CONNECT_SUCCESS:
            Serial.println("[WiFi Event] Connected to AP successfully");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_CONNECT_FAIL:
            Serial.println("[WiFi Event] Connection failed");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_DISCONNECT:
            Serial.println("[WiFi Event] Disconnected from AP");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD:
            Serial.println("[WiFi Event] Start AP command sent");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_STOP_AP_CMD:
            Serial.println("[WiFi Event] Stop AP command sent");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_CLIENT_ADDED:
            Serial.println("[WiFi Event] Client connected to AP");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED:
            Serial.println("[WiFi Event] Client disconnected from AP");
            break;
        default:
            break;
    }
}

// WARNING: This function is called from a separate RTOS task (thread)!
void printScannedNetworks(arduino_event_id_t event, arduino_event_info_t info) {
    int16_t WiFiScanStatus = WiFi.scanComplete();
    int16_t networksFound = 0;
    if (WiFiScanStatus >= 0) {
        networksFound = WiFiScanStatus;
    }

    if (networksFound == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(networksFound);
        Serial.println(" networks found");
        Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
        Serial.println("-- | -------------------------------- | ---- | -- | ----------");
        for (int i = 0; i < networksFound; ++i) {
            /* Nr (2 chars) */
            if (i + 1 < 10) Serial.print(" ");
            Serial.print(i + 1);
            Serial.print(" | ");

            String ssid = WiFi.SSID(i);
            if(ssid==""){
                Serial.println("This AP is hidden");
                delay(10);
                continue;
            }
            Serial.print(ssid);
            for (int s = ssid.length(); s < 32; s++) {
                Serial.print(" ");
            }
            Serial.print(" | ");

            int rssi = WiFi.RSSI(i);
            if (rssi > -100) Serial.print(" ");
            if (rssi > -10)  Serial.print(" ");
            Serial.print(rssi);
            Serial.print(" | ");

            int ch = WiFi.channel(i);
            if (ch < 10) Serial.print(" ");
            Serial.print(ch);
            Serial.print(" | ");

            /* Encryption */
            switch (WiFi.encryptionType(i)) {
                case AUTH_MODE_OPEN:        Serial.print("OPEN"); break;
                case AUTH_MODE_WEP:         Serial.print("WEP"); break;
                case AUTH_MODE_WPA:         Serial.print("WPA"); break;
                case AUTH_MODE_WPA2:        Serial.print("WPA2"); break;
                case AUTH_MODE_WPA_WPA2:    Serial.print("WPA+WPA2"); break;
                case AUTH_MODE_WPA3:        Serial.print("WPA3");break;
                case AUTH_MODE_WPA2_WPA3:   Serial.print("WPA2+WPA3"); break;
                default:                    Serial.print("unknown"); break;
        }
            Serial.println();
            delay(10);
        }
    }

    WiFi.scanDelete();
    Serial.println("-------------------------------------");
}

void setup() {
    Serial.begin(115200);

    // Initialize Network events
    Network.begin();

    // Examples of different ways to register wifi events;
    // these handlers will be called from another thread.
    WiFi.onEvent(WiFiEvent);

    // Register for specific events
    wifi_event_id_t disconnectEventID = WiFi.onEvent(
        [](arduino_event_id_t event, arduino_event_info_t info) {
            Serial.println("[WiFi Event] Disconnected from WiFi AP (lambda)");
        },
        ARDUINO_WIFI_MGMT_EVENT_DISCONNECT
    );

    // Remove WiFi event
    Serial.print("WiFi Disconnect Event ID: ");
    Serial.println(disconnectEventID);
    // WiFi.removeEvent(eventID);

    wifi_event_id_t scanResultEventID = WiFi.onEvent(
            printScannedNetworks,
            ARDUINO_WIFI_MGMT_EVENT_SCAN_DONE
        );

    // Connect to WiFi
    Serial.println("Wait for WiFi... ");

    // scan asychronously
    WiFi.scanNetworks(true);
    delay(10000);

    // Uncomment to use synchronous scan
    // WiFi.scanNetworks();

    WiFi.begin((char*)ssid, (char*)password);
    delay(10000);
    WiFi.disconnect();
}

void loop() {
    delay(1000);
}