// I2C sensor / actuator code for Tabloo

#ifndef _TABLOO_I2C_TARGET_H_
#define _TABLOO_I2C_TARGET_H_

// default I2C pins
#ifndef SDA_PIN
#define SDA_PIN 21
#endif

#ifndef SCL_PIN 
#define SCL_PIN 22
#endif

#ifndef I2C_TARGET_BUFFER_SIZE
#define I2C_TARGET_BUFFER_SIZE 1024
#endif

#ifndef I2C_ADDR
#define I2C_ADDR 123
#endif


#include <Arduino.h>
#include <Wire.h>
#include <WireSlave.h>
#include <TablooGeneral.h>
#include <TablooTime.h>

uint8_t buffer[I2C_TARGET_BUFFER_SIZE];

void (*i2ctarget_onDataRequested)() = nullptr;
void (*i2ctarget_onCommand)(uint8_t* data, size_t len) = nullptr;


//uint8_t packetTypeReceived = UART_PACKET_TYPE_UNDEFINED;
//bool bDataRequested;

// function that runs whenever the master sends an empty packet.
// this function is registered as an event, see setup().
// do not perform time-consuming tasks inside this function,
// do them elsewhere and simply read the data you wish to
// send inside here.
void i2ctarget_request_event() {

    size_t bytes = 0;
    //Serial.print("From controller: '");
    while (0 < WireSlave.available())
        buffer[bytes++] = WireSlave.read();  // receive byte as a character
    buffer[bytes] = 0;

    //Serial.println("'");
    if(bytes) {
        log_i("Received %d bytes", bytes);
        for(size_t i = 0; i < bytes; i++) {
            Serial.print(buffer[i]);
            Serial.print(" ");
        }
        Serial.println("");

        if(buffer[0] == UART_PACKET_TYPE_CURRENTTIME && bytes == TIME_PACKET_SIZE + 1){
            log_i("Received datetime");
            time_init_by_packet(buffer + 1);
            // setDateTime(buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
        }

        if(buffer[0] == UART_PACKET_TYPE_COMMAND) {
            log_i("Received command '%s' (length=%d)", buffer + 1, bytes - 1);
            if(i2ctarget_onCommand != nullptr)
                i2ctarget_onCommand(buffer + 1, bytes - 1);
        }
    }

    if(!bytes && (i2ctarget_onDataRequested != nullptr)) {
        log_i("Requested data");
        i2ctarget_onDataRequested();
    }
}

void i2ctarget_setup() {
    bool res = WireSlave.begin(SDA_PIN, SCL_PIN, I2C_ADDR);
    if (!res) {
        log_e("I2C target init failed");
        while(1) delay(100);
    }

    WireSlave.onRequest(i2ctarget_request_event);
    log_i("Joined I2C bus with target addr #%d", I2C_ADDR);
}

void i2ctarget_loop() {
    // the slave response time is directly related to how often
    // this update() method is called, so avoid using long delays
    // inside loop(), and be careful with time-consuming tasks
    WireSlave.update();
}


#endif