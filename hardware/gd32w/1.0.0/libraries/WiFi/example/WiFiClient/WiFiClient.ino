/*
 * WiFiClient.ino
 *
 * WiFi HTTP Client Example (ThingSpeak Integration)
 *
 * Description:
 * - Demonstrates HTTP client communication with ThingSpeak API
 * - Sends data to ThingSpeak channel and reads back values
 * - Shows WiFi client connection and HTTP request/response handling
 *
 * Features:
 * - WiFi STA mode connection
 * - HTTP GET/POST requests to ThingSpeak API
 * - Data write to ThingSpeak channel
 * - Data read from ThingSpeak channel
 * - Response parsing and display
 *
 * Configuration Required:
 * - Create a ThingSpeak account at thingspeak.com
 * - Create a new channel and note the Channel ID
 * - Replace channelID with your channel ID
 * - Replace writeApiKey with your Write API Key
 * - Replace readApiKey with your Read API Key
 * - Update ssid and password with your WiFi credentials
 *
 * ThingSpeak API:
 * - Host: api.thingspeak.com
 * - Port: 80
 * - Write: GET /update?api_key=WRITE_KEY&field1=VALUE
 * - Read: GET /channels/CHANNEL_ID/fields/FIELD/last.json
 *
 * Note:
 * - ThingSpeak accepts only integer values
 * - Minimum 15 seconds delay between writes (free account limit)
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>
#include "WiFi.h"
#include "NetworkClient.h"

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

const char *host = "api.thingspeak.com";        // This should not be changed
const int httpPort = 80;                        // This should not be changed
const String channelID = "3234433";             // Change this to your channel ID
const String writeApiKey = "DCA23S10NGI5B7YR";  // IMPORTANT: Change this to YOUR channel's Write API key!
                                                 // The current key may not have write permission for channel 3234433
const String readApiKey = "L9ITN56SI8OIFKIC";   // Change this to your Read API key

// The default example accepts one data filed named "field1"
// For your own server you can of course create more of them.
int field1 = 0;

int numberOfResults = 3;  // Number of results to be read
int fieldNumber = 1;      // Field number which will be read out

void setup() {
    Serial.begin(115200);

    // We start by connecting to a WiFi network

    Serial.println();

    Serial.println("******************************************************");
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("mac address: ");
    Serial.println(WiFi.macAddress());

    WiFi.setPSMode(WIFI_VIF_INDEX_DEFAULT, false); // Disable power save mode

    readWriteHttpData();
}

void readResponse(NetworkClient *client) {
    unsigned long timeout = millis();
    while (client->available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client->stop();
            return;
        }
    }

    // Read all lines of reply from server and print them to Serial
    while (client->available()) {
        String line = client->readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println("\nClosing connection\n");
}

void readWriteHttpData(){

    NetworkClient client;
    String footer = String(" HTTP/1.1\r\n") + "Host: " + String(host) + "\r\n" + "Connection: close\r\n\r\n";

    // WRITE --------------------------------------------------------------------------------------------
    Serial.println("\n=== Attempting to WRITE data to ThingSpeak ===");
    if (!client.connect(host, httpPort)) {
        Serial.println(">>> Failed to connect to server for WRITE!");
        return;
    }
    Serial.println("Connected to server successfully");

    String writeRequest = "GET /update?api_key=" + writeApiKey + "&field1=" + field1 + footer;
    Serial.println("Sending WRITE request:");
    Serial.println(writeRequest);

    client.print(writeRequest);
    readResponse(&client);

    // READ --------------------------------------------------------------------------------------------

    String readRequest = "GET /channels/" + channelID + "/fields/" + fieldNumber + ".json?results=" + numberOfResults + " HTTP/1.1\r\n" + "Host: " + host + "\r\n"
                           + "Connection: close\r\n\r\n";

    Serial.println("\n=== Attempting to READ data from ThingSpeak ===");
    if (!client.connect(host, httpPort)) {
        Serial.println(">>> Failed to connect to server for READ!");
        return;
    }
    Serial.println("Connected to server successfully");

    Serial.println("Sending READ request:");
    Serial.println(readRequest);

    client.print(readRequest);
    readResponse(&client);
    // -------------------------------------------------------------------------------------------------

    ++field1;
    Serial.println("field1 incremented to: " + String(field1));
    Serial.println("Waiting 16 seconds...\n");
    delay(16000);

}
void loop() {
    delay(50);
}