/*
 * HttpBasicAuthSHA1orUseToken.ino
 *
 * HTTP Authentication with SHA1 or Bearer Token Example
 *
 * Description:
 * - Demonstrates dual authentication methods: Bearer Token or Basic Auth with SHA1
 * - Accepts either "Authorization: Use SecretToken" header or standard Basic Auth
 * - Useful for API access with token or web access with username/password
 *
 * Features:
 * - Bearer Token authentication (SHA1 hashed)
 * - Basic Authentication with SHA1 hashed password
 * - Custom authentication callback supporting both methods
 *
 * Configuration:
 * - Secret token: SecretToken (SHA1: 96451d12082db3f86ab42638cd78df5e96509a65)
 * - Username: admin
 * - Password: gd32 (SHA1: 31999e21fbc68f504a8f6e5a3e6e37731be5c0cc)
 *
 * Usage Examples:
 * - With token: curl -H "Authorization: Use SecretToken" http://<IP>/
 * - With basic auth: curl -u admin:gd32 http://<IP>/
 *
 * Security Note:
 * - SHA1 is considered obsolete for modern applications
 * - Use stronger algorithms (SHA256, SHA512) for production
 *
 * Hardware Requirements:
 * - WiFi network connection
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>
#include <WebServer.h>
#include <SHA1Builder.h>

// We have two options - we either come in with a bearer
// token - i.e. a special header or API token; or we
// get a normal HTTP style basic auth prompt.

// We avoid hardcoding this "SecritToken" into the code by
// using a SHA1 instead (which is not particularly secure).

// Create the secret token SHA1 with:
//        echo -n SecretToken | openssl sha1

String secret_token_hex = "96451d12082db3f86ab42638cd78df5e96509a65";

// Wifi credentials

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

WebServer server(80);

// Rather than specify the admin password as plaintext; we
// provide it as an (unsalted!) SHA1 hash. This is not
// much more secure (SHA1 is past its retirement age,
// and long obsolete/insecure) - but it helps a little.

// The sha1 of 'gd32' (without the trailing \0) expressed as 20
// bytes of hex. Created by for example 'echo -n gd32 | openssl sha1'
// or http://www.sha1-online.com.
const char *www_username_hex = "admin";
const char *www_password_hex = "31999e21fbc68f504a8f6e5a3e6e37731be5c0cc";

static unsigned char _bearer[20];

String *check_token_or_auth(HTTPAuthMethod mode, String authReq, String params[]) {
  // we expect authReq to be "Use SecretToken"
  String lcAuthReq = authReq;
  lcAuthReq.toLowerCase();
  //curl -H "Authorization: Use SecretToken" http://192.168.22.100
  if ((lcAuthReq.startsWith("use "))) {
    String secret = authReq.substring(4);
    secret.trim();

    uint8_t sha1[20];
    SHA1Builder sha_builder;

    sha_builder.begin();
    sha_builder.add((uint8_t *)secret.c_str(), secret.length());
    sha_builder.calculate();
    sha_builder.getBytes(sha1);

    if (memcmp(_bearer, sha1, sizeof(_bearer)) == 0) {
      Serial.println("Bearer token matches");
      return new String("anything non null");
    } else {
      Serial.println("Bearer token does not match");
    }
  } else if (mode == BASIC_AUTH) {  // username:admin    password:gd32
    bool ret = server.authenticateBasicSHA1(www_username_hex, www_password_hex);
    if (ret) {
      Serial.println("Basic auth succeeded");
      return new String(params[0]);
    } else {
      Serial.println("Basic auth failed");
    }
  }

  // No auth found
  return NULL;
};

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

  // Convert token to a convenient binary representation.
  size_t len = HEXBuilder::hex2bytes(_bearer, sizeof(_bearer), secret_token_hex);
  if (len != 20) {
    Serial.println("Bearer token does not look like a valid SHA1 hex string ?!");
  }

  server.on("/", []() {
    if (!server.authenticate(&check_token_or_auth)) {
      Serial.println("No/failed authentication");
      return server.requestAuthentication();
    }
    Serial.println("Authentication succeeded");
    server.send(200, "text/plain", "Login OK");
    return;
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
