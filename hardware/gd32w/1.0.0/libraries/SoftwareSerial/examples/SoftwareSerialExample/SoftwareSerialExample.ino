#include  <SoftwareSerial.h>

void setup()
{
    Serial.begin(115200);
    Serial.println("hello arduino");
}

void loop()
{
    delay(1000);
}
