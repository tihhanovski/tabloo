/**
 * Tabloo - opensource bus stop display
 * UART Transport
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 */

#ifndef _UARTTRANSPORT_H_
#define _UARTTRANSPORT_H_

#define UART_PACKET_TYPE_TIMETABLE 1
#define UART_PACKET_TYPE_CURRENTTIME 2
#define UART_PACKET_TYPE_COMMANE 3

#include <Arduino.h>
#include <UARTIO.h>

class UARTTransport {
    UARTIO io;

    void (*onMessageReceived)(uint8_t type, uint8_t* msg, uint16_t msgLength) = nullptr; 

public:
    UARTTransport(UARTIO& p) : io(p) {}

    void setOnMessageReceived(void (*handler)(uint8_t type, uint8_t* msg, uint16_t msgLength)) {
        onMessageReceived = handler;
    }

    void loop() {
        if (io.available())
        {
            UARTIO_Message msg;
            io.readMessage(msg);
            if(onMessageReceived != nullptr) {
                uint8_t* msgBody = nullptr;
                uint16_t msgLength = 0;
                uint8_t msgType = 0;
                if(msg.body != nullptr && msg.length > 0) {
                    msgType = *msg.body;
                    msgLength = msg.length - 1;
                    msgBody = new uint8_t[msgLength];
                    memcpy(msgBody, msg.body + 1, msgLength);
                    delete[] msg.body;
                }
                onMessageReceived(msgType, msgBody, msgLength);

                if(msg.body != nullptr)
                    delete[] msgBody;
                msgLength = 0;
                msgType = 0;
            }
        }        
    }

    /**
     * @brief Send message over UART
     * @param msgType type of packet
     * @param msg packet data
     * @param msgLength message length
     */
    void write(uint8_t msgType, uint8_t* msg, uint16_t msgLength) {
        uint8_t* transportMsg = new uint8_t[msgLength + 1];
        *transportMsg = msgType;
        memcpy(transportMsg + 1, msg, (size_t)msgLength);
        io.write(transportMsg, msgLength + 1);
        delete[] transportMsg;
    }


};


#endif