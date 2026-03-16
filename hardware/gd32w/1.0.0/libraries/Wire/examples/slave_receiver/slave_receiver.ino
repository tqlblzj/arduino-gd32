/*
 * slave_receiver.ino
 *
 * I2C Slave Receiver Example
 *
 * Description:
 * - Demonstrates I2C slave mode for receiving data from master
 * - Uses interrupt-driven data reception
 * - Displays received data and address match events
 *
 * Features:
 * - I2C slave mode initialization with address 0x55
 * - Data reception callback (receiveEvent)
 * - Address match callback (addrMatchEvent)
 * - Character and byte data display
 *
 * Callbacks:
 * - receiveEvent(int howMany): Called when master sends data
 * - addrMatchEvent(): Called when slave address is matched
 *
 * Expected Data:
 * - String: "hello slave!!!"
 * - Byte: 0x11
 *
 * Usage:
 * - Use with master_writer.ino example on master device
 * - Connect SDA and SCL pins between master and slave
 * - Ensure common ground connection
 *
 * Hardware Requirements:
 * - I2C master device to send data
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
#include <Wire.h>

void setup()
{
    Wire.begin(0x55); // join i2c bus with address 0x55
    Wire.onReceive(receiveEvent);
    Wire.onAddrMatch(addrMatchEvent);
    Serial.begin(115200);
    Serial.println("I2C Slave Receiver Test");
}

void loop()
{
    delay(100);
}

/*
    function that executes whenever data is received from master.
    this function is registered as an event, see TwoWire::begin(uint8_t address).
    master first Wire.write("hello slave!!!") and then Wire.write(0x11);
    so we expect to receive "hello slave!!!" and then 0x11
*/
void receiveEvent(int howMany)
{
    // receive "hello slave!!!"
    while(1 < Wire.available()) {
        char c = Wire.read();
        Serial.print(c);
    }
    Serial.println();
    // receive 0x11
    int x = Wire.read();
    Serial.print("0x");
    Serial.println(x,HEX);
}

/*
    This function is not required by the official Arduino specification;
    it is only intended to allow users to intuitively see that the slave
    address has been successfully matched.
*/
void addrMatchEvent()
{
    Serial.println("Address Matched!");
}