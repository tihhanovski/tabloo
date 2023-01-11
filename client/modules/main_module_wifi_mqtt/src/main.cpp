//Setup
#define SETUP_LED_PIN 2             // LED will blink during setup
#define SETUP_BUTTON_PIN 4          // Button to enter setup mode
#define SETUP_BLE_ENABLED true      // Is it possible to setup over BLE (experimental)
#define SETUP_TIMEOUT_PAUSE 60000   // After 60 seconds of idle, will exit setup mode

//MQTT broker setup
#include "mqtt_setup.h"

// non standard UART pins, standard are 16, 17
#define UARTIO_DEBUG true
#define UART_RX 33
#define UART_TX 32
#define UART_SPEED 9600

//I2C
#define I2C_SDA 21
#define I2C_SCL 22
#define TARGETS_POLL_INTERVAL 1000
#define UPLOAD_BUFFER_MAX_SIZE 1024 // Buffer for data retrieved from the modules

#define TIMER_SYNC_INTERVAL 100000  // Will sync RTC every 100 seconds

#include <Arduino.h>
#include <TablooTime.h>             // RCT stuff
#include <TablooWiFi.h>             // Network connectivity
#include <TablooMQTT.h>             // MQTT connectivity layer
#include <TablooSetup.h>            // Device setup
#include <TablooI2C.h>              // Connection to modules
#include <TablooMainModule.h>       // Main module common functions

/**
 * Sync time with NTP every TIMER_SYNC_INTERVAL msec
*/
void syncTime() {
    static unsigned long nextTimeSyncMillis = 0;
    unsigned long t = millis();
    if(nextTimeSyncMillis > t)
        return;
    nextTimeSyncMillis = t + TIMER_SYNC_INTERVAL;
    log_v("Sync time");

    networking_request_datetime();

    sendTime(true, true);
}

void setup() {
    Serial.begin(115200);
    delay(3000);

    // Start UART (to communicate with display module)
    SerialPort.begin(UART_SPEED, SERIAL_8N1, UART_RX, UART_TX);
    delay(1000);

    //Start setup service
    setup_start();

    //Start networking
    mqtt_onInputReceived = onInputReceived;
    networking_start();
    mqtt_start();

    //Start communication with modules
    i2c_start();
}

void loop() {
    setup_loop();           // its possible to change device setup 
    mqtt_loop();            // MQTT 
    syncTime();             // get time from mobile network and send changed time to targets
    readDataFromTargets();  // read data from sensors / actuators and publish it to mqtt
}

