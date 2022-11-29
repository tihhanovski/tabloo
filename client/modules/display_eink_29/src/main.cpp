#include <Arduino.h>


#define DEMO_DELAY 30

#define SAVED_TIMETABLE_FILE "/timetable.bin"
#define UARTIO_DEBUG true
#define UART_RX 33
#define UART_TX 32
#define UART_SPEED 9600 //15200
#define SERIAL_SPEED 115200

#include <HardwareSerial.h>
#include <UARTIO.h>
#include <UARTTransport.h>
#include <TablooGeneral.h>
#include <TablooTime.h>
#include <BusStopData.h>
#include "eink.h"
#include <SPIFFS.h>               // ESP32 internal SD filesystem. Used to store settings etc


HardwareSerial SerialPort(2); // use UART2
UARTIO uartio(SerialPort);
UARTTransport uartt(uartio);
bool startSuccessfull = false;    // True if start was successfull
uint8_t* data = nullptr;                 // Buffer for timetable data
size_t dataSize = 0;                  // Size of timetable data
BusStopData timetable;                // Timetable object

void clearData() {
    if(data != nullptr) {
        delete[] data;
        data = nullptr;
        dataSize = 0;
    }
}

void loadData(uint8_t* msg, uint32_t size) {
    clearData();
    dataSize = size;
    data = new uint8_t[dataSize];
    memcpy(data, msg, dataSize);

    File file = SPIFFS.open(SAVED_TIMETABLE_FILE, FILE_WRITE, true);
    if(file) {
        log_v("File '%s' opened, available %d", SAVED_TIMETABLE_FILE, file.available());

        file.write(data, dataSize);
        file.close();

    } else
        log_e("Cant open file '%s'", SAVED_TIMETABLE_FILE);

    timetable.initialize((char*)data);
    display_showLines(timetable);
}

boolean bLedOn = false;

const char* CMD_REBOOT = "display reboot";

void executeCommand(uint8_t* msg, uint32_t msgLength) {
    uint8_t* cmdText = new uint8_t[msgLength + 1];
    memcpy(cmdText, msg, msgLength);
    cmdText[msgLength] = 0;
    log_v("command %s", cmdText);

    // display->fillRect(0, 24, 64, 8, DISPLAY_COLOR_BLACK);
    // display->setTextColor(DISPLAY_COLOR_RED);
    // display->setCursor(0, 24);
    // String s = String((char*)cmdText);
    // display->print(s);
    // display->drawRect(0, 24, 64, 8, DISPLAY_COLOR_RED);

    if(!strcmp((const char*)cmdText, CMD_REBOOT)) {
        log_i("will reboot");
        ESP.restart();
    }
}

void onMessageReceived (uint8_t type, uint8_t* msg, uint16_t msgLength) {
    log_i("Received %d bytes, type=%d: '%s'", msgLength, type, msg);

    switch(type) {
        case UART_PACKET_TYPE_CURRENTTIME:
            if(msgLength != TIME_PACKET_SIZE) {
                log_e("Wrong message length %d", msgLength);
                break;
            }
            log_v("Setting current time");
            // setDateTime(msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6]);
            // TODO matrix_resetCurrentTime();
            time_init_by_packet(msg);
            break;
        case UART_PACKET_TYPE_TIMETABLE:
            log_v("Timetable received %d bytes", msgLength);
            loadData(msg, msgLength);
            break;
        case UART_PACKET_TYPE_COMMAND:
            //will assume that cmd is c string
            executeCommand(msg, msgLength);
            break;
        default:
            log_e("Received unknown packet with type=%d", type);
    }

    delete msg;
    bLedOn = !bLedOn;
    digitalWrite(2, bLedOn);
}


void setup()
{
    Serial.begin(SERIAL_SPEED);
    Serial.println();
    Serial.println("setup");
    display.init(SERIAL_SPEED); // enable diagnostic output on Serial

    display_splashscreen();

    startSuccessfull = false;
    SerialPort.begin(UART_SPEED, SERIAL_8N1, UART_RX, UART_TX);
    uartt.setOnMessageReceived(onMessageReceived);


    // Start internal filesystem
    if(SPIFFS.begin(true)){
        File file = SPIFFS.open(SAVED_TIMETABLE_FILE);
        if(file) {
            dataSize = file.available();
            if(dataSize) {
                log_v("File '%s' opened, available %d", SAVED_TIMETABLE_FILE, dataSize);
                data = new uint8_t[dataSize];
                file.read(data, dataSize);
                file.close();
                log_v("Timetables data initialized from saved file");
                timetable.initialize((char*)data);
                display_showLines(timetable);
            } else
                log_e("File %s is empty", SAVED_TIMETABLE_FILE);
        } else
            log_e("Cant open file '%s'", SAVED_TIMETABLE_FILE);
    } else
        log_e("SPIFFS mount failed!");

    startSuccessfull = true;
}

void loop()
{
    if(startSuccessfull)
        display_loop(timetable);
    uartt.loop();

}