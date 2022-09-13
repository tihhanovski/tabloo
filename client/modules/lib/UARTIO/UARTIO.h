// UART message delivery

#include <Arduino.h>
#include <HardwareSerial.h>
#include <CRC32.h>

#define UARTIO_TYPE_NONE 0
#define UARTIO_TYPE_MESSAGE 2
#define UARTIO_TYPE_ANSWER 3
#define UARTIO_ESC 0
#define UARTIO_MAXBODYLENGTH 1024
#define UARTIO_ANSWER_TIMEOUT 5000
#define UARTIO_WRITE_RETRIES 3
#define UARTIO_DELAY_BETWEEN_WRITE_RETRIES 200

#ifndef UARTIO_DEBUG
#define UARTIO_DEBUG false
#endif

#if UARTIO_DEBUG
#define logToSerial Serial.print
#define logToSerialHex dbgOutputHex
#else
#define logToSerial //Serial.print
#define logToSerialHex //dbgOutputHex
#endif

class UARTIO_Message {
    public:
        uint8_t type;
        uint32_t crc32;
        uint32_t length;
        char* body;
};

class UARTIO {
    HardwareSerial& port;

    #ifdef UARTIO_DEBUG
    void dbgOutputHex(uint8_t b) {
        if(b < 16)
            Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
    }
    #endif

    void writeByte(uint8_t b) {
        port.write(b);
        logToSerialHex(b);
    }

    template <class T>
    void sendNumber(T val) {
        T mask = 255;
        for(uint8_t i = sizeof(val); i > 0;) {
            i--;
            T m1 = val >> (8 * i);
            writeByte((uint8_t)(m1 & mask));
        }	
    }

    void sendHeader() {
        writeByte(UARTIO_ESC);
    }

    void sendType(uint8_t msgType) {
        writeByte(msgType);
    }

    void sendBody(const char* msg, uint32_t len) {
        for(uint32_t i = 0; i < len; i++) {
            if(msg[i] == UARTIO_ESC)
                writeByte(UARTIO_ESC);
            writeByte(msg[i]);
        }
    }

    void writeCRC32(uint32_t checksum) {
        sendHeader();
        sendType(UARTIO_TYPE_ANSWER);
        sendNumber(sizeof(checksum));
        sendNumber(checksum);
    }

    void writeAnswer(u_int32_t crc32) {
        logToSerial("ANS[H:");
        sendHeader();

        logToSerial("|T:");
        sendType(UARTIO_TYPE_ANSWER);
        
        logToSerial("C:");
        sendNumber(crc32);
        logToSerial("]\n");
    }

    template <class T>
    void readNumber(T &val) {
        uint8_t i = sizeof(val);
        val = 0;
        while(i > 0) {
            if(port.available()) {
                uint8_t b = port.read();
                logToSerialHex(b);
                if(val)
                    val = val << 8;
                if(b)
                    val = val | (T)b;
                i--;
            }
        }
    }

