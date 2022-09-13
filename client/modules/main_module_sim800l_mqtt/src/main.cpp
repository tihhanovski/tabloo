// Sample code for https://www.aliexpress.com/item/4000456922720.html?spm=a2g0s.12269583.0.0.75154f39xjmZMx
// Based on example from TinyGSM by Volodymyr Shymanskyy 
// https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
// 

//TTGO T-Call pins
#define SETUP_LED_PIN 13
#define SETUP_BUTTON_PIN 15

//MQTT broker data
#define MQTT_BROKER "dev.intellisoft.ee"
#define MQTT_USER "ilja"
#define MQTT_PASS "inf471c"
#define MQTT_PORT 8883


#include <Arduino.h>
//#include <ArduinoHttpClient.h> //https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
#include <TablooGeneral.h>
#include <TablooGSM.h>
#include <TablooMQTT.h>
#include <TablooSetup.h>


#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

unsigned long nextTimeRequest = 0;

void requestTime() {

    unsigned long t = millis();
    if(nextTimeRequest > t)
        return;
    nextTimeRequest = t + 10000;
    SimpleTime time = requestNetworkTime();
    t = millis() - t;
    Serial.print("Time ");
    Serial.println(format(time));
    Serial.print("requested in ");
    Serial.println(t);
    SerialMon.println("--------");

}

void setup() {
    Serial.begin(115200);
    setup_start();
    
    startModem();
    ensureConnected();
    mqtt_start();

    //TODO requestTime();
}

void publishCount() {
    /*
    Serial.println("Will publish");
    ensureConnected();
    mqtt.publish(mqtt_topic_uptime, ((String)("") + millis()).c_str());
    mqtt.publish(mqtt_topic_device_temp, ((String)("") + ((temprature_sens_read() - 32) / 1.8)).c_str());
    mqtt.publish(mqtt_topic_signal, ((String)("") + modem.getSignalQuality()).c_str());
    Serial.println("Published");
    */
}

unsigned long cntTime = 0;

void loop() {

    setup_loop();
    mqtt_loop();

    /*
    if(cntTime < millis()) {
        publishCount();
        cntTime = millis() + 2000;
    }
    */
}
