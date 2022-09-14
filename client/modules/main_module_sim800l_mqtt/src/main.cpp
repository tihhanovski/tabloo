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
//#define UART_RX 33
//#define UART_TX 32

//#define UART_RX 16
//#define UART_TX 17

#include <HardwareSerial.h>
#include <UARTIO.h>
#include <UARTTransport.h>
#include <BusStopData.h>

HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);
UARTTransport uartt(io);
//char *data;      // Buffer for data retrieved from server
//size_t dataSize; // Size of data


#include <Arduino.h>
//#include <ArduinoHttpClient.h> //https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
#include <TablooGeneral.h>
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

void sendTime(uint8_t h, uint8_t m, uint8_t s) {
    Serial.print("Sending time ");
    Serial.print(h, HEX);
    Serial.print(" : ");
    Serial.print(m, HEX);
    Serial.print(" : ");
    Serial.print(s, HEX);
    Serial.println(" : ");
    uint8_t packet[3] = {h, m, s};
    uartt.write(UART_PACKET_TYPE_CURRENTTIME, packet, 3);
}

unsigned long nextTimeSyncMillis = 0;
void syncTime() {

    unsigned long t = millis();
    if(nextTimeSyncMillis > t)
        return;
    nextTimeSyncMillis = t + 100000;
    SimpleTime time = requestNetworkTime();
    t = millis() - t;
    log_v("Time %s requested in %d msec", format(time), t);

    sendTime(time.hours, time.minutes, time.seconds);
    log_v("Current time pushed to UARTIO");
}

void setup() {
    // UART
    SerialPort.begin(15200, SERIAL_8N1, 33, 32);
    delay(1000);

    sendTime(12, 34, 56);

    Serial.begin(115200);
    setup_start();

    /**/
    mqtt_onTimetableReceived = onTimetableReceived;
    
    startModem();
    ensureConnected();
    mqtt_start();
    /**/


    //TODO requestTime();
}

void publishCount() {
    /*
    Serial.println("Will publish");
    ensureConnected();
    mqtt.publish(mqtt_topic_uptime, ((String)("") + millis()).c_str());
    mqtt.publish(mqtt_topic_device_temp, ((String)("") + ((temprature_sens_read() - 32) / 1.8)).c_str());
    mqtt.publish(mqtt_topic_signal, ((String)("") + modem.getSignalQuality()).c_str());
    Serial.println("Published");
    */
}

unsigned long cntTime = 0;

void loop() {

    //sendTime(12, 34, 56);
    //delay(2000);

    setup_loop();
    mqtt_loop();
    syncTime();

    /*
    if(cntTime < millis()) {
        publishCount();
        cntTime = millis() + 2000;
    }
    */
}