    void receive(UARTIO_Message& msg, uint32_t timeoutMsec) {

        //TODO
        msg.type = UARTIO_TYPE_NONE;
        msg.crc32 = 0;
        msg.length = 0;
        msg.body = nullptr;

        unsigned long timeout = timeoutMsec > 0 ? millis() + timeoutMsec : 0;

        logToSerial("R: ");
        logToSerial("t=");
        logToSerial(timeout);
        logToSerial("[");

        uint8_t b1;
        uint8_t b2 = 1;

        while(true) {
            if(timeout && (timeout < millis())) {
                logToSerial("] !!!TIMEOUT\n");
                return;
            }
            if(port.available()) {
                b1 = b2;
                b2 = port.read();
                logToSerialHex(b2);

                if((b1 == UARTIO_ESC) && (b2 != UARTIO_ESC)) {
                    // got ESC and it is not escaped
                    msg.type = b2;
                    logToSerial("|T=");
                    logToSerialHex(msg.type);
                    logToSerial("|");
                    break;
                }
            } else {
                logToSerial(".");
                delay(100);
            }
        }

        readNumber(msg.crc32);
        logToSerial("|C=");
        logToSerial(msg.crc32);

        if(msg.type == UARTIO_TYPE_ANSWER) {
            logToSerial("]\n");
            return;
        }
        logToSerial("|");

        readNumber(msg.length);
        logToSerial("|L=");
        logToSerial(msg.length);
        logToSerial("|");
        if(msg.length > UARTIO_MAXBODYLENGTH) {
            //TODO throw exception?
            msg.type = UARTIO_TYPE_NONE;
            logToSerial("]\n!!! ERROR. body length too large: ");
            logToSerial((int)msg.length);
            logToSerial("\n");
            return;
        }
        msg.body = new char[msg.length + 1];
        msg.body[msg.length] = 0;
        uint32_t i = 0;
        bool esc = false;
        while(i < msg.length)
            if(port.available()) {
                char c = port.read();
                logToSerialHex(c);
                if(esc) {
                    //TODO throw exception?
                    // Only character to be escaped is escape itself
                    if(c != UARTIO_ESC) {
                        msg.type = UARTIO_TYPE_NONE;
                        logToSerial("]\n!!!ERROR. 0x00 expected, got ");
                        logToSerialHex(c);
                        return;
                    }
                    c = UARTIO_ESC;
                    esc = false;
                } else {
                    if(c == UARTIO_ESC)
                        esc = true;
                }

                if(!esc) {
                    msg.body[i] = c;
                    i++;
                }
            }

        logToSerial("]\n");
    }

    void receive(UARTIO_Message& msg) {
        return receive(msg, 0);
    }
    
    void writePacket(const char* msg, uint32_t len) {
        uint32_t checksum = CRC32::calculate(msg, len);

        for(uint16_t retriesCount = 0; retriesCount < UARTIO_WRITE_RETRIES; retriesCount++) {
            logToSerial("W: ");

            //HEADER
            logToSerial("[H:");
            sendHeader();

            //TYPE
            logToSerial("|T:");
            sendType(UARTIO_TYPE_MESSAGE);

            //CHECKSUM
            logToSerial("|C:");
            sendNumber(checksum);

            //LENGTH
            logToSerial("|L:");
            sendNumber(len);

            //BODY
            logToSerial("|");
            sendBody(msg, len);
            logToSerial("]\n");

            //Waiting for response
            UARTIO_Message msg;
            while(msg.type != UARTIO_TYPE_ANSWER) {
                receive(msg, UARTIO_ANSWER_TIMEOUT);
                if(msg.type == UARTIO_TYPE_NONE)
                    break;
                //logToSerial("Received type ");
                //logToSerial(msg.type);
                if(msg.body != nullptr)
                    delete msg.body;
            }

            logToSerial("\nchecksum send: ");
            logToSerial(checksum);
            logToSerial("\nchecksum rcvd: ");
            logToSerial(msg.crc32);
            if(msg.crc32 == checksum) {
                logToSerial("\t Checksums OK");
                return;
            } else
                logToSerial("\n!!! Checksums different !!!");

            delay(UARTIO_DELAY_BETWEEN_WRITE_RETRIES);
        }
        logToSerial("!!!Retries count reached");
    }


    
public:
    UARTIO(HardwareSerial& p) : port(p) {}

    void write(const char* msg, uint32_t msgLength) {
        writePacket(msg, msgLength);
    }

    void write(const char* msg) {
        writePacket(msg, strlen(msg));
    }

    void readMessage(UARTIO_Message& msg) {
        while(true) {
            receive(msg);

            uint32_t checksum = CRC32::calculate(msg.body, msg.length);
            writeAnswer(checksum);
            if(checksum == msg.crc32)
                return;
            else {
                logToSerial("!!!ERROR wrong CRC32\n");
                delete msg.body;
            }
        }
    }

    char* read() {
        UARTIO_Message msg;
        readMessage(msg);
        return msg.body;
    }


};