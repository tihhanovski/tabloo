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
#define UART_SPEED 9600 //15200

//I2C
#define I2C_SDA              21
#define I2C_SCL              22


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
#include <ExternalSensors.h>
#include <time.h>


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

void onCommandReceived(char* data, size_t dataSize) {
    log_v("got command, will send to display");
    uartt.write(UART_PACKET_TYPE_COMMAND, (uint8_t*)data, dataSize);
    log_v("sent %d bytes", dataSize);
}

void onSensorsListReceived(char* data, size_t dataSize) {
    log_v("got new sensors list, will save it");
    saveSensorsList(data, dataSize);
    log_v("Slaves data saved");
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
    log_v("Time %s (request took %d msec)", format(time), t);

    /*
    log_v("Will config time using NTP");
    configTime(3 * 3600, 0, "pool.ntp.org");

    struct tm timeinfo;
    if(getLocalTime(&timeinfo)) {
        log_i("local time: %d-%d-%d %d:%d:%d", timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }else{
        Serial.println("Failed to obtain time");
    }
    log_v("Configured time using NTP");
    */


    sendTime(time);
    log_v("Current time pushed to UARTIO");
}

#define UPLOAD_BUFFER_MAX_SIZE 1024
uint8_t uploadBuffer[UPLOAD_BUFFER_MAX_SIZE];
size_t bytesToUpload;
unsigned long nextTimeToPollSensors = 0;

TaskHandle_t externalSensorsDataUploadTaskHandle;

void externalSensorsDataUploadTask(void *pvParameters)
{
    while(true) {
        unsigned long t = millis();
        if(t > nextTimeToPollSensors) {
            nextTimeToPollSensors = t + 1000;

            uint8_t *p = uploadBuffer;
            bytesToUpload = 0;

            if (xMessageBufferIsEmpty(xMessageBuffer) != pdTRUE)
            {
                while (true)
                {
                    size_t xReceivedBytes;
                    log_v("loading from buffer");
                    xReceivedBytes = xMessageBufferReceive(
                        xMessageBuffer,
                        (void *)p,
                        UPLOAD_BUFFER_MAX_SIZE - bytesToUpload,
                        pdMS_TO_TICKS(100));
                    log_v("received %d bytes", xReceivedBytes);
                    bytesToUpload += xReceivedBytes;
                    if ((xReceivedBytes == 0) || (bytesToUpload >= UPLOAD_BUFFER_MAX_SIZE))
                        break;
                    p = p + xReceivedBytes;
                }

                log_v("Will publish %d bytes of sensor data", bytesToUpload);
                mqtt.publish(mqtt_topic_output, (uint8_t*)uploadBuffer, bytesToUpload, 0);
                log_v("Published %d bytes of sensor data", bytesToUpload);
            }
            else
                log_v("Nothing here from sensors");
        }
    }

}

void setup() {
    delay(3000);
    // UART
    SerialPort.begin(UART_SPEED, SERIAL_8N1, UART_RX, UART_TX);
    delay(1000);

    sendTime(SimpleDateTime());

    Serial.begin(115200);
    setup_start();

    mqtt_onTimetableReceived = onTimetableReceived;
    mqtt_onCommandReceived = onCommandReceived;
    mqtt_onSensorsListReceived = onSensorsListReceived;
    
    startModem();
    if(ensureConnected())
        mqtt_start();

    startExternalSensors();

    // xTaskCreate(
    //     externalSensorsDataUploadTask,          /* Task function. */
    //     "externalSensorsDataUploadTask",        /* name of task. */
    //     10000,                                  /* Stack size of task */
    //     NULL,                                   /* parameter of the task */
    //     1,                                      /* priority of the task */
    //     &externalSensorsDataUploadTaskHandle    /* Task handle to keep track of created task */
    //     );

}

void readSensors() {
    unsigned long t = millis();
    if(t > nextTimeToPollSensors) {
        nextTimeToPollSensors = t + 1000;

        uint8_t *p = uploadBuffer;
        bytesToUpload = 0;

        if (xMessageBufferIsEmpty(xMessageBuffer) != pdTRUE)
        {
            while (true)
            {
                size_t xReceivedBytes;
                log_v("loading from buffer");
                xReceivedBytes = xMessageBufferReceive(
                    xMessageBuffer,
                    (void *)p,
                    UPLOAD_BUFFER_MAX_SIZE - bytesToUpload,
                    pdMS_TO_TICKS(100));
                log_v("received %d bytes", xReceivedBytes);
                bytesToUpload += xReceivedBytes;
                if ((xReceivedBytes == 0) || (bytesToUpload >= UPLOAD_BUFFER_MAX_SIZE))
                    break;
                p = p + xReceivedBytes;
            }

            log_v("Will publish %d bytes of sensor data", bytesToUpload);
            mqtt.publish(mqtt_topic_output, (uint8_t*)uploadBuffer, bytesToUpload, 0);
            log_v("Published %d bytes of sensor data", bytesToUpload);
        }
        else
            log_v("Nothing here from sensors");
    }
}

unsigned long cntTime = 0;

void loop() {
    setup_loop();
    mqtt_loop();
    syncTime();
    readSensors();  // read sensors once per second
}
