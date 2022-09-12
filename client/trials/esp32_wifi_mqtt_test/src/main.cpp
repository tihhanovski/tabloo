/*
 * INF471C - Connected Objects and the Internet of Things (IoT)
 * PubSubClient and ESP8266
 * Ilja Tihhanovski
 * 
 * Controller connects to WiFi with given SSID and password, then to MQTT broker
 * (see defines below) and starts publishing temperature, humidity and heatIndex.
 * 
 * Controller subscribes to certain topic on the broker and react to 
 * commands published under that topic
 *  
 * Examples used
 * https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
 * https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/
 */


#include <WiFi.h>             // Wifi library
#include <PubSubClient.h>     // MQTT client
//#include "DHT.h"              // Temperature and humidity sensor library

//  Wifi credentials
#define WIFI_SSID "k10-firstfloor"
#define WIFI_PASS "aPustiKaV1nternet"

// MQTT broker connection information
#define MQTT_SERVER "dev.intellisoft.ee"
#define MQTT_CLIENT "ESP32Client"
#define MQTT_USER "ilja"
#define MQTT_PASS "inf471c"
#define MQTT_PORT 1883

// Pins
#define LED_PIN 2       // Led used to do feedback
#define DHTPIN 4        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

// Measurement / publications delays
#define DEF_TIME_BETWEEN_PUBS 2000 // in milliseconds
#define MIN_TIME_BETWEEN_PUBS_SEC 1 // Minimal possible delay, in seconds
#define MAX_TIME_BETWEEN_PUBS_SEC 60 // Maximal possible delay, in seconds

// Limit maximum command length from MQTT, that could be processed
#define MAX_COMMAND_LENGTH 50
#define MAX_PUBLISHED_MESSAGE_LENGTH 50

//MQTT topics
const char* commandTopic = "esp32/command";           // Subscribes to this topic to get commands
const char* temperatureTopic = "esp32/temperature";   // Temperature (Celsius)
const char* humidityTopic = "esp32/humidity";         // Humidity
const char* heatIndexTopic = "esp32/heatIndex";       // Heat index
const char* counterTopic = "esp32/counter";           // Counter to make sure, controller is alive

//Commands controller will react to
const char* CMD_RESET_COUNTER = "resetCounter";       // To reset measurements counter
const char* CMD_BLINK = "blink";                      // Puts controller to blink three times
const char* CMD_SETDELAY = "setDelay";                // setDelay followed by integer from MIN_TIME_BETWEEN_PUBS_SEC to MAX_TIME_BETWEEN_PUBS_SEC will set a new delay between measurements
const size_t CMD_SETDELAY_LENGTH = 8;

// Instantiate wifi, mqtt and sensor objects
WiFiClient wifi;
PubSubClient mqtt(wifi);
//DHT dht(DHTPIN, DHTTYPE);

int cnt = 0;                                        // Counter
unsigned long nextPubMillis = 0;                    // next time to measure

// Here we save last measurements
float humidity;
float temperature;
float heatIndex;

bool requestBlink = false;                          // True if blink was requested via MQTT
unsigned long timeBetweenPubs = DEF_TIME_BETWEEN_PUBS;

char msgToSend[MAX_PUBLISHED_MESSAGE_LENGTH];       // Buffer used to form message to be send to MQTT

