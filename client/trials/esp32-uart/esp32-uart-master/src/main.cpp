#define UARTIO_DEBUG true
#define WIFI_SSID "IK"
#define WIFI_PASSWORD "ProoviM1ngi4uvitauPar0ol2017"
#define STOP_ID "7820162-1"
#define CLOUD_POLL_INTERVAL_SEC 60

#include <Arduino.h>
#include <HardwareSerial.h>
#include <UARTIO.h>
#include <TablooCloud.h>



HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);

String msg ="Sending longer message to slave. Message #";

char* data;                 // Buffer for data retrieved from server
size_t dataSize;            // Size of data

void setup()
{
    Serial.begin(115200);
    SerialPort.begin(15200, SERIAL_8N1, 33, 32);  // 16, 17
    delay(500);

    startConnection(WIFI_SSID, WIFI_PASSWORD);
    //fetchData(STOP_ID, data, dataSize);

}


int cnt = 0;

unsigned long cloudPollNextTime = 0;

void loop()
{
    if(millis() > cloudPollNextTime) {

        size_t uds = 0;

        fetchData(STOP_ID, nullptr, uds, data, dataSize);
        io.write(data, dataSize);


        cloudPollNextTime = millis() + CLOUD_POLL_INTERVAL_SEC * 1000;
    }


    /*
    String s = msg + cnt;
    Serial.print(s);
    unsigned long l = millis();
    io.write(s.c_str());
    l = millis() - l;
    Serial.print(": sent in ");
    Serial.println(l);
    cnt++;
    if(cnt > 200)
        cnt = 0;
    delay(1000);
    */
}