/*
 * MD5.ino
 *
 * MD5 Hash Example
 *
 * Description:
 * - Demonstrates MD5 hash calculation for various input types
 * - Tests String, HEX string, and byte array inputs
 * - Validates MD5 hash results
 *
 * Features:
 * - MD5 hash from String
 * - MD5 hash from HEX string
 * - MD5 hash from byte array
 * - Raw byte comparison
 *
 * Serial Baud Rate: 115200
 */

#include <MD5Builder.h>
#include <Arduino.h>

void setup() {
  Serial.begin(115200);

  Serial.println("\n\n\nStart.");

  // Check if a password obfuscated in an MD5 actually
  // matches the original string.
  //
  // echo -n "Hello World" | openssl md5
  {
    String md5 = "b10a8db164e0754105b7a99be72e3fe5";
    String password = "Hello World";

    MD5Builder md;

    md.begin();
    md.add(password);
    md.calculate();

    String result = md.toString();

    if (!md5.equalsIgnoreCase(result)) {
      Serial.println("Odd - failing MD5 on String");
    } else {
      Serial.println("OK!");
    }
  }

  // Check that this also work if we add the password not as
  // a normal string - but as a string with the HEX values.
  {
    String passwordAsHex = "48656c6c6f20576f726c64";
    String md5 = "b10a8db164e0754105b7a99be72e3fe5";

    MD5Builder md;

    md.begin();
    md.addHexString(passwordAsHex);
    md.calculate();

    String result = md.toString();

    if (!md5.equalsIgnoreCase(result)) {
      Serial.println("Odd - failing MD5 on hex string");
      Serial.println(md5);
      Serial.println(result);
    } else {
      Serial.println("OK!");
    }
  }

  // Check that this also work if we add the password as
  // an unsigned byte array.
  {
    uint8_t password[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64};
    String md5 = "b10a8db164e0754105b7a99be72e3fe5";
    MD5Builder md;

    md.begin();
    md.add(password, sizeof(password));
    md.calculate();

    String result = md.toString();

    if (!md5.equalsIgnoreCase(result)) {
      Serial.println("Odd - failing MD5 on byte array");
    } else {
      Serial.println("OK!");
    }

    // And also check that we can compare this as pure, raw, bytes
    uint8_t raw[16] = {0xb1, 0x0a, 0x8d, 0xb1, 0x64, 0xe0, 0x75, 0x41, 0x05, 0xb7, 0xa9, 0x9b, 0xe7, 0x2e, 0x3f, 0xe5};
    uint8_t res[16];
    md.getBytes(res);
    if (memcmp(raw, res, 16)) {
      Serial.println("Odd - failing MD5 on byte array when compared as bytes");
    } else {
      Serial.println("OK!");
    }
  }

  Serial.println("Done.");
}

void loop() {
    delay(1000);
}
