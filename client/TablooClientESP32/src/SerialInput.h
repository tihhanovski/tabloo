#include <Arduino.h>

#define MAX_SERIAL_COMMAND_SIZE 100

class SerialInput {

    char serialCommandBuffer[MAX_SERIAL_COMMAND_SIZE] = {0};      // Setup command buffer
    char* serialCommandCurrentByte = serialCommandBuffer;         // current byte being received from serial
    void (*onCommandReceived)(char* command) = nullptr;           // Command received
 
public:

    SerialInput(void (*commandReceivedCallback)(char* command)) {
        this->onCommandReceived = commandReceivedCallback;
    }

    void processSerialCommand() {
        *serialCommandCurrentByte = 0;
        if(onCommandReceived != nullptr)
            onCommandReceived(serialCommandBuffer);
    }

    void loop() {
        while(Serial.available())
        {
            uint8_t incomingByte = Serial.read();
            if(incomingByte == 10) {
                processSerialCommand();
                serialCommandCurrentByte = serialCommandBuffer;
            }
            if((incomingByte > 31) && (serialCommandCurrentByte - serialCommandBuffer < MAX_SERIAL_COMMAND_SIZE))
            *(serialCommandCurrentByte++) = incomingByte;
        }
    }
};