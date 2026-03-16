/*
 * CommonRTC.ino
 *
 * RTC Basic Example
 *
 * Description:
 * - Demonstrates basic RTC (Real-Time Clock) functionality
 * - Sets initial time and displays current time via serial
 * - Uses interrupt callback for time updates
 *
 * Features:
 * - Set initial UTC time
 * - Second interrupt callback for automatic time update
 * - Serial output of current time
 *
 * Usage:
 * - Modify the timeNow variable to set initial time
 * - Open Serial Monitor to see time output
 * - Time is updated automatically every second via interrupt
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
// set current time: year, month, date, hour, minute, second
UTCTimeStruct timeNow = { 2026, 3, 3, 3, 3, 33 };

void setup()
{
    Serial.println("RTC second interrupt demo");
    Serial.begin(115200);
    rtc.setUTCTime(&timeNow);
    rtc.attachInterrupt(secondIntCallback, INT_SECOND_MODE);
}

void loop()
{
    delay(1000);
    Serial.print(timeNow.year, DEC);
    Serial.print('.');
    Serial.print(timeNow.month, DEC);
    Serial.print('.');
    Serial.print(timeNow.day, DEC);
    Serial.print(' ');
    Serial.print(timeNow.hour, DEC);
    Serial.print(':');
    Serial.print(timeNow.minutes, DEC);
    Serial.print(':');
    Serial.println(timeNow.seconds, DEC);
}

// second interrupt callback function
void secondIntCallback(void)
{
    rtc.getUTCTime(&timeNow);
}
