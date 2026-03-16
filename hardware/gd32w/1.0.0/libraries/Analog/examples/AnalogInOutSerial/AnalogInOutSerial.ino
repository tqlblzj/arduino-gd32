/*
 * AnalogInOutSerial.ino
 *
 * Analog Input/Output Serial Example
 *
 * Description:
 * - Reads an analog input value from ADC_IN pin
 * - Maps the value to PWM output range (0-255)
 * - Outputs the mapped value to PWM0 pin
 * - Displays sensor and output values via serial
 *
 * Hardware Requirements:
 * - Analog sensor connected to ADC_IN pin
 * - LED or other device connected to PWM0 pin
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>

const int analogInPin = ADC_IN;
const int analogOutPin = PWM0;

int sensorValue = 0;
int outputValue = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("---------------------------");
}

void loop() {
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  Serial.print("sensor = ");
  Serial.println(sensorValue);
  // map it to the range of the analog out:
  outputValue = map(sensorValue, 0, 1023, 0, 255);
  // change the analog out value:
  analogWrite(analogOutPin, outputValue);

  Serial.print("\t output = ");
  Serial.println(outputValue);

  delay(1000);
}