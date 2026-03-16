/*
 * master_reader.ino
 *
 * I2C Master Reader Example
 *
 * Description:
 * - Demonstrates I2C master reading data from a slave device
 * - Requests data from slave and displays received bytes
 *
 * Features:
 * - I2C master mode initialization
 * - Data request from slave using requestFrom()
 * - Received data reading with Wire.read()
 * - Character display of received data
 *
 * Configuration:
 * - Slave Address: 0x55
 * - Request Size: 12 bytes
 *
 * Usage:
 * - Use with slave_sender.ino example on slave device
 * - Connect SDA and SCL pins between master and slave
 * - Ensure common ground connection
 *
 * Hardware Requirements:
 * - I2C slave device configured to send data
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
#include <Wire.h>

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    Serial.println("\nI2C Master Reader Test");
}

void loop()
{
    Serial.println("Request data...");
    byte error, slave_addr=0x55;
    Wire.requestFrom(slave_addr, 12);
    while (Wire.available()) {
        Serial.println((char)(Wire.read()));
    }
    delay(3000);
}