// Connect to a WiFi network 
void connectToWifi()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) { // Will wait forever until connected
    delay(500);
    Serial.print(".");
  }

  // After successfull connection output some connection information
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Callback method to react to messages being published on topic(s) we subscribed to
void callback(char* topic, byte* message, unsigned int length) {

  // Read message from MQTT library to buffer with limited size.
  // Maximum size is priorly defined MAX_COMMAND_LENGTH.
  // Other bytes in message will be just ignored.
  char msg[MAX_COMMAND_LENGTH + 1];
  size_t msgLength = (length > MAX_COMMAND_LENGTH) ? MAX_COMMAND_LENGTH : length;
  memcpy(msg, message, msgLength);
  msg[msgLength] = 0;  //c_str final byte

  Serial.print("Message arrived on topic: '");
  Serial.print(topic);
  Serial.print("'. Message: '");
  Serial.print(msg);  
  Serial.println("'");

  if(!strcmp(topic, commandTopic)) {
    Serial.println("process a command");

    //resetCounter sets cnt to 0
    if(!strcmp(msg, CMD_RESET_COUNTER)) {
      Serial.println("resetCounter requested");
      cnt = 0;
    }

    //blink makes controller to blink its built-in led
    if(!strcmp(msg, CMD_BLINK)) {
      Serial.println("blink");
      requestBlink = true;
    }

    // setDelay NN (where NN is integer) changes delay between publications
    // delay given in seconds.
    // If delay is out of min and max delay interval [MAX_TIME_BETWEEN_PUBS_SEC .. MAX_TIME_BETWEEN_PUBS_SEC]
    // then controller will output corresponding message to Serial and blink the built-in led
    if(!memcmp(msg, CMD_SETDELAY, CMD_SETDELAY_LENGTH)) {
      Serial.print("New delay requested ");

      // Get new delay in seconds as integer from message
      unsigned long newDelayRequested = atoi(msg + CMD_SETDELAY_LENGTH);

      // Check if requested delay is too large or too small (and blink in such case)
      if(newDelayRequested > MAX_TIME_BETWEEN_PUBS_SEC) {
        Serial.print("delay ");
        Serial.print(newDelayRequested);
        Serial.print(" is too long, max ");
        Serial.println(MAX_TIME_BETWEEN_PUBS_SEC);
        requestBlink = true;
        return;
      }
  
      if(newDelayRequested < MIN_TIME_BETWEEN_PUBS_SEC) {
        Serial.print("delay ");
        Serial.print(newDelayRequested);
        Serial.print(" is too short, min ");
        Serial.println(MIN_TIME_BETWEEN_PUBS_SEC);
        requestBlink = true;
        return;
      }

      // If delay requested is OK, then save it
      timeBetweenPubs = newDelayRequested * 1000;   // We get request in seconds and convert it into milliseconds we use internally
      Serial.print(timeBetweenPubs);
      Serial.println(" OK");
    }
  }
}

// Reconnect to MQTT if was disconnected
void reconnectMQTT() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");
      // Subscribe
      mqtt.subscribe(commandTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {
  pinMode(LED_PIN, OUTPUT); // Setup PIN for (internal) LED 
  Serial.begin(115200);     // Start Serial
  while(!Serial) {}         // Wait for Serial to start

  //dht.begin();              // Connect sensor
  connectToWifi();

  //Setup MQTT, (re)connection will be initiated from loop
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
}




void loop() {
  
  if (!mqtt.connected())              // Check connection to the broker
    reconnectMQTT();                  // Reconnect if needed
  mqtt.loop();                        // Let MQTT client to do its things

  if(requestBlink) {                  // If blink was requested (by user or as result of some error)
    for(uint8_t i = 0; i < 3; i++)    // Blink three times
    {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
    requestBlink = false;
  }

  if(millis() > nextPubMillis) {        // If it is next measurement time
    cnt++;                              // Increase "heartbeat" counter

    sprintf(msgToSend, "%d", cnt);
    mqtt.publish(counterTopic, msgToSend);

    nextPubMillis = millis() + timeBetweenPubs; // Save new measurement time
  }
}

void saveAndUploadReading(const char* topic, float& savedReading, float newReading)
{
  // If reading was changed (up to last 2 digits after coma)
  if(trunc(savedReading * 100) != trunc(newReading * 100))
  {
    // then save and publish reading
    Serial.print("Publish ");
    Serial.print(newReading);
    Serial.print(" to ");
    Serial.println(topic);
    
    savedReading = newReading;
    sprintf(msgToSend, "%.2f", newReading);
    mqtt.publish(topic, msgToSend);
  }
}