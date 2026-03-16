/*
 * PathArgServer.ino
 *
 * Web Server with Path Arguments Example
 *
 * Description:
 * - Demonstrates URL path parameter extraction using different URI patterns
 * - Shows both UriBraces (simple placeholders) and UriRegex (regular expressions)
 *
 * Features:
 * - UriBraces: Simple path parameters using {} placeholders
 * - UriRegex: Advanced pattern matching with regular expressions
 * - Path argument extraction with server.pathArg(index)
 *
 * Routes:
 * - / - Returns greeting message
 * - /userID/{id}/userName/{name} - Extracts ID and name using UriBraces
 * - /ID/{id}/Name/{name} - Extracts ID and name using UriRegex
 *
 * Example URLs:
 * - http://<IP>/userID/123/userName/john
 * - http://<IP>/ID/abc123/Name/jane
 *
 * Hardware Requirements:
 * - WiFi network connection
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>

#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

WebServer server(80);

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


  server.on(F("/"), []() {
    server.send(200, "text/plain", "hello from gd32!");
  });

  server.on(UriBraces("/userID/{}/userName/{}"), []() {
    String id = server.pathArg(0);
    String name = server.pathArg(1);
    server.send(200, "text/plain", "UserID: '" + id + "'" + "  UserName: '" + name + "'");
  });

  server.on(UriRegex("^\\/ID\\/([a-zA-Z0-9]+)\\/Name\\/([a-zA-Z0-9]+)$"), []() {
    String id = server.pathArg(0);
    String name = server.pathArg(1);
    server.send(200, "text/plain", "ID: '" + id + "'  Name: '" + name + "'");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(10);
}