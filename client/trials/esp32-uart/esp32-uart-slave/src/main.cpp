#include <Arduino.h>
#include <HardwareSerial.h>

#define UARTIO_DEBUG true

#include <UARTIO.h>
#include <UARTTransport.h>

HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);
UARTTransport transport(io);

boolean b = false;

void onMessageReceived (uint8_t type, uint8_t* msg, uint16_t msgLength) {
    log_i("Received %d bytes, type=%d: '%s'", msgLength, type, msg);
    delete msg;
    b = !b;
    digitalWrite(2, b);
}

void setup()
{
    Serial.begin(115200);
    SerialPort.begin(15200, SERIAL_8N1, 33, 32);
    delay(1000);
    Serial.println("UART slave:");
    pinMode(2, OUTPUT);
    transport.setOnMessageReceived(onMessageReceived);
}

/*
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
*/

void loop()
{
    transport.loop();
    delay(200);
}