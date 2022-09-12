// Sample code for https://www.aliexpress.com/item/4000456922720.html?spm=a2g0s.12269583.0.0.75154f39xjmZMx
// Based on example from TinyGSM by Volodymyr Shymanskyy 
// https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
// 

#include <Arduino.h>
#include <ArduinoHttpClient.h> //https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
#include <TablooGeneral.h>
#include <TablooGSM.h>

#define WEB_ENABLED false
#define PUBSUBCLIENT_ENABLED true

#if WEB_ENABLED
const char server[]   = "dev.intellisoft.ee";
const char resource[] = "/";
const int  port       = 443;
HttpClient http(client, server, port);

unsigned long int nextRequestTime = 0;

void doRequest() {
    if(millis() < nextRequestTime)
        return;
    SerialMon.print(F("Performing HTTPS GET request... "));

    http.connectionKeepAlive();  // Currently, this is needed for HTTPS
    int err = http.get(resource);
    if (err != 0) {
        SerialMon.println(F("failed to connect"));
        delay(10000);
        return;
    }

    int status = http.responseStatusCode();
    SerialMon.print(F("Response status code: "));
    SerialMon.println(status);
    if (!status) {
        delay(10000);
        return;
    }

    SerialMon.println(F("Response Headers:"));
    while (http.headerAvailable()) {
        String headerName  = http.readHeaderName();
        String headerValue = http.readHeaderValue();
        SerialMon.println("    " + headerName + " : " + headerValue);
    }

    int length = http.contentLength();
    if (length >= 0) {
        SerialMon.print(F("Content length is: "));
        SerialMon.println(length);
    }
    if (http.isResponseChunked()) {
        SerialMon.println(F("The response is chunked"));
    }

    String body = http.responseBody();
    SerialMon.println(F("Response:"));
    SerialMon.println(body);

    SerialMon.print(F("Body length is: "));
    SerialMon.println(body.length());

    // Shutdown
    //http.stop();
    //SerialMon.println(F("Server disconnected"));


    nextRequestTime = millis() + 10000;
}
#endif

#if PUBSUBCLIENT_ENABLED
const char* mqtt_broker = "dev.intellisoft.ee"; //"test.mosquitto.org"; //
const char* mqtt_client_id = "client_7820162-1";
const char* mqtt_user = "ilja";
const char* mqtt_pass = "inf471c";
const int mqtt_port = 8883;

const char* mqtt_topic_table = "tabloo/stops/7820162-1/table";
const char* mqtt_topic_sensors_list = "tabloo/stops/7820162-1/sensors";

const char* mqtt_topic_init = "tabloo/stops/7820162-1/device";
const char* mqtt_topic_uptime = "tabloo/stops/7820162-1/device/uptime";
const char* mqtt_topic_device_temp = "tabloo/stops/7820162-1/device/temp";

//const char* topicLed       = "GsmClientTest/led";
//const char* topicInit      = "GsmClientTest/init";
//const char* topicLedStatus = "GsmClientTest/ledStatus";

#include <PubSubClient.h>
PubSubClient  mqtt(client);
uint32_t lastReconnectAttempt = 0;

void mqttCallback(char* topic, byte* payload, unsigned int len) {
    SerialMon.print("Message arrived [");
    SerialMon.print(topic);
    SerialMon.print("]: ");
    SerialMon.write(payload, len);
    SerialMon.println();

    // Only proceed if incoming message's topic matches
    /*
    if (String(topic) == topicLed) {
        ledStatus = !ledStatus;
        digitalWrite(LED_PIN, ledStatus);
        mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
    }*/
}

boolean mqttConnect() {
    SerialMon.print("Connecting to ");
    SerialMon.print(mqtt_broker);
    SerialMon.print(" as ");
    SerialMon.print(mqtt_user);
    SerialMon.print(" / ");
    SerialMon.println(mqtt_pass);

    ensureConnected();

    // Connect to MQTT Broker
    //boolean status = mqtt.connect(mqtt_client_id);
    boolean status = mqtt.connect(mqtt_client_id, mqtt_user, mqtt_pass);

    if (status == false) {
        SerialMon.println(" fail");
        Serial.print("Client state ");
        Serial.println(mqtt.state());
        return false;
    }
    SerialMon.println(" success");
    mqtt.publish(mqtt_topic_init, "device started");
    mqtt.subscribe(mqtt_topic_table);
    mqtt.subscribe(mqtt_topic_sensors_list);
    
    return mqtt.connected();
}
#endif

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
    delay(10);
    startModem();


    ensureConnected();

    //doRequest();

    /*
    IPAddress ip(198, 211, 122, 176);
    client.connect(ip, 1883);
    client.println("PROOVIN PROOV");
    */

    #if PUBSUBCLIENT_ENABLED


    //requestTime();

    Serial.print("MQTT server: ");
    Serial.print(mqtt_broker);
    Serial.print(":");
    Serial.println(mqtt_port);
    mqtt.setServer(mqtt_broker, mqtt_port);
    mqtt.setCallback(mqttCallback);

    mqttConnect();
    #endif
}

void publishCount() {
    Serial.println("Will publish");
    ensureConnected();
    mqtt.publish(mqtt_topic_uptime, ((String)("") + millis()).c_str());
    mqtt.publish(mqtt_topic_device_temp, ((String)("") + ((temprature_sens_read() - 32) / 1.8)).c_str());

    Serial.println("Published");
}

unsigned long cntTime = 0;

void loop() {

    //Serial.println("Connect");
    //ensureConnected();

    //Serial.println("Ask time");
    //requestTime();

    #if WEB_ENABLED
    doRequest();
    #endif

    //Serial.println("Wait");
    //delay(5000);

    #if PUBSUBCLIENT_ENABLED
    if (!mqtt.connected()) {

        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 10000L) {
            lastReconnectAttempt = t;
            SerialMon.println("MQTT not connected, reconnecting");
            if (mqttConnect())
                lastReconnectAttempt = 0;
        }
        delay(100);
        return;
    } else {
        mqtt.loop();
    }

    if(cntTime < millis()) {
        publishCount();
        cntTime = millis() + 2000;
    }

    #endif
}
