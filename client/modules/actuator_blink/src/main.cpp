#include <Arduino.h>

//#define SDA_PIN 21
//#define SCL_PIN 22
#define I2C_ADDR 0x05
//#define I2C_TARGET_BUFFER_SIZE 1024

#define LED_PIN 2

#include <TablooI2CTarget.h>

uint8_t y = 0;
bool blinkRequested = false;
bool ledOn = false;
unsigned long ledOffTime = 0;


void dataRequested() {
    log_v("requested data");
}

const char* COMMAND_BLINK = "blink";

void commandReceived(uint8_t* command, size_t cmdLength) {
    char* c = new char[cmdLength + 1];
    memcpy(c, command, cmdLength);
    c[cmdLength] = 0;
    log_i("Command received '%s'", c);

    if(!strcmp(c, COMMAND_BLINK)) {
        log_v("Blink requested");
        blinkRequested = true;
    }

    delete[] c;
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);

    i2ctarget_setup();
    i2ctarget_onDataRequested = dataRequested;
    i2ctarget_onCommand = commandReceived;
}

void loop() {
    i2ctarget_loop();
    delay(1);

    if(blinkRequested) {
        log_v("led on");
        digitalWrite(LED_PIN, 1);
        blinkRequested = false;
        ledOn = true;
        ledOffTime = millis() + 500;
    }

    if(ledOn && ledOffTime < millis()) {
        log_v("led off");
        digitalWrite(LED_PIN, 0);
        ledOn = false;
    }
}