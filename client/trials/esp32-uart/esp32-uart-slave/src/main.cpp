#include <Arduino.h>
#include <HardwareSerial.h>

#define UARTIO_DEBUG true

#include <UARTIO.h>

HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);

boolean b = false;

void setup()
{
    Serial.begin(115200);
    SerialPort.begin(15200, SERIAL_8N1, 33, 32);
    delay(1000);
    Serial.println("UART slave:");
    pinMode(2, OUTPUT);
}

template <class T>
void readNumber(T &val) {
    uint8_t i = sizeof(val);
    val = 0;
    while(i > 0) {
        if(SerialPort.available()) {
            uint8_t b = SerialPort.read();
            if(val)
                val = val << 8;
            if(b)
                val = val & b;
            i--;
        }
    }
}

void loop()
{
    if (SerialPort.available())
    {
        UARTIO_Message msg;
        unsigned long l = millis();
        //char* txt = io.read();

        io.readMessage(msg);

        l = millis() - l;

        log_i("Received %d bytes in %d msec: CRC32=%d", msg.length, l, msg.crc32);

        delete msg.body;

        b = !b;

        digitalWrite(2, b);



    } else {
        delay(200);
    }
}