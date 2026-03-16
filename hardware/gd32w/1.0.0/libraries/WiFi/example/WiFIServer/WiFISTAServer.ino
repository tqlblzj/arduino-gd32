/*
 * WiFISTAServer.ino
 *
 * WiFi STA Server Example
 *
 * Description:
 * - Demonstrates a simple TCP server running on WiFi STA mode
 * - Creates a basic HTTP server to control an LED
 * - Shows manual HTTP request parsing and response handling
 *
 * Features:
 * - WiFi STA mode connection
 * - TCP server on port 80
 * - HTTP request parsing
 * - LED control via HTTP GET requests
 * - Manual HTML response generation
 *
 * Routes:
 * - GET /H - Turn LED on
 * - GET /L - Turn LED off
 * - / - Display control page with links
 *
 * Hardware Requirements:
 * - LED connected to LED2 pin
 *
 * Usage:
 * - Connect to WiFi network
 * - Open browser and navigate to device IP
 * - Click links to control LED
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

NetworkServer server(80);

void setup() {
    Serial.begin(115200);
    delay(10);

    pinMode(LED0, OUTPUT);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();
}

void loop() {
    delay(1000);
    NetworkClient client = server.accept();        // listen for incoming clients

    if (client.fd() != -1) {                       // if you get a client,
        Serial.println("New Client.");
        String currentLine = "";
        while (client.connected()) {
        if (client.available()) {                  // if there's bytes to read from the client,
            char c = (char)client.read();          // read a byte, then
            if (c == '\n') {                       // if the byte is a newline character

            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                // and a content-type so the client knows what's coming, then a blank line:
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println();

                // the content of the HTTP response follows the header:
                client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
                client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

                // The HTTP response ends with another blank line:
                client.println();
                // break out of the while loop:
                break;
            } else {  // if you got a newline, then clear currentLine:
                Serial.println(currentLine);
                currentLine = "";
                delay(1000);
            }
            } else if (c != '\r') {  // if you got anything else but a carriage return character,
                currentLine += c;      // add it to the end of the currentLine
            }

            // Check to see if the client request was "GET /H" or "GET /L":
            if (currentLine.endsWith("GET /H")) {
                digitalWrite(LED0, HIGH);  // GET /H turns the LED on, you wiil see the LED light up
                Serial.println("GET /H");
            }
            if (currentLine.endsWith("GET /L")) {
                digitalWrite(LED0, LOW);  // GET /L turns the LED off, you will see the LED turn off
                Serial.println("GET /L");
            }
        }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}