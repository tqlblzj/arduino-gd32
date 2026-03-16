/*
 * slave_sender.ino
 *
 * I2C Slave Sender Example
 *
 * Description:
 * - Demonstrates I2C slave mode for sending data to master
 * - Responds to master read requests with data
 * - Uses interrupt-driven request handling
 *
 * Features:
 * - I2C slave mode initialization with address 0x55
 * - Request callback (requestEvent) for sending data
 * - Address match callback (addrMatchEvent)
 * - Automatic data transmission when master reads
 *
 * Callbacks:
 * - requestEvent(): Called when master requests data
 * - addrMatchEvent(): Called when slave address is matched
 *
 * Data Sent:
 * - String: "hello master!!!"
 *
 * Usage:
 * - Use with master_reader.ino example on master device
 * - Connect SDA and SCL pins between master and slave
 * - Ensure common ground connection
 *
 * Hardware Requirements:
 * - I2C master device to request data
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
#include <Wire.h>

void setup()
{
    Wire.begin(0x55); // join i2c bus with address 0x55 as slave
    Wire.onRequest(requestEvent);
    Wire.onAddrMatch(addrMatchEvent);
    Serial.begin(115200);
    Serial.println("I2C Slave Sender Test");
}

void loop()
{
    delay(100);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see TwoWire::begin(uint8_t address)
void requestEvent()
{
    Serial.println("Request Event Triggered");
    Wire.write("hello master!!!");
}

void addrMatchEvent()
{
    Serial.println("Address Matched!");
}