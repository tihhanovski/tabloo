//Setup service
#define SETUP_LED_PIN 13            // LED will blink during setup
#define SETUP_BUTTON_PIN 15         // Button to enter setup mode
#define SETUP_BLE_ENABLED false      // Is it possible to setup over BLE (experimental)
#define SETUP_TIMEOUT_PAUSE 60000   // After 60 seconds of idle, will exit setup mode

//MQTT broker setup
#include "mqtt_setup.h"

// non standard UART pins, standard are 16, 17
#define UARTIO_DEBUG true
#define UART_RX 33
#define UART_TX 32
#define UART_SPEED 9600

//I2C
#define I2C_SDA              21
#define I2C_SCL              22
#define TARGETS_POLL_INTERVAL 1000
#define UPLOAD_BUFFER_MAX_SIZE 1024 // Buffer for data retrieved from the modules

#define TIMER_SYNC_INTERVAL 100000  // Will sync RTC every 100 seconds

#include <Arduino.h>
#include <TablooTime.h>             // RCT stuff
#include <TablooGSM.h>              // Network connectivity
#include <TablooMQTT.h>             // MQTT connectivity layer
#include <TablooSetup.h>            // Device setup
#include <TablooI2C.h>              // Connection to modules
#include <TablooMainModule.h>       // Main module common functions


// Old syncTime
// void syncTime() {
//     static unsigned long nextTimeSyncMillis = 0;
//     unsigned long t = millis();
//     if(nextTimeSyncMillis > t)
//         return;
//     nextTimeSyncMillis = t + TIMER_SYNC_INTERVAL;
//     log_v("Sync time with mobile network");

//     SimpleDateTime time = networking_request_datetime();

//     // byte per field (year - 2000)
//     // y-m-d-h-m-s-z

//     setDateTime(time.year, time.month, time.day, time.hours, time.minutes, time.seconds, time.offset);

//     /*
//         log_v("Will config time using NTP");
//         configTime(3 * 3600, 0, "pool.ntp.org");

//         struct tm timeinfo;
//         if(getLocalTime(&timeinfo)) {
//             log_i("local time: %d-%d-%d %d:%d:%d", timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
//         }else{
//             Serial.println("Failed to obtain time");
//         }
//         log_v("Configured time using NTP");
//     */

//     sendTime(time);
// }

void syncTime() {
    static unsigned long nextTimeSyncMillis = 0;
    unsigned long t = millis();
    if(nextTimeSyncMillis > t)
        return;
    nextTimeSyncMillis = t + TIMER_SYNC_INTERVAL;
    log_v("Sync time");
    // SimpleDateTime time = 
    networking_request_datetime();

    sendTime(true, true);
}

void setup() {
    Serial.begin(115200);

    delay(3000);
    // UART
    SerialPort.begin(UART_SPEED, SERIAL_8N1, UART_RX, UART_TX);
    delay(1000);

    setup_start();
    mqtt_onInputReceived = onInputReceived;
    networking_start();
    mqtt_start();

    i2c_start();
}

unsigned long cntTime = 0;

void loop() {
    setup_loop();           // its possible to change device setup 
    mqtt_loop();            // MQTT 
    syncTime();             // get time from mobile network and send changed time to targets
    readDataFromTargets();  // read data from sensors / actuators and publish it to mqtt
}
