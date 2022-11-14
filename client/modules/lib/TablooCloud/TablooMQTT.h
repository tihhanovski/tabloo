/**
 * Tabloo - opensource bus stop display
 * MQTT connection
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 */

#ifndef _TABLOOMQTT_H_
#define _TABLOOMQTT_H_

#include <Arduino.h>
#include <PubSubClient.h>
#include <TablooSetup.h>

#define MQTT_SETUPSTRING_MAXLENGTH 255
#define MQTT_RECONNECTION_TIMEOUT 10000L

const char* MQTT_CLIENT_ID_MASK = "tc_%s";
const char* MQTT_TOPIC_INPUT_MASK = "tabloo/stops/%s/input/#";
const char* MQTT_TOPIC_OUTPUT_MASK = "tabloo/stops/%s/output";

char* mqtt_client_id;
char* stop_code;
char* mqtt_topic_output;
char* mqtt_topic_input;
size_t mqtt_topic_input_length;

PubSubClient  mqtt(client);

void (*mqtt_onInputReceived)(char* topic, char* data, size_t len) = nullptr;

void mqtt_callback(char* topic, byte* payload, unsigned int len) {
    log_v("Message on topic: '%s', %d bytes", topic, len);

    if(mqtt_onInputReceived != nullptr && !memcmp(topic, mqtt_topic_input, mqtt_topic_input_length))
        mqtt_onInputReceived(topic + mqtt_topic_input_length, (char*)payload, len);
}


inline void initMaskedVariable(char*& maskedVar, const char* mask) {
    char buffer[MQTT_SETUPSTRING_MAXLENGTH];
    size_t n;
    n = sprintf(buffer, mask, stop_code);
    if(n >= MQTT_SETUPSTRING_MAXLENGTH || n < 0) 
        throw 1;
    maskedVar = new char[n + 1];
    memcpy(maskedVar, buffer, n);
    maskedVar[n] = 0;
    log_v("%s >> %s (%d bytes)", mask, maskedVar, n);
}

void mqtt_load_setup() {
    try{
        stop_code = setup_getStringValue(SETUP_KEY_STOP_CODE);
        log_i("Stop code = '%s'", stop_code);
        if(strlen(stop_code)) {
            initMaskedVariable(mqtt_client_id, MQTT_CLIENT_ID_MASK);
            initMaskedVariable(mqtt_topic_output, MQTT_TOPIC_OUTPUT_MASK);
            initMaskedVariable(mqtt_topic_input, MQTT_TOPIC_INPUT_MASK);
            mqtt_topic_input_length = strlen(mqtt_topic_input) - 1; //
        }
    } catch (int i) {
        log_e("Setup load failed");
        log_i("Fix stop code in setup and restart");
        *stop_code = 0;
    }
}

boolean mqtt_connect() {
    log_v("Starting connection for stop %s", stop_code);
    if(!strlen(stop_code)) {
        log_e("No stop code in setup!");
        log_i("Provide stop code and restart to connect to MQTT broker");
    }
    if(!networking_connect()) {
        log_e("No connection");
        return false;
    }
    log_v("Connecting to %s as %s : %s", MQTT_BROKER, MQTT_USER, (strlen(MQTT_PASS) ? "**PASS_SET**" : "**PASS_NOT_SET**"));
    // Connect to MQTT Broker
    boolean status = mqtt.connect(mqtt_client_id, MQTT_USER, MQTT_PASS);

    if (status == false) {
        log_e("MQTT connect failed. Client state %d", mqtt.state());
        return false;
    }
    log_v("MQTT connect successfull");
    // Subscribe to topics
    mqtt.subscribe(mqtt_topic_input);
    log_v("subscribed to MQTT topics");
    return mqtt.connected();
}


void mqtt_start() {
    mqtt_load_setup();
    log_v("Start MQTT: broker %s : %d", MQTT_BROKER, MQTT_PORT);
    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt.setCallback(mqtt_callback);
    mqtt_connect();
}

void mqtt_loop() {
    static uint32_t lastReconnectAttempt = 0;
    if (!mqtt.connected()) {
        uint32_t t = millis();
        if (t - lastReconnectAttempt > MQTT_RECONNECTION_TIMEOUT) {    // Reconnect every 10 seconds
            lastReconnectAttempt = t;
            log_v("MQTT not connected, reconnecting");
            if (mqtt_connect())
                lastReconnectAttempt = 0;
        }
        delay(100);
        return;
    } else {
        mqtt.loop();
    }
}


#endif