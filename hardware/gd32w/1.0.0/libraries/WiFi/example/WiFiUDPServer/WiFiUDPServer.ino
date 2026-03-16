/*
 * WiFiUDPServer.ino
 *
 * WiFi UDP Server Example
 *
 * Description:
 * - Demonstrates UDP server creation on WiFi SoftAP
 * - Receives UDP packets from clients
 * - Echoes received data back to sender
 * - Displays packet statistics
 *
 * Features:
 * - WiFi SoftAP mode
 * - UDP packet reception with parsePacket()
 * - UDP packet transmission with beginPacket()/endPacket()
 * - Remote IP and port extraction
 * - Packet statistics tracking
 *
 * Server Configuration:
 * - Mode: SoftAP (Access Point)
 * - SSID: GD32_UDPServer
 * - Password: 12345678
 * - Channel: 1
 * - Auth Mode: WPA2_PSK
 * - UDP Port: 3333
 *
 * Testing:
 * - Use WiFiUDPClient example
 * - Or run udp_client.py on a connected machine
 *
 * Statistics:
 * - Packets received
 * - Bytes received
 * - Remote client information
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>
#include <WiFi.h>

// AP configuration
const char *ap_ssid = "GD32_UDPServer";
const char *ap_password = "12345678";
const int ap_channel = 1;
const int ap_auth_mode = 3;  // 3 = WPA2_PSK

// UDP server port
const int udpPort = 3333;

// The udp library class
WiFiUDP udp;

// Track if server is ready
bool serverReady = false;

// Statistics
unsigned long packetsReceived = 0;
unsigned long bytesReceived = 0;

void setup() {
    // Initialize hardware serial:
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("  WiFi UDP Server Example");
    Serial.println("========================================");
    Serial.println();

    // Initialize Network events
    if (!Network.begin()) {
        Serial.println("Failed to initialize Network events!");
        return;
    }

    // Register event handler for AP events
    WiFi.onEvent(WiFiEvent);

    // Start SoftAP
    Serial.println("Starting SoftAP...");
    Serial.print("SSID: ");
    Serial.println(ap_ssid);
    Serial.print("Password: ");
    Serial.println(ap_password);
    Serial.print("Channel: ");
    Serial.println(ap_channel);
    Serial.println();

    if (!WiFi.softAP((char*)ap_ssid, (char*)ap_password, ap_channel, ap_auth_mode, 0)) {
        Serial.println("Failed to start SoftAP!");
        return;
    }

    Serial.println("SoftAP started successfully!");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println();

    // Start UDP server
    Serial.println("Starting UDP server...");
    Serial.print("Listening on port: ");
    Serial.println(udpPort);

    // Begin listening on the specified port
    udp.begin(udpPort);

    Serial.println("UDP server ready!");
    Serial.println();
    Serial.println("========================================");
    Serial.println("  Waiting for UDP packets...");
    Serial.println("========================================");
    Serial.println();
}

void loop() {
    // Check if there's data available to read
    int packetSize = udp.parsePacket();

    if (packetSize > 0) {
        packetsReceived++;
        bytesReceived += packetSize;

        // Get sender's IP and port
        IPAddress remoteIp = udp.remoteIP();
        uint16_t remotePort = udp.remotePort();

        // Read the packet
        char packetBuffer[256];
        int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
        if (len > 0) {
            packetBuffer[len] = '\0';  // Null-terminate the string
        }

        // Print received packet info
        Serial.println("--- Packet Received ---");
        Serial.print("From: ");
        Serial.print(remoteIp);
        Serial.print(":");
        Serial.println(remotePort);
        Serial.print("Length: ");
        Serial.println(packetSize);
        Serial.print("Data: ");
        Serial.println(packetBuffer);

        // Echo the data back to the client
        udp.beginPacket(remoteIp, remotePort);
        udp.print("Echo: ");
        udp.write((uint8_t*)packetBuffer, len);
        udp.endPacket();

        Serial.print("Echoed back to ");
        Serial.print(remoteIp);
        Serial.print(":");
        Serial.println(remotePort);
        Serial.println("-----------------------");
        Serial.println();
    }

    // Print statistics every 10 seconds
    static unsigned long lastStatsPrint = 0;
    unsigned long now = millis();

    if (now - lastStatsPrint >= 10000) {
        lastStatsPrint = now;

        Serial.println("--- Server Statistics ---");
        Serial.print("AP SSID: ");
        Serial.println(WiFi.softAPSSID());
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("Station Count: ");
        Serial.println(WiFi.softAPgetStationNum());
        Serial.print("Packets Received: ");
        Serial.println(packetsReceived);
        Serial.print("Bytes Received: ");
        Serial.println(bytesReceived);
        Serial.print("Uptime: ");
        Serial.print(now / 1000);
        Serial.println(" seconds");
        Serial.println("-------------------------");
        Serial.println();
    }

    delay(10);
}

// WARNING: WiFiEvent is called from a separate RTOS task (thread)!
void WiFiEvent(arduino_event_id_t event) {
    String eventName = WiFi.eventName(event);
    Serial.print("[WiFi Event] Event: ");
    Serial.println(eventName);

    switch (event) {
        case ARDUINO_WIFI_MGMT_EVENT_INIT:
            Serial.println("[WiFi Event] WiFi initialized");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_START_AP_CMD:
            Serial.println("[WiFi Event] Start AP command sent");
            break;
        case ARDUINO_WIFI_MGMT_EVENT_CLIENT_ADDED:
            Serial.println("[WiFi Event] Client connected to AP");
            Serial.print("[WiFi Event] Current station count: ");
            Serial.println(WiFi.softAPgetStationNum());
            break;
        case ARDUINO_WIFI_MGMT_EVENT_CLIENT_REMOVED:
            Serial.println("[WiFi Event] Client disconnected from AP");
            Serial.print("[WiFi Event] Current station count: ");
            Serial.println(WiFi.softAPgetStationNum());
            break;
        default:
            break;
    }
}

/*
 * Example Output(use udp_client.py):
    09:41:19.768  ========================================
    09:41:19.768    WiFi UDP Server Example
    09:41:19.769  ========================================
    09:41:19.770
    09:41:19.770  Starting SoftAP...
    09:41:19.771  SSID: GD32_UDPServer
    09:41:19.827  Password: 12345678
    09:41:19.828  Channel: 1
    09:41:19.829
    09:41:19.830  [WiFi Event] Event: ARDUINO_WIFI_MGMT_EVENT_INIT
    09:41:19.830  [WiFi Event] WiFi initialized
    09:41:19.949  SoftAP started successfully!
    09:41:19.950  AP IP Address: 192.168.237.1
    09:41:19.950
    09:41:19.951  Starting UDP server...
    09:41:19.952  Listening on port: 3333
    09:41:19.952  UDP server ready!
    09:41:19.953
    09:41:19.953  ========================================
    09:41:19.954    Waiting for UDP packets...
    09:41:20.010  ============================
    09:41:20.010
    09:41:20.011  [WiFi Event] Event: ARDUINO_MGMT_EVENT_START_AP_CMD
    09:41:20.011  [WiFi Event] Start AP command sent
    09:41:28.783  --- Server Statistics ---
    09:41:28.784  AP SSID: GD32_UDPServer
    09:41:28.785  AP IP: 192.168.237.1
    09:41:28.786  Station Count: 0
    09:41:28.786  Packets Received: 0
    09:41:28.793  Bytes Received: 0
    09:41:28.794  Uptime: 10 seconds
    09:41:28.794  -------------------------
    09:41:28.795
    09:41:38.779  --- Server Statistics ---
    09:41:38.780  AP SSID: GD32_UDPServer
    09:41:38.781  AP IP: 192.168.237.1
    09:41:38.783  Station Count: 1
    09:41:38.783  Packets Received: 0
    09:41:38.784  Bytes Received: 0
    09:41:38.785  Uptime: 20 seconds
    09:41:38.785  -------------------------
    09:41:38.785
    09:41:38.970  [WiFi Event] Event: ARDUINO_WIFI_MGMT_EVENT_CLIENT_DHCP_OFFER
    09:41:48.771  --- Server Statistics ---
    09:41:48.773  AP SSID: GD32_UDPServer
    09:41:48.774  AP IP: 192.168.237.1
    09:41:48.832  Station Count: 1
    09:41:48.834  Packets Received: 0
    09:41:48.835  Bytes Received: 0
    09:41:48.835  Uptime: 30 seconds
    09:41:48.837  -------------------------
    09:41:48.838
    09:41:53.619  --- Packet Received ---
    09:41:53.622  From: 192.168.237.2:51448
    09:41:53.623  Length: 15
    09:41:53.624  Data: Connection Test
    09:41:53.625  Echoed back to 192.168.237.2:51448
    09:41:53.626  -----------------------
    09:41:53.627
    09:41:58.773  --- Server Statistics ---
    09:41:58.775  AP SSID: GD32_UDPServer
    09:41:58.776  AP IP: 192.168.237.1
    09:41:58.777  Station Count: 1
    09:41:58.834  Packets Received: 1
    09:41:58.836  Bytes Received: 15
    09:41:58.838  Uptime: 40 seconds
    09:41:58.839  -------------------------
    09:41:58.840
    09:42:00.428  --- Packet Received ---
    09:42:00.430  From: 192.168.237.2:60782
    09:42:00.431  Length: 42
    09:42:00.432  Data: Test Packet 1/5 - Timestamp: 1770082920.36
    09:42:00.433  Echoed back to 192.168.237.2:60782
    09:42:00.434  -----------------------
    09:42:00.435
    09:42:01.410  --- Packet Received ---
    09:42:01.414  From: 192.168.237.2:60782
    09:42:01.470  Length: 42
    09:42:01.472  Data: Test Packet 2/5 - Timestamp: 1770082921.39
    09:42:01.473  Echoed back to 192.168.237.2:60782
    09:42:01.475  -----------------------
    09:42:01.476
    09:42:02.452  --- Packet Received ---
    09:42:02.458  From: 192.168.237.2:60782
    09:42:02.512  Length: 42
    09:42:02.512  Data: Test Packet 3/5 - Timestamp: 1770082922.43
    09:42:02.513  Echoed back to 192.168.237.2:60782
    09:42:02.513  -----------------------
    09:42:02.514
    09:42:03.496  --- Packet Received ---
    09:42:03.500  From: 192.168.237.2:60782
    09:42:03.501  Length: 42
    09:42:03.556  Data: Test Packet 4/5 - Timestamp: 1770082923.48
    09:42:03.558  Echoed back to 192.168.237.2:60782
    09:42:03.559  -----------------------
    09:42:03.560
    09:42:04.536  --- Packet Received ---
    09:42:04.537  From: 192.168.237.2:60782
    09:42:04.596
    09:42:04.599  Length: 42
    09:42:04.600  Data: Test Packet 5/5 - Timestamp: 1770082924.52
    09:42:04.601  Echoed back to 192.168.237.2:60782
    09:42:04.602  -----------------------
    09:42:04.603
    09:42:08.827  --- Server Statistics ---
    09:42:08.829  AP SSID: GD32_UDPServer
    09:42:08.831  AP IP: 192.168.237.1
    09:42:08.832  Station Count: 1
    09:42:08.833  Packets Received: 6
    09:42:08.834  Bytes Received: 225
    09:42:08.834  Uptime: 50 seconds
    09:42:08.835  -------------------------
    09:42:08.835
    09:42:18.828  --- Server Statistics ---
    09:42:18.829  AP SSID: GD32_UDPServer
    09:42:18.829  AP IP: 192.168.237.1
    09:42:18.830  Station Count: 1
    09:42:18.830  Packets Received: 6
    09:42:18.830  Bytes Received: 225
    09:42:18.831  Uptime: 60 seconds
    09:42:18.831  -------------------------
    09:42:18.832
    09:42:28.827  --- Server Statistics ---
    09:42:28.828  AP SSID: GD32_UDPServer
    09:42:28.829  AP IP: 192.168.237.1
    09:42:28.829  Station Count: 1
    09:42:28.830  Packets Received: 6
    09:42:28.831  Bytes Received: 225
    09:42:28.831  Uptime: 70 seconds
    09:42:28.831  -------------------------
    09:42:28.832
    09:42:31.706  --- Packet Received ---
    09:42:31.709  From: 192.168.237.2:58881
    09:42:31.711  Length: 27
    09:42:31.712  Data: user data transmission test
    09:42:31.713  Echoed back to 192.168.237.2:58881
    09:42:31.714  -----------------------
    09:42:31.716
    09:42:38.818  --- Server Statistics ---
    09:42:38.821  AP SSID: GD32_UDPServer
    09:42:38.823  AP IP: 192.168.237.1
    09:42:38.825  Station Count: 1
    09:42:38.826  Packets Received: 7
    09:42:38.828  Bytes Received: 252
    09:42:38.829  Uptime: 80 seconds
    09:42:38.830  -------------------------
 */


/*
 * Example Output(use WIFIUDPClient.ino): the same as udp_client.py except packet info:
    10:05:09.239  --- Packet Received ---
    10:05:09.301  From: 192.168.237.4:3333
    10:05:09.306  Length: 30
    10:05:09.309  Data: milliSeconds since boot: 19394
    10:05:09.310  Echoed back to 192.168.237.4:3333
    10:05:09.311  -----------------------
*/

