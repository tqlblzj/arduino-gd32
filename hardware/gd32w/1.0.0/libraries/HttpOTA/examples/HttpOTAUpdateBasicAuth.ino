/*
 * HttpOTAUpdateBasicAuth.ino
 *
 * HTTP OTA Update with Basic Authentication Example
 *
 * Description:
 * - Demonstrates HTTP OTA firmware updates with basic authentication
 * - Web interface protected by username/password
 * - Secure firmware upload mechanism
 *
 * Features:
 * - Web interface for firmware upload
 * - Basic authentication (username/password protection)
 * - Progress indication during upload
 * - Automatic reboot after successful update
 * - Dual image system (IMAGE_0 and IMAGE_1)
 *
 * Usage:
 * 1. Update WiFi credentials and authentication below
 * 2. Upload this sketch to your device
 * 3. Open browser to http://<device_ip>/
 * 4. Enter username and password when prompted
 * 5. Select firmware .bin file and upload
 * 6. Device will reboot with new firmware
 *
 * Firmware Requirements:
 * - The firmware .bin file should be compiled for the correct memory layout
 * - Use the same compiler flags as the original firmware
 * - Maximum firmware size: ~1.92MB (RE_IMG_1_OFFSET - RE_IMG_0_OFFSET)
 *
 * Recovery from Failed OTA Update:
 * - If you implement your own OTA library and the system fails to boot due to
 *   improper code implementation, you need to recover the device.
 * - After a failed OTA update, the system will attempt to boot from the newly
 *   written image (either IMAGE_0 or IMAGE_1).
 * - If the system tries to boot from IMAGE_0, you can recover by simply
 *   re-flashing via VSCode or Arduino IDE (which writes to IMAGE_0).
 * - If the system tries to boot from IMAGE_1, you MUST recover by dragging and
 *   dropping the corresponding .bin file from the variants directory
 *   (e.g., hardware/gd32w/1.0.0/variants/GD32VW553_START_MINI/image.bin)
 *   onto the device's USB drive. This will force the system to boot from IMAGE_0.
 * - After recovery, you can use VSCode or Arduino IDE to flash new code normally.
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <HttpOTA.h>
#include "ota_html.h"
#include "Middlewares.h"

// WiFi credentials - UPDATE THESE
char *ssid = "HIKVISION_6022";
char *password = "12345678";

const char *www_username = "admin";
const char *www_password = "gd32";

// Web server on port 80
WebServer server(80);

// HttpOTA instance
HttpOTA httpota(server);

// Handle root page - serve HttpOTA upload form
void handleRoot() {
    server.send_P(200, "text/html", httpotaHTML);
}

// Handle system info
void handleInfo() {
    server.send(200, "application/json", httpota.getSystemInfo());
}

// Handle firmware info
void handleFirmwareInfo() {
    server.send(200, "application/json", httpota.getFirmwareInfo());
}

// Handle firmware upload wrapper
void handleFirmwareUpload() {
    httpota.handleFirmwareUpload();
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n============= GD32VW553 HttpOTA Update =============\n");

  // Initialize HttpOTA
  if (!httpota.begin()) {
      Serial.println("HttpOTA initialization failed!");
      return;
  }

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup web server routes
  server.on("/", HTTP_GET, [](){
    if (!server.authenticate(www_username, www_password)) {
      return server.requestAuthentication();
    }
    handleRoot();
  });
  server.on("/info", HTTP_GET, handleInfo);
  server.on("/firmware", HTTP_GET, [](){
    handleFirmwareInfo();
  });

  // Handle firmware upload
  // You can first login in "/" first and then upload .bin
  // or excute like curl -X POST http://192.168.22.100/update -u admin:gd32 -F firmware=@gd_arduino_test.ino.bin
  server.on("/update", HTTP_POST,
      []() {
          // Response is handled in UPLOAD_FILE_END
      },
      [](){
        HTTPUpload &upload = server.upload();
        // Only check authentication at the beginning of upload
        if (upload.status == UPLOAD_FILE_START) {
            if (!server.authenticate("admin", "gd32")) {
                Serial.println("Need auth for uploading!!!");
                server.sendHeader("Connection", "close");
                server.send(401, "text/plain", "Log in http://{localIP}}/ first!!! or excute like curl -X /POST http://{localIP}/update -u admin:gd32 -F firmware=@gd_arduino_test.ino.bin");
                upload.needAuthBeforeUpload = true;
                return;
            }
        }
        handleFirmwareUpload();
      }

  );

  // Start server
  server.begin();
  Serial.println("\nHTTP server started");
  Serial.println("Open http://" + WiFi.localIP().toString() + "/ in your browser");
  Serial.println("System ready for HttpOTA update\n");
}

void loop() {
    server.handleClient();
    delay(2);
}
