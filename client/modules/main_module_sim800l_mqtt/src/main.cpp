// Sample code for https://www.aliexpress.com/item/4000456922720.html?spm=a2g0s.12269583.0.0.75154f39xjmZMx
// Based on example from TinyGSM by Volodymyr Shymanskyy 
// https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
// 

//TTGO T-Call pins
#define SETUP_LED_PIN 13
#define SETUP_BUTTON_PIN 15

//MQTT broker data
#define MQTT_BROKER "dev.intellisoft.ee"
#define MQTT_USER "ilja"
#define MQTT_PASS "inf471c"
#define MQTT_PORT 8883

// non standard UART pins, standard are 16, 17
#define UARTIO_DEBUG true
#define UART_RX 33
#define UART_TX 32

#include <HardwareSerial.h>
#include <UARTIO.h>
#include <UARTTransport.h>
#include <BusStopData.h>

HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);
UARTTransport uartt(io);

#include <Arduino.h>
#include <TablooGeneral.h>
#include <TablooTime.h>
#include <TablooGSM.h>
#include <TablooMQTT.h>
#include <TablooSetup.h>


#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();


void onTimetableReceived(char* data, size_t dataSize) {
    log_v("got timetable, will send to display");
    uartt.write(UART_PACKET_TYPE_TIMETABLE, (uint8_t*)data, dataSize);
    log_v("sent %d bytes", dataSize);
}

void sendTime(SimpleDateTime dt) {
    uint8_t packet[7] = {dt.year, dt.month, dt.day, dt.hours, dt.minutes, dt.seconds, dt.offset};
    uartt.write(UART_PACKET_TYPE_CURRENTTIME, packet, 7);
}

unsigned long nextTimeSyncMillis = 0;
void syncTime() {

    // byte per field (year - 2000)
    // y-m-d-h-m-s-z

    unsigned long t = millis();
    if(nextTimeSyncMillis > t)
        return;
    nextTimeSyncMillis = t + 100000;
    SimpleDateTime time = requestNetworkDateTime();
    t = millis() - t;
    log_v("Time %s requested in %d msec", format(time), t);

    sendTime(time);
    log_v("Current time pushed to UARTIO");
}

void setup() {
    // UART
    SerialPort.begin(15200, SERIAL_8N1, UART_RX, UART_TX);
    delay(1000);

    sendTime(SimpleDateTime());

    Serial.begin(115200);
    setup_start();

    mqtt_onTimetableReceived = onTimetableReceived;
    
    startModem();
    ensureConnected();
    mqtt_start();
}

unsigned long cntTime = 0;

void loop() {
    setup_loop();
    mqtt_loop();
    syncTime();
}
