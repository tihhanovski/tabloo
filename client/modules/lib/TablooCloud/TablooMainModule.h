/**
 * Tabloo - opensource bus stop display
 * Main module utilities
 * There are two flavours of my module: with Wifi and mobile connectivity.
 * It is possible that it is more to come
 * Most of methods of main module are the same
 * To reuse the code, all common methods are moved here
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com> 
 */

#ifndef _TABLOO_MAINMODULE_H_
#define _TABLOO_MAINMODULE_H_

#include <HardwareSerial.h>
#include <UARTIO.h>
#include <UARTTransport.h>
#include <BusStopData.h>
#include <TablooGeneral.h>
#include <TablooTime.h>
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

void sendTime(bool bSendToUART, bool bSendToI2C) {
    if(!time_is_initialized()) {
        log_e("Time is not initialized, will not send");
        return;
    }

    uint8_t packet[TIME_PACKET_SIZE + 1] = {0};
    time_setup_packet(packet + 1);
    packet[0] = UART_PACKET_TYPE_CURRENTTIME;

    //send time to display (UART)
    if(bSendToUART) {
        log_v("Send time to UART");
        uartt.write(UART_PACKET_TYPE_CURRENTTIME, packet + 1, TIME_PACKET_SIZE);
    }

    if(bSendToI2C) {
        log_v("Send time to I2C targets");
        i2c_write(packet, TIME_PACKET_SIZE + 1);
    }
}

void onTargetsListReceived(char* data, size_t dataSize) {
    log_v("got new targets list, will save it");
    log_v("Targets (%d) are: ", dataSize);
    for(uint8_t i = 0; i < dataSize; i++)
        log_v("\t%d", (uint8_t)data[i]);
    i2c_save_list(data, dataSize);
    log_v("targets data saved");
    // TODO send time to targets (not to UART)
    sendTime(false, true);
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

#endif