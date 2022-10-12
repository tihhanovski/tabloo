// Communication with tabloo sensors and actuators via I2C
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

// default I2C pins
#ifndef SDA_PIN
    #define SDA_PIN 21
#endif

#ifndef SCL_PIN 
#define SCL_PIN 22
#endif


//Alternative I2C pins
//#define SDA_PIN 33
//#define SCL_PIN 32

#define TARGETDATA_BUFFER_SIZE 256
#define EXTERNAL_TARGETS_POLL_INTERVAL 5000
#define TARGETS_MAX_COUNT 256

// set the max number of bytes the slave will send.
// if the slave send more bytes, they will still be read
// but the WireSlaveRequest will perform more reads
// until a whole packet is read
#define MAX_TARGET_RESPONSE_LENGTH 32


#include <Arduino.h>
#include <Wire.h>
#include <WireSlaveRequest.h>
#include <freertos/stream_buffer.h>
#include <freertos/message_buffer.h>
#include <WirePacker.h>

bool i2c_started = false;

MessageBufferHandle_t targetsMessageBuffer;
MessageBufferHandle_t xSensorsList;

SemaphoreHandle_t i2c_talk_semaphore = xSemaphoreCreateMutex();

bool _i2c_takeSemaphore() {
    log_v("taking semaphore");
    return xSemaphoreTake(i2c_talk_semaphore,100) == pdTRUE;
}

void _i2c_giveSemaphore() {
    log_v("giving semaphore");
    xSemaphoreGive(i2c_talk_semaphore);
}

bool i2c_isStarted() {
    if(!i2c_started)
        log_e("I2C not started");
    return i2c_started;
}

long unsigned int nextPollTime = 0;

TaskHandle_t i2c_task_handle;
//MessageBufferHandle_t msgs, bhSensors;

struct SensorRecord {
    uint8_t address;
    uint32_t dataSize;
    uint8_t data[TARGETDATA_BUFFER_SIZE];
};

SensorRecord sensorData;

uint8_t targets[TARGETS_MAX_COUNT];
size_t targetsCount = 0;

void i2c_write(byte addr, uint8_t* msg, size_t msgSize) {
    if(i2c_isStarted()) {
        WirePacker packer;
        for(size_t i = 0; i < msgSize; i++)
            packer.write(msg[i]);
        packer.end();

        Wire.beginTransmission(addr);
        while(packer.available())
            Wire.write(packer.read());
        Wire.endTransmission();
    }
}

void i2c_read(byte addr) {
    if(i2c_isStarted()) {
        log_v("Target %d :", addr);
        WireSlaveRequest req(Wire, addr, MAX_TARGET_RESPONSE_LENGTH);
        req.setRetryDelay(5);

        if (req.request()) {
            sensorData.address = addr;
            uint32_t i = 0;
            Serial.print("[");
            while (1 < req.available()) {  // loop through all but the last byte
                char c = req.read();       // receive byte as a character
                Serial.print(c);                // print the character
                if(i < TARGETDATA_BUFFER_SIZE)
                    sensorData.data[i++] = c;
                else {
                    log_v("!!!ERROR!!! Too many bytes from slave");
                    break;
                }
            }
            Serial.println("]");
            sensorData.dataSize = i;

            xMessageBufferSend( targetsMessageBuffer,
                                ( void * ) &sensorData,
                                i + 5,
                                pdMS_TO_TICKS(500) );
        }
        else 
            log_v("Error reading from slave: %s", req.lastStatusToString());
    }
}

void i2c_save_list(char*& data, size_t dataSize) {
    //TODO - this may fail. Check docs
    xMessageBufferReset(xSensorsList);  // Try to reset buffer

    //First byte is targets count
    //data + 1 points to start of targets list
    log_v("Saving list of targets:");
    for(char* i = data + 1; i < data + dataSize; i++)
        log_v("  sensor addr %d", *i);

    xMessageBufferSend( xSensorsList,       // put targets data to buffer
        ( void * ) (data + 1),              // pointer to targets addr list (byte per sensor)
        dataSize - 1,                       // targets count
        pdMS_TO_TICKS(500) );               // block for 500 msec max
    log_v("sent %d bytes", dataSize - 1);
}

/**
 * @brief Update list of targets from message buffer if buffer is not empty
 */
void i2c_load_list() {
    // while instead of if just to make sure we read last targets config
    if(!xMessageBufferIsEmpty(xSensorsList)) {
        log_v("targets buffer is not empty, update");
        targetsCount = xMessageBufferReceive(
            xSensorsList,
            (void *)targets,
            TARGETS_MAX_COUNT,
            pdMS_TO_TICKS(100));
    } else
        log_v("targets buffer is empty, use old list");
}

void i2c_process_targets() {
    if(nextPollTime > millis())
        return;
    nextPollTime = millis() + EXTERNAL_TARGETS_POLL_INTERVAL;

    log_v("start");
    log_v("targetsTask running on core %d", xPortGetCoreID());

    i2c_load_list();

    if(_i2c_takeSemaphore()) {
        //Read slaves
        if(targetsCount) {
            log_v("Reading data from %d targets:", targetsCount);
            uint8_t* p = targets;
            for(size_t i = 0; i < targetsCount; i++)
                i2c_read(*(p++));
        }
        log_v("finish");
        _i2c_giveSemaphore();
    }
    else
        log_i("Cant take semaphore");
}

void i2c_write(uint8_t* msg, size_t msgSize) {
    i2c_load_list();
    if(_i2c_takeSemaphore()) {
        log_v("Writing data to %d targets:", targetsCount);
        uint8_t* p = targets;
        for(size_t i = 0; i < targetsCount; i++)
            i2c_write(*(p++), msg, msgSize);
        _i2c_giveSemaphore();
    } else
        log_i("Cant take semphore");
    log_v("finish");
}

void i2c_task(void * pvParameters) {
    const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
    log_v("Start I2C with SDA=%d, SCL=%d", I2C_SDA, I2C_SCL);
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.println("I2C started");
    while(true) {
        i2c_process_targets();
        vTaskDelay(xDelay);
    }
}


void i2c_start() {
    targetsMessageBuffer = xMessageBufferCreate(1024);
    if (targetsMessageBuffer == NULL) {
        log_e("Cant create message buffer");
        return;
    }
    xSensorsList = xMessageBufferCreate(256);
    if (xSensorsList == NULL) {
        log_e("Cant create targets list");
        return;
    }
    log_v("Starting task");

    //xTaskCreatePinnedToCore
    xTaskCreate(
                    i2c_task,        /* Task function. */
                    "i2c_task",      /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    0,           /* priority of the task */
                    &i2c_task_handle      /* Task handle to keep track of created task */
                    );
    log_v("Task started");

    i2c_started = true;
}
