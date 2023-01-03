#include <Arduino.h>


#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_ADDR 0x04
#define I2C_TARGET_BUFFER_SIZE 1024

//#define OTA_ENABLED true
//#define OTA_HOSTNAME "sensor_dht11"

#include <TablooI2CTarget.h>

#ifdef OTA_ENABLED
    #include "OTA.h"
#endif

// Example testing sketch for various DHT humidity/temperature sensors written by ladyada
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"

#define DHTPIN 4     // Digital pin connected to the DHT sensor

#define MEASUREMENT_INTERVAL 2000

#define DHTTYPE DHT11       // DHT 11
DHT dht(DHTPIN, DHTTYPE);   // Initialize DHT sensor.


float humidity;
float temperature;
float heatIndex;

// Code from WireSlave Sender
// by Gutierrez PS <https://github.com/gutierrezps>
// ESP32 I2C slave library: <https://github.com/gutierrezps/ESP32_I2C_Slave>
// based on the example by Nicholas Zambetti <http://www.zambetti.com>

uint8_t y = 0;

void dataRequested() {
    log_v("requested data");

    uint8_t yr, mo, dy, hr, mn, sc, of;

    // struct tm timeinfo;
    time_t now;
    time(&now);
    // localtime_r(&now, &timeinfo);

    // getDateTime(yr, mo, dy, hr, mn, sc, of);

    WireSlave.print(y++);
    WireSlave.print(" D=");
    // WireSlave.print("" + String(yr) + "-" + String(mo) + "-" + String(dy) + "T" + String(hr) + ":" + String(mn) + ":" + String(sc) + "+" + String(of));
    WireSlave.print(String(now));
    WireSlave.print(";");
    WireSlave.print("H=");
    WireSlave.print(humidity);
    WireSlave.print(";T=");
    WireSlave.print(temperature);
    WireSlave.print(";I=");
    WireSlave.print(heatIndex);
    WireSlave.println("");
    log_v("data printed");
}

void commandReceived(uint8_t* command, size_t cmdLength) {
    char* c = new char[cmdLength + 1];
    memcpy(c, command, cmdLength);
    c[cmdLength] = 0;
    log_i("Command received '%s'", c);
    delete[] c;
}

void setup() {
    Serial.begin(115200);
    dht.begin();

    #ifdef OTA_ENABLED
    ota_setup();
    #endif

    i2ctarget_setup();
    i2ctarget_onDataRequested = dataRequested;
    i2ctarget_onCommand = commandReceived;
}

unsigned long nextTime = 0;

void readSensor() {
    unsigned long time = millis();
    if(nextTime > time) 
        return;
    nextTime = time + MEASUREMENT_INTERVAL;

    float h = dht.readHumidity();               // Read humidity
    float t = dht.readTemperature();            // Read temperature as Celsius (the default)
    //float f = dht.readTemperature(true);      // Read temperature as Fahrenheit (isFahrenheit = true)

    if (isnan(h) || isnan(t))                   // Check if any reads failed and exit early (to try again).
        log_e("Failed to read from DHT sensor!");
    else {
        temperature = t;
        humidity = h;
        heatIndex = dht.computeHeatIndex(t, h, false);
        log_v("H: %f, T: %f, %I: %f", humidity, temperature, heatIndex);
    }
}

void loop() {
    i2ctarget_loop();
    delay(1);
    readSensor();
    #ifdef OTA_ENABLED
    ArduinoOTA.handle();
    #endif
}