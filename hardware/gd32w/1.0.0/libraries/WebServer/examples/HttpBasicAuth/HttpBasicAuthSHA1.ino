/*
 * HttpBasicAuthSHA1.ino
 *
 * HTTP Basic Authentication with SHA1 Hash Example
 *
 * Description:
 * - Demonstrates HTTP Basic Authentication using SHA1 hashed passwords
 * - Supports cleartext, HEX-encoded SHA1, and Base64-encoded SHA1 passwords
 * - Provides better security than plaintext passwords
 *
 * Features:
 * - Cleartext password authentication (username: admin, password: gd32)
 * - HEX-encoded SHA1 password authentication (username: hexadmin)
 * - Base64-encoded SHA1 password authentication (username: base64admin)
 *
 * Security Note:
 * - SHA1 is considered obsolete and insecure for modern applications
 * - Use stronger hashing algorithms (SHA256, SHA512) for production
 * - Consider using salted hashes for better security
 *
 * Configuration:
 * - Default credentials: admin/gd32
 * - SHA1 of 'gd32': 31999e21fbc68f504a8f6e5a3e6e37731be5c0cc (HEX)
 * - SHA1 of 'gd32': MZmeIfvGj1BKj25aPm43cxvlwMw= (Base64)
 *
 * Hardware Requirements:
 * - WiFi network connection
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>
#include <WebServer.h>

// Rather than specify the password as plaintext; we
// provide it as an (unsalted!) SHA1 hash. This is not
// much more secure (SHA1 is past its retirement age,
// and long obsolete/insecure) - but it helps a little.

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

WebServer server(80);

// Passwords as plaintext - human readable and easily visible in
// the sourcecode and in the firmware/binary.
const char *www_username = "admin";
const char *www_password = "gd32";

// The sha1 of 'gd32' (without the trailing \0) expressed as 20
// bytes of hex. Created by for example 'echo -n gd32 | openssl sha1'
// or http://www.sha1-online.com.
// username:hexadmin  password:gd32
const char *www_username_hex = "hexadmin";
const char *www_password_hex = "31999e21fbc68f504a8f6e5a3e6e37731be5c0cc";

// The same; but now expressed as a base64 string (e.g. as commonly used
// by webservers). Created by ` echo -n gd32 | openssl sha1 -binary | openssl base64`
// username:base64admin  password:gd32
const char *www_username_base64 = "base64admin";
const char *www_password_base64 = "MZmeIfvGj1BKj25aPm43cxvlwMw=";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
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

  server.on("/", []() {
    if (server.authenticate(www_username, www_password)) {
      server.send(200, "text/plain", "Login against cleartext password OK");
      return;
    }
    if (server.authenticateBasicSHA1(www_username_hex, www_password_hex)) {
      server.send(200, "text/plain", "Login against HEX of the SHA1 of the password OK");
      return;
    }
    if (server.authenticateBasicSHA1(www_username_base64, www_password_base64)) {
      server.send(200, "text/plain", "Login against Base64 of the SHA1 of the password OK");
      return;
    }
    Serial.println("No/failed authentication");
    return server.requestAuthentication();
  });

  server.begin();

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
}

void loop() {
  server.handleClient();
  delay(2);  //allow the cpu to switch to other tasks
}
