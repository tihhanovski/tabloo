/**
 * Tabloo - opensource bus stop display
 * Accumulate data received from Serial port and send it to process as a command
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com> 
 */

#include <Arduino.h>

#define MAX_SERIAL_COMMAND_SIZE 100

/**
 * Receive data from Serial port and send it for processing on EOL (symbol with code = 10)
 * Ignores lines longer than MAX_SERIAL_COMMAND_SIZE
 */
class SerialInput {

    char serialCommandBuffer[MAX_SERIAL_COMMAND_SIZE] = {0};      // Setup command buffer
    char* serialCommandCurrentByte = serialCommandBuffer;         // current byte being received from serial
    void (*onCommandReceived)(char* command) = nullptr;           // Command received
 
    void processSerialCommand() {
        *serialCommandCurrentByte = 0;
        if(onCommandReceived != nullptr)
            onCommandReceived(serialCommandBuffer);
    }
public:

    /**
     * @param commandReceivedCallback pointer to command handler method
     */
    SerialInput(void (*commandReceivedCallback)(char* command)) {
        this->onCommandReceived = commandReceivedCallback;
    }

    /**
     * Call it from main loop
     */
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