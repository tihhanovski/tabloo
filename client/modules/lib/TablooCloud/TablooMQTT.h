/**
 * Tabloo - opensource bus stop display
 * Connection code
 * This is used only for testing of prototype: wifi connection, data fetch from HTTP
 * In actual system other medium (maybe LoRa) and protocol will be used.
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 */

#ifndef _TABLOOMQTT_H_
#define _TABLOOMQTT_H_

#include <Arduino.h>
#include <PubSubClient.h>
#include <TablooSetup.h>

#define MQTT_CLIENT_ID_MASK "tc_%s"
#define MQTT_SETUPSTRING_MAXLENGTH 255
const char* MQTT_TOPIC_TABLE_MASK = "tabloo/stops/%s/table";
const char* MQTT_TOPIC_SENSORS_LIST_MASK = "tabloo/stops/%s/sensors";
const char* MQTT_TOPIC_TASKS_QUEUE_MASK = "tabloo/stops/%s/tasks";
const char* MQTT_TOPIC_OUTPUT_MASK = "tabloo/stops/%s/output";

char* mqtt_client_id;
char* stop_code;
char* mqtt_topic_table;
char* mqtt_topic_sensors_list;
char* mqtt_topic_tasks_queue;
char* mqtt_topic_output;

PubSubClient  mqtt(client);
uint32_t lastReconnectAttempt = 0;

void (*mqtt_onTimetableReceived)(char* data, size_t len) = nullptr;
void (*mqtt_onCommandReceived)(char* data, size_t len) = nullptr;
void (*mqtt_onSensorsListReceived)(char* data, size_t len) = nullptr;

void mqtt_callback(char* topic, byte* payload, unsigned int len) {
    log_v("Message on topic: '%s', %d bytes", topic, len);

    if(!strcmp(topic, mqtt_topic_table) && mqtt_onTimetableReceived != nullptr)
        mqtt_onTimetableReceived((char*)payload, len);

    if(!strcmp(topic, mqtt_topic_tasks_queue) && mqtt_onCommandReceived != nullptr)
        mqtt_onCommandReceived((char*)payload, len);

    if(!strcmp(topic, mqtt_topic_sensors_list) && mqtt_onSensorsListReceived != nullptr)
        mqtt_onSensorsListReceived((char*)payload, len);

    // Only proceed if incoming message's topic matches
    /*
    if (String(topic) == topicLed) {
        ledStatus = !ledStatus;
        digitalWrite(LED_PIN, ledStatus);
        mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
    }*/
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
    log_v("Loading setup");
    //TODO get client id, stop id
    stop_code = setup_getStringValue(SETUP_KEY_STOP_CODE);
    log_i("Stop code = '%s'", stop_code);
    if(strlen(stop_code)) {
        initMaskedVariable(mqtt_client_id, MQTT_CLIENT_ID_MASK);
        initMaskedVariable(mqtt_topic_table, MQTT_TOPIC_TABLE_MASK);
        initMaskedVariable(mqtt_topic_sensors_list, MQTT_TOPIC_SENSORS_LIST_MASK);
        initMaskedVariable(mqtt_topic_tasks_queue, MQTT_TOPIC_TASKS_QUEUE_MASK);
        initMaskedVariable(mqtt_topic_output, MQTT_TOPIC_OUTPUT_MASK);
    }
    } catch (int i) {
        log_e("Setup load failed");
        log_i("Fix stop code in setup and restart");
        *stop_code = 0;
    }

}

boolean mqtt_connect() {
    log_v("Starting connection");
    if(!strlen(stop_code)) {
        log_e("No stop code in setup!");
        log_i("Provide stop code and restart to connect to MQTT broker");
    }
    if(!ensureConnected()) {
        log_e("No connection");
        return false;
    }
    log_v("Connecting to %s as %s : %s", MQTT_BROKER, MQTT_USER, (strlen(MQTT_PASS) ? "**PASS_SET**" : "**PASS_NOT_SET**"));
    // Connect to MQTT Broker
    //boolean status = mqtt.connect(mqtt_client_id);
    boolean status = mqtt.connect(mqtt_client_id, MQTT_USER, MQTT_PASS);

    if (status == false) {
        log_e("MQTT connect failed. Client state %d", mqtt.state());
        return false;
    }
    log_v("MQTT connect successfull");
    //TODO subscribe to topics
    //mqtt.publish(mqtt_topic_init, "device started");
    mqtt.subscribe(mqtt_topic_table);
    mqtt.subscribe(mqtt_topic_sensors_list);
    mqtt.subscribe(mqtt_topic_tasks_queue);
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
    if (!mqtt.connected()) {

        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 10000L) {
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