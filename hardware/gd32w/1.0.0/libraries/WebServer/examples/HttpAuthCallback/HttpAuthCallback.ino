/*
 * HttpAuthCallback.ino
 *
 * HTTP Authentication with Callback Example
 *
 * Description:
 * - Demonstrates HTTP authentication using custom callback functions
 * - Supports multiple users with username/password pairs
 * - Shows both function pointer and lambda callback approaches
 *
 * Features:
 * - Custom credentials handler function
 * - Multiple user support (admin, daddy, mom, son, daughter)
 * - Lambda-based authentication on /lambda route
 * - Function pointer-based authentication on root route
 * - SVG logo generation for authenticated users
 *
 * Configuration:
 * - Default credentials stored in passwdfile array
 * - Modify passwdfile to add/remove users
 *
 * Hardware Requirements:
 * - WiFi network connection
 *
 * Serial Baud Rate: 115200
 */

#include <WiFi.h>
#include <WebServer.h>

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

WebServer server(80);

const char *www_username = "admin";
const char *www_password = "gd32";

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
  server.send(200, "image/svg+xml", out);
}

typedef struct credentials_t {
  String username;
  String password;
} credentials_t;

credentials_t passwdfile[] = {
  {"admin", "gd32"}, {"daddy", "ve34t3"}, {"mom", "@fveve"}, {"son", "mym9cx"}, {"daughter", "vcnernvvr"},
};
const size_t N_CREDENTIALS = sizeof(passwdfile) / sizeof(credentials_t);

String *credentialsHandler(HTTPAuthMethod mode, String username, String params[]) {
  for (int i = 0; i < N_CREDENTIALS; i++) {
    if (username == passwdfile[i].username) {
      return new String(passwdfile[i].password);
    }
  }
  return NULL;
}

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
    if (!server.authenticate(&credentialsHandler)) {
      return server.requestAuthentication();
    }
    // server.send(200, "text/plain", "Login OK");
    drawGraph();
  });

  server.on("/lambda", []() {
    if (!server.authenticate([](HTTPAuthMethod mode, String username, String params[]) -> String * {
          // Scan the password list for the username and return the password if
          // we find the username.
          //
          for (credentials_t *entry = passwdfile; entry->username; entry++) {
            if (username == entry->username) {
              return new String(entry->password);
            };
          };
          // we've not found the user in the list.
          return NULL;
        })) {
      server.requestAuthentication();
      return;
    }
    // server.send(200, "text/plain", "Login OK");
    drawGraph();
  });

  server.begin();

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
}


void loop() {
  server.handleClient();
  delay(10);
}
