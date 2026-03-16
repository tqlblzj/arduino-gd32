/*
 * pwmoutput.ino
 *
 * PWM Output Example
 *
 * Description:
 * - Demonstrates PWM (Pulse Width Modulation) output
 * - Maps sensor values to PWM duty cycle
 * - Outputs varying PWM signals to control brightness or speed
 *
 * Features:
 * - PWM output on PWM0 pin
 * - Value mapping (0-1023 to 0-255)
 * - Serial output for monitoring
 *
 * Hardware Requirements:
 * - LED or other device connected to PWM0 pin
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>

const int analogOutPin = PWM0;

int sensorValue = 0;
int outputValue = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("---------------------------");
}

void loop() {
  sensorValue = 1023;
  outputValue = map(sensorValue, 0, 1023, 0, 255);
  // change the analog out value:
  analogWrite(analogOutPin, outputValue);
  Serial.print("sensor = ");
  Serial.println(sensorValue);
  Serial.print("\t output = ");
  Serial.println(outputValue);

  delay(5000);
  sensorValue = 300;
  outputValue = map(sensorValue, 0, 1023, 0, 255);
  // change the analog out value:
  analogWrite(analogOutPin, outputValue);
  Serial.print("sensor = ");
  Serial.println(sensorValue);
  Serial.print("\t output = ");
  Serial.println(outputValue);

  delay(5000);
}