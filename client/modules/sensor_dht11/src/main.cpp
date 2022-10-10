#include <Arduino.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_ADDR 0x04
#define I2C_TARGET_BUFFER_SIZE 1024

#include <TablooI2CTarget.h>

// Example testing sketch for various DHT humidity/temperature sensors written by ladyada
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"

#define DHTPIN 4     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

#define MEASUREMENT_DELAY 2000

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

float humidity;
float temperature;
float heatIndex;

// Code from WireSlave Sender
// by Gutierrez PS <https://github.com/gutierrezps>
// ESP32 I2C slave library: <https://github.com/gutierrezps/ESP32_I2C_Slave>
// based on the example by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the WireSlave library for ESP32.
// Sends data as an I2C/TWI slave device; data is packed using WirePacker.
// In order to the slave send the data, an empty packet must
// be received first. This is internally done by the WireSlaveRequest class.
// The data is sent using WirePacker, also done internally by WireSlave.
// Refer to the "master_reader" example for use with this

uint8_t y = 0;


void dataRequested() {
    log_v("requested data");

    uint8_t yr, mo, dy, hr, mn, sc, of;
    getDateTime(yr, mo, dy, hr, mn, sc, of);

    WireSlave.print(y++);
    WireSlave.print("D: ");
    WireSlave.print("" + String(yr) + "-" + String(mo) + "-" + String(dy) + "T" + String(hr) + ":" + String(mn) + ":" + String(sc) + "+" + String(of));
    WireSlave.print(": ");
    WireSlave.print("H: ");
    WireSlave.print(humidity);
    WireSlave.print(", T: ");
    WireSlave.print(temperature);
    WireSlave.print(", I: ");
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

    i2ctarget_setup();
    i2ctarget_onDataRequested = dataRequested;
    i2ctarget_onCommand = commandReceived;
}

unsigned long nextTime = 0;

void readSensor() {
    if(nextTime > millis()) return;

    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (!(isnan(h) || isnan(t))) {
        temperature = t;
        humidity = h;
        // Compute heat index in Celsius (isFahreheit = false)
        heatIndex = dht.computeHeatIndex(t, h, false);

        log_v("H: %f, T: %f, %I: %f", humidity, temperature, heatIndex);
    } else {
        log_e("Failed to read from DHT sensor!");
    }


    nextTime = millis() + MEASUREMENT_DELAY;
}



void loop() {
    i2ctarget_loop();
    delay(1);
    readSensor();
}