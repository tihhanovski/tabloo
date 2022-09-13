#include <Arduino.h>
#include <Preferences.h>
#include <TablooSetup.h>

void setup() {
    Serial.begin(115200);

    setup_start();

    log_v("startup");
    Serial.println();
    preferences.begin("test", false);

    unsigned int counter = preferences.getUInt("counter", 0);
    Serial.printf("Current counter value: %u\n", counter);
    counter++;
    preferences.putUInt("counter", counter);
    //preferences.end();

    log_v("startup finished");
}

void loop() {
    setup_loop();
}