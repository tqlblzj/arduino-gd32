/*
 * master_writer.ino
 *
 * I2C Master Writer Example
 *
 * Description:
 * - Demonstrates I2C master writing data to a slave device
 * - Sends string and byte data to slave
 * - Displays transmission status and error codes
 *
 * Features:
 * - I2C master mode initialization
 * - Data transmission using beginTransmission()/write()/endTransmission()
 * - Error code interpretation:
 *   - I2C_OK: Data sent successfully
 *   - I2C_DATA_TOO_LONG: Data exceeds buffer size
 *   - I2C_NACK_ADDR: No device at address
 *   - I2C_NACK_DATA: Slave NACK on data
 *   - I2C_TIMEOUT: Bus timeout
 *   - I2C_BUSY: I2C bus is busy
 *
 * Configuration:
 * - Slave Address: 0x55
 * - Data Sent: "hello slave!!!" + 0x11
 *
 * Usage:
 * - Use with slave_receiver.ino example on slave device
 * - Connect SDA and SCL pins between master and slave
 * - Ensure common ground connection
 *
 * Hardware Requirements:
 * - I2C slave device configured to receive data
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
#include <Wire.h>

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    Serial.println("\nI2C Master Writer Test");
}

void loop()
{
    byte error, slave_addr=0x55;
    Wire.beginTransmission(slave_addr);
    Wire.write("hello slave!!!");
    Wire.write(0x11);
    error = Wire.endTransmission();

    if(error == I2C_OK) {
        Serial.print("I2C device found at slave_addr 0x");
        if (slave_addr < 16)
            Serial.print("0");
        Serial.println(slave_addr, HEX);
        Serial.println("Data sent successfully");
    }
    else if(error == I2C_DATA_TOO_LONG) {
        Serial.println("Data too long to fit in transmit buffer");
    }
    else if (error == I2C_NACK_ADDR) {
        Serial.print("Device with slave_addr 0x");
        if (slave_addr < 16)
        Serial.print("0");
        Serial.print(slave_addr, HEX);
        Serial.println(" do not acknowledge (NACK)");
    }
    else if(error == I2C_NACK_DATA) {
        Serial.println("I2C NACK on data");
    }
    else if(error == I2C_TIMEOUT) {
        Serial.println("I2C timeout error");
    }
    else if(error == I2C_BUSY) {
        Serial.println("I2C bus is busy");
    }

    delay(3000);
}
