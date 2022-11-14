//Setup service
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
#include <TablooGSM.h>
#include <TablooMQTT.h>
#include <TablooSetup.h>
#include <TablooI2C.h>
#include <time.h>

const char* MQTT_INPUT_SUBTOPIC_TIMETABLE = "timetable";
const char* MQTT_INPUT_SUBTOPIC_TARGETS = "targets";

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
    log_v("Will send to targets: %d.%d.%d %d:%d:%d +%d", p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    if(p[1] > 4) {
        log_v("Send time to I2C targets");
        i2c_write(p, 8);
    } else 
        log_v("Invalid time, wont send it to I2C targets");
}

void onSensorsListReceived(char* data, size_t dataSize) {
    log_v("got new targets list, will save it");
    i2c_save_list(data, dataSize);
    log_v("targets data saved");
    // TODO send data to slave
    sendTimeToTargets();
}

void onCommandReceived(char* data, size_t dataSize) {
    log_v("Command received");
    //first byte of command is address
    switch (data[0])
    {
        case 0:     //command for main controller
            log_i("TODO Command for main controller received: '%s'", data + 1);
            break;
        case 1:     //command for display. Will be sent over UART
            log_v("Command for display");
            uartt.write(UART_PACKET_TYPE_COMMAND, (uint8_t*)(data + 1), dataSize - 1);
            log_v("sent %d bytes", dataSize);
            break;
        default:    //other, I2C target assumed
            byte targetAddr = (byte)data[0];
            data[0] = UART_PACKET_TYPE_COMMAND; //TODO bad style
            log_i("Command for I2C target %d", targetAddr);
            i2c_write(targetAddr, (uint8_t*)(data), dataSize);
            data[0] = targetAddr; //TODO bad style
            break;
    }
}

void onInputReceived(char* topic, char* data, size_t len) {
    log_v("Input received '%s' >> '%s'", topic, data);
    if(!strcmp(topic, MQTT_INPUT_SUBTOPIC_TIMETABLE))
        return onTimetableReceived(data, len);
    if(!strcmp(topic, MQTT_INPUT_SUBTOPIC_TARGETS))
        return onSensorsListReceived(data, len);
    return onCommandReceived(data, len);
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
    log_v("Sync time with mobile network");

    SimpleDateTime time = networking_request_datetime();

    // byte per field (year - 2000)
    // y-m-d-h-m-s-z

    setDateTime(time.year, time.month, time.day, time.hours, time.minutes, time.seconds, time.offset);

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
    if(networking_connect())
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
