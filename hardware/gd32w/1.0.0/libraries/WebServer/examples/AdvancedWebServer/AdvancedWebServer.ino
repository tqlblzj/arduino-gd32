/*
 * AdvancedWebServer.ino
 *
 * Advanced Web Server Example
 *
 * Description:
 * - Demonstrates advanced web server with multiple route handlers
 * - Serves HTML page with uptime counter and auto-refresh
 * - Generates dynamic SVG graph
 * - Handles 404 errors with detailed information
 *
 * Features:
 * - Root path (/) - HTML page with uptime display
 * - SVG graph generation (/test.svg)
 * - Inline route handler using lambda function
 * - Custom 404 Not Found handler with request details
 * - Multiple route registration with server.on()
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
  char temp[400];
  int sec = millis() / 1000;
  int hr = sec / 3600;
  int min = (sec / 60) % 60;
  sec = sec % 60;

  snprintf(
    temp, 400,

    "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>GD32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from GD32!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

    hr, min, sec
  );
  server.send(200, "text/html", temp);
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
  server.on("/test.svg", drawGraph);
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

void drawGraph() {
  String out = "";
  char temp[200];

  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";

  out += "<defs>\n";
  out += "  <linearGradient id=\"bgGradient\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">\n";
  out += "    <stop offset=\"0%\" style=\"stop-color:#1a5490;stop-opacity:1\" />\n";
  out += "    <stop offset=\"100%\" style=\"stop-color:#0d3a6b;stop-opacity:1\" />\n";
  out += "  </linearGradient>\n";
  out += "</defs>\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"url(#bgGradient)\" />\n";

  out += "<g transform=\"translate(30, 25)\">\n";
  out += "  <rect x=\"0\" y=\"0\" width=\"80\" height=\"100\" rx=\"5\" fill=\"#ffffff\" />\n";
  out += "  <g fill=\"#ffffff\">\n";
  for (int i = 0; i < 6; i++) {
    sprintf(temp, "    <rect x=\"-8\" y=\"%d\" width=\"10\" height=\"6\" />\n", 15 + i * 14);
    out += temp;
  }
  for (int i = 0; i < 6; i++) {
    sprintf(temp, "    <rect x=\"78\" y=\"%d\" width=\"10\" height=\"6\" />\n", 15 + i * 14);
    out += temp;
  }
  out += "  </g>\n";
  out += "  <text x=\"40\" y=\"55\" font-family=\"Arial, sans-serif\" font-size=\"28\" font-weight=\"bold\" fill=\"#1a5490\" text-anchor=\"middle\">GD</text>\n";
  out += "  <rect x=\"15\" y=\"65\" width=\"50\" height=\"20\" fill=\"#1a5490\" opacity=\"0.3\" rx=\"2\" />\n";
  out += "</g>\n";
  out += "<text x=\"130\" y=\"60\" font-family=\"Arial, sans-serif\" font-size=\"32\" font-weight=\"bold\" fill=\"#ffffff\">GigaDevice</text>\n";
  out += "<text x=\"130\" y=\"95\" font-family=\"SimHei, Arial, sans-serif\" font-size=\"24\" fill=\"#ffffff\" opacity=\"0.9\">兆易创新</text>\n";
  out += "<line x1=\"130\" y1=\"110\" x2=\"370\" y2=\"110\" stroke=\"#ffffff\" stroke-width=\"2\" opacity=\"0.5\" />\n";
  out += "<text x=\"250\" y=\"130\" font-family=\"Arial, sans-serif\" font-size=\"12\" fill=\"#ffffff\" opacity=\"0.7\" text-anchor=\"middle\">Making Innovation Easier</text>\n";
  out += "</svg>\n";
  // server.send(200, "image/svg+xml", out);
  server.send_P(200, "image/svg+xml", out.c_str());
}

