/*
 * i2c_scanner.ino
 *
 * I2C Device Scanner Example
 *
 * Description:
 * - Demonstrates I2C bus scanning to detect connected devices
 * - Tests for device presence at a specific I2C address
 * - Displays scan results with error codes
 *
 * Features:
 * - I2C device detection at address 0x55
 * - Error code interpretation:
 *   - I2C_OK: Device found and acknowledged
 *   - I2C_NACK_ADDR: No device at address (NACK)
 *   - I2C_BUSY: I2C bus is busy
 *
 * Usage:
 * - Connect I2C devices to the I2C bus (SDA/SCL)
 * - Modify slave_addr variable to scan different addresses
 * - For full range scanning (0x00-0x7F), modify the loop
 *
 * Hardware Requirements:
 * - I2C device(s) connected to SDA/SCL pins
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
#include <Wire.h>

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    Serial.println("\nI2C Scanner");
}

void loop()
{
    byte error, slave_addr=0x55;
    Serial.println("Scanning...");
    Wire.beginTransmission(slave_addr);
    error = Wire.endTransmission();

    if(error == I2C_OK) {
        Serial.print("I2C device found at slave_addr 0x");
        if (slave_addr < 16)
            Serial.print("0");
        Serial.println(slave_addr, HEX);
    }
    else if (error == I2C_NACK_ADDR) {
        Serial.print("Device with slave_addr 0x");
        if (slave_addr < 16)
        Serial.print("0");
        Serial.print(slave_addr, HEX);
        Serial.println(" do not acknowledge (NACK)");
    }
    else if(error == I2C_BUSY) {
        Serial.println("I2C bus is busy");
    }

    delay(3000);
}
