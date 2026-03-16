/*
 * HelloServer.ino
 *
 * Basic Web Server Example
 *
 * Description:
 * - Demonstrates a simple web server with basic route handlers
 * - Serves "hello from gd32!" message on root path
 * - Includes 404 error handler with request details
 *
 * Features:
 * - Root path (/) handler - returns plain text greeting
 * - Inline route handler using lambda function
 * - Custom 404 Not Found handler
 *
 * Hardware Requirements:
 * - WiFi network connection
 *
 * Configuration:
 * - Modify ssid and password variables to match your WiFi network
 * - Server runs on port 80
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "hello from gd32!");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(10);
}
