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

// default I2C pins
#define SDA_PIN 21
#define SCL_PIN 22

//Alternative I2C pins
//#define SDA_PIN 33
//#define SCL_PIN 32

#define EXTERNAL_SENSORS_POLL_INTERVAL 5000

// set the max number of bytes the slave will send.
// if the slave send more bytes, they will still be read
// but the WireSlaveRequest will perform more reads
// until a whole packet is read
#define MAX_SLAVE_RESPONSE_LENGTH 32

long unsigned int nextPollTime = 0;

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
        while (1 < slaveReq.available()) {  // loop through all but the last byte
            char c = slaveReq.read();       // receive byte as a character
            Serial.print(c);                // print the character
        }
        
        int x = slaveReq.read();    // receive byte as an integer
        Serial.println(x);          // print the integer
    }
    else {
        // if something went wrong, print the reason
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

            /*
            //readSlave(address);
            int l = 100;
            Wire.beginTransmission(address);
            Serial.println("Transmission started");
            Wire.requestFrom(address, 5);
            Serial.println("Requested 5");
            while(Wire.available()) {
                char c = Wire.read();
                Serial.print(c);
            }
            Serial.println(" >> finished reading");
            Wire.endTransmission();
            Serial.println("Disconnected");
            */

        }
        else if (error == 4) {
            Serial.print("ERROR: 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.print(address,HEX);
        }    
    }
    if (nDevices == 0) {
        Serial.println("No I2C devices found");
    }
    else {
        Serial.println("");
    }

    readSlave(4);


    // read data slaves
    /* */
}

void sensorsTask(void * pvParameters) {

    Serial.print("Start I2C with SDA=");
    Serial.print(SDA_PIN);
    Serial.print("; SCL=");
    Serial.println(SCL_PIN);
    Wire.begin(SDA_PIN, SCL_PIN);   // join i2c bus
    Serial.println("I2C started");

    while(true) {
        processExternalSensors();
        //delay(1);
    }
}

TaskHandle_t task1;

void startExternalSensors() {

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
