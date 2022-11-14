/**
 * Tabloo - opensource bus stop display
 * Main module
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 */

#include <Arduino.h>

// Wifi debug and OTA
// #define WIFIDEV_SSID "k10-firstfloor"
// #define WIFIDEV_PASS "aPustiKaV1nternet"
// #define WIFIDEV_HOST "display4"
// #define WEBSOCKET_DISABLED
// #include <WifiDev.h>


#define MATRIX_HUB75_ENABLED true
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32      // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 4       // Total number of panels chained one to another

#define NUM_ROWS 2
#define NUM_COLS 2
#define SERPENT true
#define TOPDOWN false

#define SAVED_TIMETABLE_FILE "/timetable.bin"


#define UARTIO_DEBUG true
#define UART_RX 33
#define UART_TX 32
#define UART_SPEED 9600 //15200

#include <HardwareSerial.h>
#include <UARTIO.h>
#include <UARTTransport.h>
#include <TablooGeneral.h>
#include <TablooTime.h>

HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);
UARTTransport uartt(io);


#include "Util.h"           // Misc utilities will be here


// ESP32 internal SD filesystem. Used to store settings etc
#include <SPIFFS.h>

bool startSuccessfull = false;    // True if start was successfull

#include "TablooMatrixPanel.h"

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
    matrix_startScrolls(timetable);
    outputMemoryData();
}

boolean bLedOn = false;

const char* CMD_REBOOT = "display reboot";

void executeCommand(uint8_t* msg, uint32_t msgLength) {
    uint8_t* cmdText = new uint8_t[msgLength + 1];
    memcpy(cmdText, msg, msgLength);
    cmdText[msgLength] = 0;
    log_v("command %s", cmdText);

    display->fillRect(0, 24, 64, 8, DISPLAY_COLOR_BLACK);
    display->setTextColor(DISPLAY_COLOR_RED);
    display->setCursor(0, 24);
    String s = String((char*)cmdText);
    display->print(s);
    display->drawRect(0, 24, 64, 8, DISPLAY_COLOR_RED);

    if(!strcmp((const char*)cmdText, CMD_REBOOT)) {
        log_i("will reboot");
        ESP.restart();
    }


}

void onMessageReceived (uint8_t type, uint8_t* msg, uint16_t msgLength) {
    log_i("Received %d bytes, type=%d: '%s'", msgLength, type, msg);

    switch(type) {
        case UART_PACKET_TYPE_CURRENTTIME:
            if(msgLength < 7) {
                log_e("Wrong message length %d", msgLength);
                break;
            }
            log_v("Setting current time");
            setDateTime(msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6]);
            matrix_resetCurrentTime();
            break;
        case UART_PACKET_TYPE_TIMETABLE:
            log_v("TODO: timetable");
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

void setup() {
    startSuccessfull = false;

    Serial.begin(115200);

    // WifiDev
    // wifidev_setup(WIFIDEV_SSID, WIFIDEV_PASS, WIFIDEV_HOST);

    // UART
    SerialPort.begin(UART_SPEED, SERIAL_8N1, UART_RX, UART_TX);
    uartt.setOnMessageReceived(onMessageReceived);
    outputMemoryData();
    matrix_init(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
    matrix_splashscreen();

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
                matrix_startScrolls(timetable);
            } else
                log_e("File %s is empty", SAVED_TIMETABLE_FILE);
        } else
            log_e("Cant open file '%s'", SAVED_TIMETABLE_FILE);
    } else
        log_e("SPIFFS mount failed!");

    startSuccessfull = true;
}

bool b = false;

void loop() {
    if(startSuccessfull)
        matrix_loop(timetable);

    uartt.loop();

    // WifiDev
    // wifidev_loop();


    //delay(50);
}