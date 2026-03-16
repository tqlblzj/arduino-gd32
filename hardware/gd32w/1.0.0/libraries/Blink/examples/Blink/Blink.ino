/*
 * Blink.ino
 *
 * LED Blink Example
 *
 * Description:
 * - Blinks the onboard LED (LED0) on and off
 * - LED turns on for 500ms, then off for 500ms
 * - Demonstrates basic digital output control
 *
 * Hardware Requirements:
 * - Onboard LED connected to LED0 pin
 *
 * This is the classic "Hello World" example for Arduino
 */

#include "Arduino.h"

// flash the LED0 500ms
void setup()
{
    pinMode(LED0, OUTPUT);
}

// the loop function runs over and over again forever
void loop()
{
    digitalWrite(LED0, HIGH);
    delay(500);
    digitalWrite(LED0, LOW);
    delay(500);
}
