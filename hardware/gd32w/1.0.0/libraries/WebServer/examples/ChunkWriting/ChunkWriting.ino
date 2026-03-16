/*
 * ChunkWriting.ino
 *
 * HTTP Chunked Transfer Encoding Example
 *
 * Description:
 * - Demonstrates HTTP chunked transfer encoding for streaming data
 * - Sends countdown data in chunks with 500ms delays
 * - Useful for real-time data streaming or large responses
 *
 * Features:
 * - Chunked response using chunkResponseBegin()/chunkWrite()/chunkResponseEnd()
 * - Real-time countdown from 10 to 1
 * - Progressive data delivery without buffering entire response
 *
 * Hardware Requirements:
 * - WiFi network connection
 *
 * Configuration:
 * - Modify ssid and password variables to match your WiFi network
 * - Server runs on port 80
 * - Access root path (/) to see chunked response
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";
WebServer server(80);

void handleChunks() {
  uint8_t countDown = 10;
  server.chunkResponseBegin();
  char countContent[8];
  while (countDown) {
    sprintf(countContent, "%d...\r\n", countDown--);
    server.chunkWrite(countContent, strlen(countContent));
    delay(500);
  }
  server.chunkWrite("DONE!", 5);
  server.chunkResponseEnd();
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

  server.on("/", handleChunks);

  server.onNotFound([]() {
    server.send(404, "text/plain", "Page not found");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(10);
}
