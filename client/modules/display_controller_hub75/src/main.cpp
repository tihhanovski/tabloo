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



#define UARTIO_DEBUG true

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

char* data = nullptr;                 // Buffer for timetable data
size_t dataSize = 0;                  // Size of timetable data
BusStopData timetable;                // Timetable object

void loadData(uint8_t* msg, uint32_t size) {
    dataSize = size;
    data = new char[dataSize];
    memcpy(data, msg, dataSize);

    timetable.initialize(data);
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
            //setTime(msg[0], msg[1], msg[2]);
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
    SerialPort.begin(15200, SERIAL_8N1, 33, 32);
    uartt.setOnMessageReceived(onMessageReceived);

    // Start internal filesystem
    // Settings (and more) is stored here
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
    }

    outputMemoryData();

    matrix_init(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
    matrix_splashscreen();

    startSuccessfull = true;
}

bool b = false;

void loop() {
    if(startSuccessfull)
        matrix_loop(timetable);

    uartt.loop();
    delay(50);
}