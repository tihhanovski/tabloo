#include <Arduino.h>
#include <HardwareSerial.h>

#define UARTIO_ESC 0
#define UARTIO_MAXBODYLENGTH 1024

class UARTIO_Message {
    public:
        uint8_t type;
        uint32_t length;
        char* body;
};

class UARTIO {
    HardwareSerial& port;

    void dbgOutputHex(uint8_t b) {
        if(b < 16)
            Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
    }

    void writeByte(uint8_t b) {
        port.write(b);
        if(b < 16)
            Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
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

    void writePacket(const char* msg) {
        uint32_t len = strlen(msg);

        Serial.print("msg '");
        Serial.print(msg);
        Serial.print("' L: ");
        Serial.print((int)len);
        Serial.print(": [");

        sendHeader();
        sendType(2);


        sendNumber(len);
        sendBody(msg, len);
        Serial.println("]");
    }

    template <class T>
    void readNumber(T &val) {
        uint8_t i = sizeof(val);
        val = 0;
        while(i > 0) {
            if(port.available()) {
                uint8_t b = port.read();
                dbgOutputHex(b);
                if(val)
                    val = val << 8;
                if(b)
                    val = val | (T)b;
                i--;
            }
        }
    }

public:
    UARTIO(HardwareSerial& p) : port(p) {}

    void write(const char* msg) {
        writePacket(msg);
    }

    void receive(UARTIO_Message& msg) {

        Serial.print("R: [");

        uint8_t b1;
        uint8_t b2 = 1;

        while(true) {
            if(port.available()) {
                b1 = b2;
                b2 = port.read();
                dbgOutputHex(b2);

                if((b1 == UARTIO_ESC) && (b2 != UARTIO_ESC)) {
                    // got ESC and it is not escaped
                    msg.type = b2;
                    Serial.print("|T=");
                    dbgOutputHex(msg.type);
                    Serial.print("|");
                    break;
                }
            } else {
                Serial.print(".");
                delay(500);
            }
        }


        //while(!port.available());
        readNumber(msg.length);
        Serial.print("|L=");
        Serial.print(msg.length);
        Serial.print("|");
        if(msg.length > UARTIO_MAXBODYLENGTH) {
            Serial.print("]\n!!! ERROR. body length too large: ");
            Serial.println((int)msg.length);
            return;
        }
        msg.body = new char[msg.length + 1];
        msg.body[msg.length] = 0;
        uint32_t i = 0;
        bool esc = false;
        while(i < msg.length)
            if(port.available()) {
                char c = port.read();
                dbgOutputHex(c);
                if(esc) {
                    // Only character to be escaped is escape itself
                    if(c != UARTIO_ESC) {
                        Serial.print("]\n!!!ERROR. 0x00 expected, got ");
                        Serial.println(c, HEX);
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

        Serial.println("]");
    }
};