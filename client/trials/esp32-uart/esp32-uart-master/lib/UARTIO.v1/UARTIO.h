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
                if(val)
                    val = val << 8;
                if(b)
                    val = val & b;
                i--;
            }
        }
    }

public:
    UARTIO(HardwareSerial& p) : port(p) {}

    void write(const char* msg) {
        writePacket(msg);
    }

    void receive(UARTIO_Message msg) {

        uint8_t b1;
        uint8_t b2 = 1;

        while(true) {
            if(port.available()) {
                b1 = b2;
                uint8_t b2 = port.read();
                if(b2 == UARTIO_ESC && b1 != UARTIO_ESC)    //got ESC and it is not escaped
                    break;
            }
        }

        while(!port.available());
        msg.type = port.read();
        readNumber(msg.length);
        if(msg.length > UARTIO_MAXBODYLENGTH) {
            Serial.print("ERROR. body length too large: ");
            Serial.println((int)msg.length);
            return;
        }
        msg.body = new char[msg.length];
        char* p = msg.body;
        bool esc = false;
        while((msg.body + msg.length) < p)
            if(port.available()) {
                char c = port.read();
                if(esc) {
                    // Only character to be escaped is escape itself
                    if(c != UARTIO_ESC) {
                        Serial.print("ERROR. 0x00 expected, got ");
                        Serial.println(c, HEX);
                        return;
                    }
                    c = 0;
                    esc = false;
                } else {
                    if(c == UARTIO_ESC)
                        esc = true;
                }

                if(!esc) {
                    *p = c;
                    p++;
                }
            }
    }
};