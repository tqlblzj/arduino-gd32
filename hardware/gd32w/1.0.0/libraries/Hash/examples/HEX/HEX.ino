/*
 * HEX.ino
 *
 * HEX Encoding/Decoding Example
 *
 * Description:
 * - Demonstrates HEX string to binary buffer conversion
 * - Demonstrates binary buffer to HEX string conversion
 * - Tests various HEX encoding scenarios
 *
 * Features:
 * - HEX to bytes conversion (hex2bytes)
 * - Bytes to HEX conversion (bytes2hex)
 * - Case-insensitive HEX comparison
 * - String and buffer handling
 *
 * Serial Baud Rate: 115200
 */

#include <HEXBuilder.h>
#include <Arduino.h>

void setup() {
  Serial.begin(115200);

  Serial.println("\n\n\nStart.");

  // Convert a HEX string like 6c6c6f20576f726c64 to a binary buffer
  {
    const char *out = "Hello World";
    const char *hexin = "48656c6c6f20576f726c6400";  // As the string above is \0 terminated too

    unsigned char buff[256];
    size_t len = HEXBuilder::hex2bytes(buff, sizeof(buff), hexin);

    if (len != 1 + strlen(out)) {
      Serial.println("Odd - length 1 is wrong");
    }

    if (memcmp(buff, out, len) != 0) {
      Serial.println("Odd - decode 1 went wrong");
    }

  };

  {
    String helloHEX = "48656c6c6f20576f726c64";
    const char hello[] = "Hello World";

    unsigned char buff[256];
    size_t len = HEXBuilder::hex2bytes(buff, sizeof(buff), helloHEX);

    if (len != strlen(hello)) {
      Serial.println("Odd - length 2 is wrong");
    }

    if (strcmp((char *)buff, hello) != 0) {
      Serial.println("Odd - decode 2 went wrong");
    }
  }

  {
    const unsigned char helloBytes[] = {0x48, 0x56, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64};
    String helloHEX = "48566c6c6f20576f726c64";

    String out = HEXBuilder::bytes2hex(helloBytes, sizeof(helloBytes));
    if (out.length() != 2 * sizeof(helloBytes)) {
      Serial.println("Odd - length 3 is wrong");
    }

    // we need to ignore case - as a hex string can be spelled in uppercase and lowercase
    if (!out.equalsIgnoreCase(helloHEX)) {
      Serial.println("Odd - decode 3 went wrong");
    }
  }

  {
    const unsigned char helloBytes[] = {0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64};
    const char helloHex[] = "6c6c6f20576f726c64";

    char buff[256];
    size_t len = HEXBuilder::bytes2hex(buff, sizeof(buff), helloBytes, sizeof(helloBytes));
    if (len != 1 + 2 * sizeof(helloBytes)) {
      Serial.println("Odd - length 4 is wrong");
    }

    // we need to ignore case - as a hex string can be spelled in uppercase and lowercase
    if (strcasecmp(buff, helloHex)) {
      Serial.println("Odd - decode 4 went wrong");
    }
  }
  Serial.println("Done.");
}

void loop() {
    delay(1000);
}
