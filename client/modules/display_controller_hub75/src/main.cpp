/**
 * Tabloo - opensource bus stop display
 * Main module
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 */

#include <Arduino.h>

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32      // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 4       // Total number of panels chained one to another

// Chained display setup
#define NUM_ROWS 2
#define NUM_COLS 2
#define SERPENT true
#define TOPDOWN false

// Timetable will be saved to this file
#define SAVED_TIMETABLE_FILE "/timetable.bin"

// #define DEBUG_TIMETABLES true

// Comm setup with main controller
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

const char* CMD_REBOOT = "reboot";

#include "Util.h"           // Misc utilities will be here


// ESP32 internal SD filesystem. Used to store settings etc
#include <SPIFFS.h>

bool startSuccessfull = false;    // True if start was successfull

#include "TablooMatrixPanel.h"

uint8_t* data = nullptr;              // Buffer for timetable data
size_t dataSize = 0;                  // Size of timetable data
BusStopData timetable;                // Timetable object

/**
 * @brief Purge data and free memory
*/
void clearData() {
    if(data != nullptr) {
        delete[] data;
        data = nullptr;
        dataSize = 0;
    }
}

/**
 * @brief initialize timetable and start scrolling
*/
void initData() {
    timetable.initialize((char*)data);
    matrix_startScrolls(timetable);
    outputMemoryData();
}

/**
 * @brief Save arrived data to memory and file, start display of data
*/
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

    initData();
}

/**
 * @brief Command arrived. Execute or display contents
*/
void executeCommand(uint8_t* msg, uint32_t msgLength) {

    // copy command body
    uint8_t* cmdText = new uint8_t[msgLength + 1];
    memcpy(cmdText, msg, msgLength);
    cmdText[msgLength] = 0;
    log_v("command '%s'", cmdText);

    // execute command
    if(!strcmp((const char*)cmdText, CMD_REBOOT)) {
        log_i("will reboot");
        ESP.restart();
        return;
    }

    // it was not command, just output it as a message
    matrix_outputMessage(cmdText, msgLength);

    // cleanup
    delete[] cmdText;
}

/**
 * @brief Called when message from main controller received
 * @param type message type
 * @param msg message body
 * @param msgLength message size
*/
void onMessageReceived (uint8_t type, uint8_t* msg, uint16_t msgLength) {
    log_i("Received message. length=%d bytes, type=%d", msgLength, type);

    switch(type) {
        case UART_PACKET_TYPE_CURRENTTIME:
            if(msgLength != TIME_PACKET_SIZE) {
                log_e("Wrong message length %d", msgLength);
                break;
            }
            log_v("Set current time");
            time_init_by_packet(msg);
            matrix_resetCurrentTime();
            break;
        case UART_PACKET_TYPE_TIMETABLE:
            log_v("New timetable");
            loadData(msg, msgLength);
            break;
        case UART_PACKET_TYPE_COMMAND:
            executeCommand(msg, msgLength);
            break;
        default:
            log_e("Received unknown packet with type=%d", type);
    }
}

void setup() {
    startSuccessfull = false;

    Serial.begin(115200);

    // init UART comm
    SerialPort.begin(UART_SPEED, SERIAL_8N1, UART_RX, UART_TX);
    uartt.setOnMessageReceived(onMessageReceived);
    outputMemoryData();
    matrix_init(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
    matrix_splashscreen();

    // Start internal filesystem and load saved timetable to show right after start
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
                initData();
            } else
                log_e("File %s is empty", SAVED_TIMETABLE_FILE);
        } else
            log_e("Cant open file '%s'", SAVED_TIMETABLE_FILE);
    } else
        log_e("SPIFFS mount failed!");

    startSuccessfull = true;
}

void loop() {
    if(startSuccessfull)
        matrix_loop(timetable);
    uartt.loop();
}