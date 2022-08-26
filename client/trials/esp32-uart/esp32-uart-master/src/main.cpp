#include <Arduino.h>
#include <HardwareSerial.h>

HardwareSerial SerialPort(2); // use UART2

void setup()
{
    Serial.begin(115200);
    // put your setup code here, to run once:
    SerialPort.begin(15200, SERIAL_8N1, 16, 17);
    delay(500);
}

int cnt = 0;

void loop()
{
    Serial.print("sending ");
    Serial.println(cnt);
    SerialPort.print("From master ");
    SerialPort.println(cnt);

    cnt++;
    delay(1000);
}