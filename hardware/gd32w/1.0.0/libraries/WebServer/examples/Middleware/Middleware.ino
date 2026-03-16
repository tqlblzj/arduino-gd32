/*
 * Middleware.ino
 *
 * Web Server Middleware Example
 *
 * Description:
 * - Demonstrates middleware usage with WebServer for common request/response processing
 * - Shows CORS, Logging, and Authentication middleware
 * - Middleware can be applied globally or to specific handlers
 *
 * Features:
 * - CORS Middleware: Handles OPTIONS requests and adds CORS headers
 * - Logging Middleware: Logs requests/responses in curl-like format
 * - Authentication Middleware: Digest Authentication for protected routes
 * - Custom middleware support by extending Middleware class
 *
 * Configuration:
 * - AP SSID: GD_Arduino_TestAP
 * - AP Password: 12345678
 * - Auth Username: admin
 * - Auth Password: admin
 *
 * Test Commands:
 * - curl -v -X OPTIONS -H "origin: http://192.168.237.1" http://192.168.237.1/
 * - curl -v -X GET -H "origin: http://192.168.237.1" http://192.168.237.1/
 * - curl -v -X GET -H "origin: http://192.168.237.1" --digest -u admin:admin http://192.168.237.1/protected
 *
 * Hardware Requirements:
 * - WiFi AP mode support
 *
 * Serial Baud Rate: 115200
 */
#include <WiFi.h>
#include <WebServer.h>
#include <Middlewares.h>

// AP configuration
const char* ap_ssid = "GD_Arduino_TestAP";
const char* ap_password = "12345678";
const int ap_channel = 1;              // WiFi channel (1-13)
const int ap_auth_mode = 4;            // AUTH_MODE_WPA_WPA2
const int ap_ssid_hidden = 0;          // 0 = visible, 1 = hidden

WebServer server(80);

LoggingMiddleware logger;
CorsMiddleware cors;
AuthenticationMiddleware auth;

void setup(void) {
  Serial.begin(115200);
    if (WiFi.softAP((char*)ap_ssid, (char*)ap_password, ap_channel, ap_auth_mode, ap_ssid_hidden)) {
        Serial.println("SoftAP started successfully!");
        Serial.println();

        // Print AP information
        Serial.println("AP Information:");
        Serial.print("  SSID: ");
        Serial.println(WiFi.softAPSSID());
        Serial.print("  IP Address: ");
        Serial.println(WiFi.softAPIP().toString());
        Serial.print("  MAC Address: ");
        Serial.println(WiFi.softAPmacAddress());
        Serial.print("  Subnet Mask: ");
        Serial.println(WiFi.softAPSubnetMask().toString());
        Serial.print("  Broadcast IP: ");
        Serial.println(WiFi.softAPBroadcastIP().toString());
        Serial.print("  Network ID: ");
        Serial.println(WiFi.softAPNetworkID().toString());
        Serial.println();

        Serial.println("========================================");
        Serial.println("  AP is ready!");
        Serial.println("  Connect your device to: " + String(ap_ssid));
        Serial.println("  Password: " + String(ap_password));
        Serial.println("========================================");
        Serial.println();
    } else {
        Serial.println("Failed to start SoftAP!");
        Serial.println("Please check your configuration and try again.");
    }

    Serial.println("Monitoring connected stations...");
    Serial.println();

  logger.setOutput(Serial);

  cors.setOrigin("http://192.168.237.1");
  cors.setMethods("POST,GET,OPTIONS,DELETE");
  cors.setHeaders("X-Custom-Header");
  cors.setAllowCredentials(false);
  cors.setMaxAge(600);

  auth.setUsername("admin");
  auth.setPassword("admin");
  auth.setRealm("My Super App");
  auth.setAuthMethod(DIGEST_AUTH);
  auth.setAuthFailureMessage("Authentication Failed");

  server.addMiddleware(&logger);
  server.addMiddleware(&cors);

  // > curl -v -X OPTIONS -H "origin: http://192.168.237.1" http://192.168.237.1/
  // > curl -v -X GET -H "origin: http://192.168.237.1" http://192.168.237.1/
  // > curl -v -X GET -H "origin: http://192.168.237.1"  http://192.168.237.1/protected
  // > curl -v -X GET -H "origin: http://192.168.237.1" --digest -u admin:admin  http://192.168.237.1/protected
  // > curl -v -X GET -H "origin: http://192.168.237.1" http://192.168.237.1/inexsting
  server.on("/", []() {
    server.send(200, "text/plain", "Home");
  });

  server
    .on(
      "/protected",
      []() {
        Serial.println("Request handling...");
        server.send(200, "text/plain", "Protected");
      }
    )
    .addMiddleware(&auth);
  server.onNotFound([]() {
    server.send(404, "text/plain", "Page not found");
  });

  server.collectAllHeaders();
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(10);
}
