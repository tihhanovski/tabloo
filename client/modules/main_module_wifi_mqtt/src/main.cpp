//Setup service
#define SETUP_LED_PIN 2
#define SETUP_BUTTON_PIN 4

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
#define TARGETS_POLL_INTERVAL 1000

#define UPLOAD_BUFFER_MAX_SIZE 1024

#define TIMER_SYNC_INTERVAL 100000


#include <Arduino.h>
#include <HardwareSerial.h>
#include <UARTIO.h>
#include <UARTTransport.h>
#include <BusStopData.h>
#include <TablooGeneral.h>
#include <TablooTime.h>
#include <TablooWiFi.h>
#include <TablooMQTT.h>
#include <TablooSetup.h>
#include <TablooI2C.h>
#include <time.h>

HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);
UARTTransport uartt(io);

void onTimetableReceived(char* data, size_t dataSize) {
    log_v("got timetable, will send to display");
    uartt.write(UART_PACKET_TYPE_TIMETABLE, (uint8_t*)data, dataSize);
    log_v("sent %d bytes", dataSize);
}

void sendTimeToTargets() {
    if(!isTimeInitialized()) {
        log_v("Time not initialized yet");
        return;
    }
    uint8_t p[8] = {UART_PACKET_TYPE_CURRENTTIME, 0, 0, 0, 0, 0, 0, 0};
    getDateTime(p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    log_i("Will send to targets: %d.%d.%d %d:%d:%d +%d", p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    if(p[1] > 4) {
        log_v("Send time to I2C targets");
        i2c_write(p, 8);
    } else 
        log_v("Invalid time, wont send it to I2C targets");
}

void onTargetsListReceived(char* data, size_t dataSize) {
    log_v("got new targets list, will save it");
    log_v("Targets (%d) are: ", dataSize);
    for(uint8_t i = 0; i < dataSize; i++)
        log_v("\t%d", (uint8_t)data[i]);
    i2c_save_list(data, dataSize);
    log_v("targets data saved");
    // TODO send time to targets
    sendTimeToTargets();
}

void processLocalInput(char* data, size_t len) {
    log_i("Input will be processed '%s': %d bytes", data, len);
    uint8_t type = *data;
    if(type == PACKET_TYPE_TARGETSLIST)
        return onTargetsListReceived(data + 1, len - 1);
    if(type == UART_PACKET_TYPE_COMMAND) {
        // TODO
        log_i("TODO Command for main controller received: '%s'", data + 1);
        return;
    }
}

void sendToUART(char* data, size_t len) {
    log_i("Data for display will be sent via UART");
    uartt.write(*data, (uint8_t*)(data + 1), len - 1);
    log_i("sent %d bytes", len);
}

void sendToTarget(uint8_t target, char* data, size_t len) {
    i2c_write(target, (uint8_t*)(data), len);
}

void onInputReceived(char* topic, char* data, size_t len) {
    if(len < 2) {
        log_e("Received wrong input. Too short");
        return;
    }
    uint8_t target = *data;
    log_i("Input received '%s' >> '%s': %d bytes. Target=%d, type=%d ", topic, data, len, target, *(data + 1));

    if(target == 0) // main controller, self
        return processLocalInput(data + 1, len - 1);

    if(target == 1) // display controller, UART
        return sendToUART(data + 1, len - 1);

    // All other targets considered as I2C
    sendToTarget(*data, data + 1, len - 1);
}

void sendTime(const SimpleDateTime& dt) {
    log_v("Send time to display");
    uint8_t packet[7] = {dt.year, dt.month, dt.day, dt.hours, dt.minutes, dt.seconds, dt.offset};
    uartt.write(UART_PACKET_TYPE_CURRENTTIME, packet, 7);

    sendTimeToTargets();
}

void syncTime() {
    static unsigned long nextTimeSyncMillis = 0;
    unsigned long t = millis();
    if(nextTimeSyncMillis > t)
        return;
    nextTimeSyncMillis = t + TIMER_SYNC_INTERVAL;
    log_v("Sync time");
    SimpleDateTime time = networking_request_datetime();
    sendTime(time);
}

void readDataFromTargets() {
    static unsigned long nextTimeToPollSensors = 0;
    unsigned long t = millis();
    if(t > nextTimeToPollSensors) {
        nextTimeToPollSensors = t + TARGETS_POLL_INTERVAL;
    
        uint8_t uploadBuffer[UPLOAD_BUFFER_MAX_SIZE];
        uint8_t *p = uploadBuffer;
        size_t bytesToUpload = 0;

        if (xMessageBufferIsEmpty(targetsMessageBuffer) != pdTRUE) {
            log_v("loading from buffer");
            size_t received;
            do {
                received = xMessageBufferReceive(
                    targetsMessageBuffer,
                    (void *)p,
                    UPLOAD_BUFFER_MAX_SIZE - bytesToUpload,
                    pdMS_TO_TICKS(100));
                bytesToUpload += received;
                p += received;
            } while(received && bytesToUpload < UPLOAD_BUFFER_MAX_SIZE);

            log_v("Will publish %d bytes of sensor data", bytesToUpload);
            mqtt.publish(mqtt_topic_output, (uint8_t*)uploadBuffer, bytesToUpload, false);
            log_v("Published %d bytes of sensor data", bytesToUpload);
        }
        else
            log_v("No data from targets");
    }
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
