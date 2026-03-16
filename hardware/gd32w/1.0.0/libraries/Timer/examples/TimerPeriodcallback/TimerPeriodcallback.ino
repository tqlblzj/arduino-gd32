/*
 * TimerPeriodcallback.ino
 *
 * Timer Period Callback Example
 *
 * Description:
 * - Demonstrates hardware timer with periodic interrupt callback
 * - Configures TIMER1 for 500ms period
 * - Toggles LED state on each timer period update interrupt
 *
 * Features:
 * - Hardware timer period configuration (500ms)
 * - Periodic interrupt callback function
 * - LED state toggling via timer interrupt
 * - Serial output for monitoring callback execution
 *
 * Hardware Requirements:
 * - LED connected to LED0 pin
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"

HardwareTimer Timer_1(TIMER1);
int led_state = 1;
void setup()
{
    Serial.begin(115200);
    pinMode(LED0, OUTPUT);
    digitalWrite(LED0, led_state);
    delay(10000);

    // timer period 500ms
    Timer_1.setPeriodTime(500, FORMAT_MS);
    Timer_1.attachInterrupt(Updatecallback);
    Timer_1.start();
}

// the loop function runs over and over again forever
void loop()
{
    delay(50);
}

// timer period interrupt callback function
void Updatecallback(void)
{
    Serial.print("Updatecallback led_state:");
    Serial.println(led_state);
    digitalWrite(LED0, led_state);
    led_state = 1-led_state;
}