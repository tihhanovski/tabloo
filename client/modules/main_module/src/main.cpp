#define UARTIO_DEBUG false
//#define WIFI_SSID "IK"
//#define WIFI_PASSWORD "ProoviM1ngi4uvitauPar0ol2017"

#define WIFI_SSID "k10-firstfloor"
#define WIFI_PASSWORD "aPustiKaV1nternet"

#define STOP_ID "7820162-1"
#define CLOUD_POLL_INTERVAL_SEC 60
#define UPLOAD_BUFFER_MAX_SIZE 1024

// non standard UART pins, standard are 16, 17
#define UART_RX 33
#define UART_TX 32

#include <Arduino.h>
#include <HardwareSerial.h>
#include <UARTIO.h>
#include <TablooCloud.h>
#include <freertos/stream_buffer.h>
#include <freertos/message_buffer.h>

#include "ExternalSensors.h"
#include <BusStopData.h>

MessageBufferHandle_t xMessageBuffer;
MessageBufferHandle_t xSensorsList;

HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);

BusStopData busStopData;
char *data;      // Buffer for data retrieved from server
size_t dataSize; // Size of data
unsigned long cloudPollNextTime = 0;

char uploadBuffer[UPLOAD_BUFFER_MAX_SIZE];
size_t bytesToUpload;

void buildDataToUpload()
{
    char *p = uploadBuffer;
    bytesToUpload = 0;

    if (xMessageBufferIsEmpty(xMessageBuffer) != pdTRUE)
    {
        while (true)
        {
            size_t xReceivedBytes;
            Serial.println("loading from buffer");
            xReceivedBytes = xMessageBufferReceive(
                xMessageBuffer,
                (void *)p,
                UPLOAD_BUFFER_MAX_SIZE - bytesToUpload,
                pdMS_TO_TICKS(100));
            Serial.print("received ");
            Serial.println(xReceivedBytes);
            bytesToUpload += xReceivedBytes;
            if ((xReceivedBytes == 0) || (bytesToUpload >= UPLOAD_BUFFER_MAX_SIZE))
                break;
            p = p + xReceivedBytes;
        }
    }
    else
        Serial.println("Buffer to upload is empty");

    Serial.print("Will upload ");
    Serial.print(bytesToUpload);
    Serial.println(" bytes of sensor data");
}

void communicationTask(void *pvParameters)
{
    SerialPort.begin(15200, SERIAL_8N1, UART_RX, UART_TX);
    delay(500);
    startConnection(WIFI_SSID, WIFI_PASSWORD);

    while (true)
    {
        if (millis() > cloudPollNextTime)
        {
            digitalWrite(2, 1);
            Serial.print("communicationTask running on core ");
            Serial.println(xPortGetCoreID());

            //Load sensors data from buffer
            buildDataToUpload();

            // download data from server
            fetchData(STOP_ID, uploadBuffer, bytesToUpload, data, dataSize);

            // push data as is to display
            io.write(data, dataSize);

            busStopData.initialize(data);
            Serial.print("Sensors count ");
            Serial.println(busStopData.sensorCount);
            for(uint8_t i = 0; i < busStopData.sensorCount; i++) {
                Serial.print(*(busStopData.sensors + i), HEX);
                Serial.print("; ");
            }

            //TODO - this may fail. Check docs
            xMessageBufferReset(xSensorsList);  // Try to reset buffer

            xMessageBufferSend( xSensorsList,   // put sensors data to buffer
                ( void * ) busStopData.sensors, // pointer to sensors addr list (byte per sensor)
                busStopData.sensorCount,        // sensors count
                pdMS_TO_TICKS(500) );           // block for 500 msec max

            Serial.println(" saved.");

            cloudPollNextTime = millis() + CLOUD_POLL_INTERVAL_SEC * 1000;
            digitalWrite(2, 0);
        }
    }
}

TaskHandle_t communicationTaskHandle;

void startCommunications() {
    xTaskCreatePinnedToCore(
        communicationTask,        /* Task function. */
        "communicationTask",      /* name of task. */
        10000,                    /* Stack size of task */
        NULL,                     /* parameter of the task */
        1,                        /* priority of the task */
        &communicationTaskHandle, /* Task handle to keep track of created task */
        1);                       /* pin task to core 1 */
}

void setup()
{
    Serial.begin(115200);

    pinMode(2, OUTPUT);

    xMessageBuffer = xMessageBufferCreate(1024);
    if (xMessageBuffer == NULL)
        Serial.println("ERROR!!! Cant create message buffer");

    xSensorsList = xMessageBufferCreate(256);
    if (xSensorsList == NULL)
        Serial.println("ERROR!!! Cant create sensors list");

    startCommunications();
    startExternalSensors(xMessageBuffer, xSensorsList);
}

void loop()
{
}