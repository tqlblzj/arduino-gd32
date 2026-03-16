/*
 * TimerCapturecallback.ino
 *
 * Timer Capture Callback Example
 *
 * Description:
 * - Demonstrates timer capture mode with interrupt callback
 * - Configures TIMER1 for 500ms period with PWM1 channel capture
 * - Toggles LEDs on both rising and falling edges (BOTH_EDGE)
 *
 * Features:
 * - Hardware timer period configuration (500ms)
 * - Capture mode with both edge detection
 * - Interrupt callback function for capture events
 * - LED state toggling via timer interrupt
 *
 * Hardware Requirements:
 * - LEDs connected to LED0 pins
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
    Timer_1.setCaptureMode(PWM0, 0, BOTH_EDGE);
    Timer_1.attachInterrupt(Capturecallback, 0);
    Timer_1.start();
}

// the loop function runs over and over again forever
void loop()
{
    delay(50);
}

void Capturecallback(void)
{
    Serial.print("Capturecallback led_state:");
    Serial.println(led_state);
    digitalWrite(LED0, led_state);
    led_state = 1-led_state;
}