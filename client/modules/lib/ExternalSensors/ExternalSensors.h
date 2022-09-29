// Sensors query
//
// see:
// Wire Master Reader
// by Gutierrez PS <https://github.com/gutierrezps>
// ESP32 I2C slave library: <https://github.com/gutierrezps/ESP32_I2C_Slave>
// based on the example by Nicholas Zambetti <http://www.zambetti.com>
//
// ESP32 I2C Communication: Set Pins, Multiple Bus Interfaces and Peripherals (Arduino IDE)
// by Rui Santos
// https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/



#include <Arduino.h>
#include <Wire.h>
#include <WireSlaveRequest.h>
#include <freertos/stream_buffer.h>
#include <freertos/message_buffer.h>

MessageBufferHandle_t xMessageBuffer;
MessageBufferHandle_t xSensorsList;

// default I2C pins
#define SDA_PIN 21
#define SCL_PIN 22

#define SLAVEDATA_BUFFER_SIZE 256

//Alternative I2C pins
//#define SDA_PIN 33
//#define SCL_PIN 32

#define EXTERNAL_SENSORS_POLL_INTERVAL 5000
#define SENSORS_MAX_COUNT 256

// set the max number of bytes the slave will send.
// if the slave send more bytes, they will still be read
// but the WireSlaveRequest will perform more reads
// until a whole packet is read
#define MAX_SLAVE_RESPONSE_LENGTH 32

long unsigned int nextPollTime = 0;

TaskHandle_t task1;
//MessageBufferHandle_t msgs, bhSensors;

struct SensorRecord {
    uint8_t address;
    uint32_t dataSize;
    uint8_t data[SLAVEDATA_BUFFER_SIZE];
};

SensorRecord sensorData;

uint8_t sensors[SENSORS_MAX_COUNT];
size_t sensorsCount = 0;

void readSlave(byte addr) {
    log_v("Slave %d :", addr);
    // first create a WireSlaveRequest object
    // first argument is the Wire bus the slave is attached to (Wire or Wire1)
    WireSlaveRequest slaveReq(Wire, addr, MAX_SLAVE_RESPONSE_LENGTH);
    // optional: set delay in milliseconds between retry attempts.
    // the default value is 10 ms
    slaveReq.setRetryDelay(5);

    // attempts to read a packet from an ESP32 slave.
    // there's no need to specify how many bytes are requested,
    // since data is expected to be packed with WirePacker,
    // and thus can have any length.
    bool success = slaveReq.request();


    if (success) {
        sensorData.address = addr;
        uint32_t i = 0;
        Serial.print("[");
        while (1 < slaveReq.available()) {  // loop through all but the last byte
            char c = slaveReq.read();       // receive byte as a character
            Serial.print(c);                // print the character
            if(i < SLAVEDATA_BUFFER_SIZE)
                sensorData.data[i++] = c;
            else {
                log_v("!!!ERROR!!! Too many bytes from slave");
                break;
            }
        }
        Serial.println("]");
        sensorData.dataSize = i;

        xMessageBufferSend( xMessageBuffer,
                            ( void * ) &sensorData,
                            i + 5,
                            pdMS_TO_TICKS(500) );

        
    }
    else {
        log_v("Error reading from slave: %s", slaveReq.lastStatusToString());
    }
}

void saveSensorsList(char*& data, size_t dataSize) {
    //TODO - this may fail. Check docs
    xMessageBufferReset(xSensorsList);  // Try to reset buffer

    //First byte is sensors count
    //data + 1 points to start of sensors list
    log_v("Saving list of sensors:");
    for(char* i = data + 1; i < data + dataSize; i++)
        log_v("  sensor addr %d", *i);

    xMessageBufferSend( xSensorsList,       // put sensors data to buffer
        ( void * ) (data + 1),              // pointer to sensors addr list (byte per sensor)
        dataSize - 1,                       // sensors count
        pdMS_TO_TICKS(500) );               // block for 500 msec max
    log_v("sent %d bytes", dataSize - 1);
}

void processExternalSensors() {
    if(nextPollTime > millis())
        return;
    log_v("start");
    nextPollTime = millis() + EXTERNAL_SENSORS_POLL_INTERVAL;

    log_v("sensorsTask running on core %d", xPortGetCoreID());
 
    // update list of sensors from message buffer if buffer is not empty
    // while instead of if just to make sure we read last sensors config
    if(!xMessageBufferIsEmpty(xSensorsList)) {
        log_v("Sensors buffer is not empty, update");
        sensorsCount = xMessageBufferReceive(
            xSensorsList,
            (void *)sensors,
            SENSORS_MAX_COUNT,
            pdMS_TO_TICKS(100));
    } else
        log_v("Sensors buffer is empty, use old list");

    log_v("Sensors count %d", sensorsCount);

    //Read slaves
    if(sensorsCount) {
        log_v("Reading data from sensors:");
        uint8_t* p = sensors;
        for(size_t i = 0; i < sensorsCount; i++)
            readSlave(*(p++));
    }


    log_v("finish");
}

void sensorsTask(void * pvParameters) {

    log_v("Start I2C with SDA=%d, SCL=%d", I2C_SDA, I2C_SCL);
    Wire.begin(I2C_SDA, I2C_SCL);   // join i2c bus
    Serial.println("I2C started");

    while(true) 
        processExternalSensors();
}


void startExternalSensors() {

    xMessageBuffer = xMessageBufferCreate(1024);
    if (xMessageBuffer == NULL) {
        log_e("Cant create message buffer");
        return;
    }

    xSensorsList = xMessageBufferCreate(256);
    if (xSensorsList == NULL) {
        log_e("Cant create sensors list");
        return;
    }

    log_v("Starting task");

    //xTaskCreate()
    //xTaskCreatePinnedToCore
    xTaskCreate(
                    sensorsTask,        /* Task function. */
                    "sensorsTask",      /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    0,           /* priority of the task */
                    &task1      /* Task handle to keep track of created task */
                    );
    log_v("Started task");
}
