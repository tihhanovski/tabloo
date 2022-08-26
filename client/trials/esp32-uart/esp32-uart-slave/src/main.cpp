#include <Arduino.h>
#include <HardwareSerial.h>

HardwareSerial SerialPort(2); // use UART2

void setup()
{
    Serial.begin(115200);
    SerialPort.begin(15200, SERIAL_8N1, 33, 32);
    delay(1000);
    Serial.println("UART slave:");
}

void loop()
{
    if (SerialPort.available())
    {
        char number = SerialPort.read();
        Serial.print(number);
    }
}