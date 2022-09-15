/**
 * Tabloo - opensource bus stop display
 * Main module
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 */

#include <Arduino.h>

#define MATRIX_HUB75_ENABLED true
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32      // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1       // Total number of panels chained one to another

#define SAVED_TIMETABLE_FILE "/timetable.bin"


#define UARTIO_DEBUG true
#define UART_RX 33
#define UART_TX 32

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
            log_v("TODO: command");
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

    // UART
    SerialPort.begin(15200, SERIAL_8N1, UART_RX, UART_TX);
    uartt.setOnMessageReceived(onMessageReceived);
    outputMemoryData();
    matrix_init(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
    matrix_splashscreen();

    // Start internal filesystem
    if(SPIFFS.begin(true)){
        File file = SPIFFS.open(SAVED_TIMETABLE_FILE);
        if(file) {
            log_v("File '%s' opened, available %d", SAVED_TIMETABLE_FILE, file.available());
            dataSize = file.available();
            data = new uint8_t[dataSize];
            file.read(data, dataSize);
            file.close();
            log_v("Timetables data initialized from saved file");
            timetable.initialize((char*)data);
            matrix_startScrolls(timetable);
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
    delay(50);
}