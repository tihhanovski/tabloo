#define UARTIO_DEBUG true

#include <Arduino.h>
#include <HardwareSerial.h>
#include <UARTIO.h>
#include <TablooCloud.h>
#include <UARTTransport.h>



HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);
UARTTransport transport(io);

const char* msgMask = "Sending longer message to slave. Message #%d.";

void setup()
{
    Serial.begin(115200);
    SerialPort.begin(15200, SERIAL_8N1, 33, 32);  // 16, 17
    delay(500);
}

int cnt = 0;

unsigned long cloudPollNextTime = 0;

void loop()
{
    uint8_t s[512];
    uint16_t len = sprintf((char*)s, msgMask, cnt);
    log_v("Will send '%s'", s);

    unsigned long l = millis();
    transport.write(0, s, len + 1);

    l = millis() - l;

    log_v("Sent in %d msec", l);

    cnt++;
    if(cnt > 200)
        cnt = 0;
    delay(2000);
}