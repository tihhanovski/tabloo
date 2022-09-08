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
MessageBufferHandle_t msgs, bhSensors;

struct SensorRecord {
    uint8_t address;
    uint32_t dataSize;
    uint8_t data[SLAVEDATA_BUFFER_SIZE];
};

SensorRecord sensorData;

uint8_t sensors[SENSORS_MAX_COUNT];
size_t sensorsCount = 0;

void readSlave(byte addr) {
    Serial.print("Slave ");
    Serial.print(addr);
    Serial.print(": ");
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
        while (1 < slaveReq.available()) {  // loop through all but the last byte
            char c = slaveReq.read();       // receive byte as a character
            Serial.print(c);                // print the character
            if(i < SLAVEDATA_BUFFER_SIZE)
                sensorData.data[i++] = c;
            else 
                Serial.println("!!!ERROR!!! Too much data from slave");
        }
        sensorData.dataSize = i;

        xMessageBufferSend( msgs,
                            ( void * ) &sensorData,
                            i + 5,
                            pdMS_TO_TICKS(500) );

        
    }
    else {
        Serial.println(slaveReq.lastStatusToString());
    }
}


void processExternalSensors() {
    if(nextPollTime > millis())
        return;
    nextPollTime = millis() + EXTERNAL_SENSORS_POLL_INTERVAL;

    Serial.print("sensorsTask running on core ");
    Serial.println(xPortGetCoreID());

    // get list of slaves
    /*
    // modified code by Rui Santos 
    // see https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/
    byte error, address;
    int nDevices;
    Serial.print("Scanning: ");
    nDevices = 0;
    for(address = 1; address < 127; address++ ) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
            if (address<16) {
                Serial.print("0");
            }
            Serial.print(address,HEX);
            Serial.print("; ");
            nDevices++;
        }
        else if (error == 4) {
            Serial.print("ERROR: 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.print(address,HEX);
        }    
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found");
    else
        Serial.println("");
    */
 
    // update list of sensors from message buffer if buffer is not empty
    // while instead of if just to make sure we read last sensors config
    if(!xMessageBufferIsEmpty(bhSensors)) {
        Serial.println("Sensors buffer is not empty, update");
        sensorsCount = xMessageBufferReceive(
            bhSensors,
            (void *)sensors,
            SENSORS_MAX_COUNT,
            pdMS_TO_TICKS(100));
    } else
        Serial.println("Sensors buffer is empty, use old list");

    Serial.print("Sensors count ");
    Serial.println(sensorsCount);

    //Read slaves
    uint8_t* p = sensors;
    for(size_t i = 0; i < sensorsCount; i++)
        readSlave(*(p++));

    //Wire.end();
}

void sensorsTask(void * pvParameters) {

    Serial.print("Start I2C with SDA=");
    Serial.print(SDA_PIN);
    Serial.print("; SCL=");
    Serial.println(SCL_PIN);
    Wire.begin(SDA_PIN, SCL_PIN);   // join i2c bus
    Serial.println("I2C started");

    while(true) 
        processExternalSensors();
}


void startExternalSensors(MessageBufferHandle_t buffer, MessageBufferHandle_t sensorsList) {

    msgs = buffer;
    bhSensors = sensorsList;

    Serial.println("Starting task on core 0");
    xTaskCreatePinnedToCore(
                    sensorsTask,        /* Task function. */
                    "sensorsTask",      /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
    Serial.println("Task started on core 0");
}
