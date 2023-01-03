#include <Arduino.h>


#define DEMO_DELAY 30

#define SAVED_TIMETABLE_FILE "/timetable.bin"
#define SAVED_TIMETABLE_HASH "/timetable.md5"

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

#include "rom/md5_hash.h" // для void md5_test
// #include "mbedtls/md5.h" // для void mbed_tls_md5


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

bool loadFile(const char* filename, uint8_t*& data, size_t& size) {
    File file = SPIFFS.open(filename);
    if(file) {
        size = file.available();
        if(size) {
            log_v("File '%s' opened, available %d", filename, size);
            data = new uint8_t[size];
            file.read(data, size);
            file.close();
            log_v("Loaded %d bytes, file %s is closed", size, filename);
            return true;
        } else
            log_e("File %s is empty", filename);
    } else
        log_e("Cant open file '%s'", filename);
    return false;
}

void saveFile(const char* filename, uint8_t* data, size_t size) {
    log_v("Saving file '%s', %d bytes", filename, size);
    File file = SPIFFS.open(SAVED_TIMETABLE_FILE, FILE_WRITE, true);
    if(file) {
        log_v("File '%s' opened, available %d", SAVED_TIMETABLE_FILE, file.available());
        file.write(data, dataSize);
        file.close();
    } else
        log_e("Cant open file '%s'", SAVED_TIMETABLE_FILE);
    log_v("File '%s' saved");
}

void loadData(uint8_t* msg, uint32_t size) {
    clearData();
    dataSize = size;
    data = new uint8_t[dataSize];
    memcpy(data, msg, dataSize);

    uint8_t* savedHash;
    size_t savedHashLength = 0;
    uint8_t newHash[17] = {0};
    struct MD5Context md5;
    MD5Init(&md5);
    MD5Update(&md5, data, dataSize);
    MD5Final(newHash, &md5);

    if(loadFile(SAVED_TIMETABLE_HASH, savedHash, savedHashLength)) {
        if(!memcmp(savedHash, newHash, 16)) {
            log_v("Saved hash and new hash are equal, ignore changes");
            return;
        }
    }

    saveFile(SAVED_TIMETABLE_HASH, newHash, 16);
    saveFile(SAVED_TIMETABLE_FILE, data, dataSize);

    // File file = SPIFFS.open(SAVED_TIMETABLE_FILE, FILE_WRITE, true);
    // if(file) {
    //     log_v("File '%s' opened, available %d", SAVED_TIMETABLE_FILE, file.available());

    //     file.write(data, dataSize);
    //     file.close();

    // } else
    //     log_e("Cant open file '%s'", SAVED_TIMETABLE_FILE);

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

    if(!strcmp((const char*)cmdText, CMD_REBOOT)) {
        log_i("will reboot");
        ESP.restart();
        return;
    }

    //default: just display it

    display_message(cmdText);

}

void onMessageReceived (uint8_t type, uint8_t* msg, uint16_t msgLength) {
    log_i("Received %d bytes, type=%d", msgLength, type);

    switch(type) {
        case UART_PACKET_TYPE_CURRENTTIME:
            if(msgLength != TIME_PACKET_SIZE) {
                log_e("Wrong message length %d", msgLength);
                break;
            }
            log_v("Setting current time");
            time_init_by_packet(msg);
            break;
        case UART_PACKET_TYPE_TIMETABLE:
            log_v("Timetable received %d bytes", msgLength);
            loadData(msg, msgLength);
            break;
        case UART_PACKET_TYPE_COMMAND:
            //will assume that cmd is a CString
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
        if(loadFile(SAVED_TIMETABLE_FILE, data, dataSize)) {
            log_v("Timetables data initialized from saved file (%d bytes)", dataSize);
            timetable.initialize((char*)data);
            display_showLines(timetable);
        }
    //     File file = SPIFFS.open(SAVED_TIMETABLE_FILE);
    //     if(file) {
    //         dataSize = file.available();
    //         if(dataSize) {
    //             log_v("File '%s' opened, available %d", SAVED_TIMETABLE_FILE, dataSize);
    //             data = new uint8_t[dataSize];
    //             file.read(data, dataSize);
    //             file.close();
    //             log_v("Timetables data initialized from saved file");
    //             timetable.initialize((char*)data);
    //             display_showLines(timetable);
    //         } else
    //             log_e("File %s is empty", SAVED_TIMETABLE_FILE);
    //     } else
    //         log_e("Cant open file '%s'", SAVED_TIMETABLE_FILE);
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