/*
 * WiFiUDPClient.ino
 *
 * WiFi UDP Client Example
 *
 * Description:
 * - Demonstrates UDP client communication over WiFi
 * - Sends UDP packets to a remote server
 * - Receives and displays response packets
 *
 * Features:
 * - WiFi STA mode connection
 * - UDP packet transmission with WiFiUDP
 * - UDP packet reception with parsePacket()
 * - Event-driven WiFi connection handling
 * - Automatic reconnection on disconnect
 *
 * Configuration:
 * - Target IP: 192.168.237.1
 * - Target Port: 3333
 * - Network: GD32_UDPServer
 *
 * Data Sent:
 * - Milliseconds since boot timestamp
 *
 * Usage:
 * - Run udp_server.py on the target machine
 * - This sketch will send UDP packets every second
 * - Responses from server will be displayed
 *
 * Warning:
 * - WiFiEvent callback is called from a separate RTOS task
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>
#include <WiFi.h>

// WiFi network name and password:
const char *networkName = "GD32_UDPServer";
const char *networkPswd = "12345678";

// IP address and port to send UDP data to:
const char *udpAddress = "192.168.237.1";
const int udpPort = 3333;

// Are we currently connected?
bool connected = false;
int packetNum = 0;

// The udp library class
WiFiUDP udp;

void setup() {
    // Initialize hardware serial:
    Serial.begin(115200);

    // Connect to WiFi network
    connectToWiFi(networkName, networkPswd);
}

void loop() {
    // Only send data when connected
    if (connected) {
        // Send a packet
        udp.beginPacket(udpAddress, udpPort);
        udp.print("milliSeconds since boot: ");
        udp.print(millis());
        udp.endPacket();

        delay(10);
        int len = udp.parsePacket();
        int avail = udp.available();

        Serial.print("Packet ");
        Serial.print(packetNum);
        Serial.print(", length: ");
        Serial.println(avail);

        if(avail > 0){
            packetNum++;
            Serial.println(udp.readString());
        }
        delay(1000);
    }else{
        Serial.println("Connecting to WiFi......");
        delay(1000);
    }
}

void connectToWiFi(const char *ssid, const char *pwd) {

    Serial.printf("Connecting to WiFi network: %s\r\n", ssid);

    // Delete old config
    WiFi.disconnect();

    // Initialize Network events
    Network.begin();
    // Register event handler
    WiFi.onEvent(WiFiEvent);

    Serial.print("MAC: ");
    Serial.println(Network.macAddress());

    // Initiate connection
    WiFi.begin((char*)ssid, (char*)pwd);

    Serial.println("Waiting for WIFI connection...");
}

// WARNING: WiFiEvent is called from a separate RTOS task (thread)!
void WiFiEvent(WiFiEvent_t event) {
    String eventName = WiFi.eventName(event);
    Serial.print("[WiFi Event] eventName: ");
    Serial.println(eventName);
    switch (event) {
        case ARDUINO_WIFI_MGMT_EVENT_CONNECT_SUCCESS:
            // When connected set
            Serial.print("[WiFi Event] WiFi connected! IP address: ");
            Serial.println(WiFi.localIP());

            // Initializes UDP state
            // This initializes the transfer buffer
            udp.begin(WiFi.localIP(), udpPort);
            connected = true;
            break;
        case ARDUINO_WIFI_MGMT_EVENT_CONNECT_FAIL:
            Serial.println("[WiFi Event] WiFi connection failed");
            connected = false;
            break;

        case ARDUINO_WIFI_MGMT_EVENT_DISCONNECT:
            Serial.println("[WiFi Event] WiFi lost connection");
            connected = false;
            break;

        default:
            break;
    }
}

/*
    11:26:31.793  MAC: 76:BA:ED:26:00:63
    11:26:31.793  [WiFi Event] eventName: ARDUINO_WIFI_MGMT_EVENT_INIT
    11:26:32.984  Waiting for WIFI connection...
    11:26:32.985  Connecting to WiFi......
    11:26:32.987  [WiFi Event] eventName: ARDUINO_WIFI_MGMT_EVENT_CONNECT_SUCCESS
    11:26:32.988  [WiFi Event] WiFi connected! IP address: 172.20.10.2
    11:26:34.039  Packet 0, length: 0
    11:26:35.082  Packet 0, length: 0
    11:26:36.132  Packet 0, length: 29
    11:26:36.134  milliSeconds since boot: 2167
    11:26:37.189  Packet 1, length: 29
    11:26:37.190  milliSeconds since boot: 3222
    11:26:38.244  Packet 2, length: 29
    11:26:38.245  milliSeconds since boot: 4277
    11:26:39.304  Packet 3, length: 29
    11:26:39.305  milliSeconds since boot: 5329
    11:26:40.357  Packet 4, length: 29
    11:26:40.358  milliSeconds since boot: 6381
    11:26:41.351  Packet 5, length: 29
    11:26:41.352  milliSeconds since boot: 8485
    11:26:42.409  Packet 6, length: 29
    11:26:42.410  milliSeconds since boot: 9537
*/
