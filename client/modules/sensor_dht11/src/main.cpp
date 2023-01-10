/**
 * Tabloo - opensource bus stop display
 * Sample extension module - DHT11 based temperature and humidity sensor
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 * Module acts as I2C target
 * TablooI2CTarget.h handles basic module tasks such as current time setup, reboot etc
 * Module outputs next data:
 * - reading no (0 - 255, then restart from 0)
 * - current date (unix timestamp, UTC)
 * - humidity (float)
 * - temperature (float)
 * - humidity and temp index (float)
 * 
 * Format:
 * <reading no>;D=<current_date>;H=<humidity>;T=<temperature>;I=<index>
 * 
 * Example
 *  24 D=173814;H=6.00;T=24.80;I=23.
 * 
 */


#include <Arduino.h>


#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_ADDR 0x04
#define I2C_TARGET_BUFFER_SIZE 1024

#include <TablooI2CTarget.h>

#include "DHT.h"      // DHT library

#define DHTPIN 4     // Digital pin connected to the DHT sensor

#define MEASUREMENT_INTERVAL 2000

#define DHTTYPE DHT11       // DHT 11
DHT dht(DHTPIN, DHTTYPE);   // Initialize DHT sensor.


float humidity;
float temperature;
float heatIndex;

// Based on Example testing sketch for various DHT humidity/temperature sensors written by ladyada
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

// Based on code from WireSlave Sender
// by Gutierrez PS <https://github.com/gutierrezps>
// ESP32 I2C slave library: <https://github.com/gutierrezps/ESP32_I2C_Slave>
// based on the example by Nicholas Zambetti <http://www.zambetti.com>

uint8_t y = 0;

/**
 * Callback function
 * called by TablooI2CTarget, see setup()
*/
void dataRequested() {
    log_v("requested data");

    uint8_t yr, mo, dy, hr, mn, sc, of;

    time_t now;
    time(&now);

    WireSlave.print(y++);
    WireSlave.print(";D=");
    WireSlave.print((int)now);
    WireSlave.print(";");
    WireSlave.print("H=");
    WireSlave.print(humidity, 1);
    WireSlave.print(";T=");
    WireSlave.print(temperature, 1);
    WireSlave.print(";I=");
    WireSlave.print(heatIndex, 1);
    // WireSlave.print(";U=");
    // WireSlave.print(millis(), 10);
    WireSlave.println("");
    log_v("data printed");
}

/**
 * Callback method
 * Called by TablooI2CTarget, see setup()
 * Just output command to log
*/
void commandReceived(uint8_t* command, size_t cmdLength) {
    char* c = new char[cmdLength + 1];
    memcpy(c, command, cmdLength);
    c[cmdLength] = 0;
    log_i("Command received '%s'", c);
    delete[] c;
}

unsigned long nextTime = 0;

/**
 * Read and save sensor data
*/
void readSensor() {
    unsigned long time = millis();
    if(nextTime > time) 
        return;
    nextTime = time + MEASUREMENT_INTERVAL;

    float h = dht.readHumidity();               // Read humidity
    float t = dht.readTemperature();            // Read temperature as Celsius (the default)

    if (isnan(h) || isnan(t))                   // Check if any reads failed and exit early (to try again).
        log_e("Failed to read from DHT sensor!");
    else {
        temperature = t;
        humidity = h;
        heatIndex = dht.computeHeatIndex(t, h, false);
        log_v("H: %f, T: %f, %I: %f", humidity, temperature, heatIndex);
    }
}

/**
 * Setup everything
*/
void setup() {
    Serial.begin(115200);
    dht.begin();

    i2ctarget_setup();
    i2ctarget_onDataRequested = dataRequested;
    i2ctarget_onCommand = commandReceived;
}

/**
 * Arduino loop
*/
void loop() {
    i2ctarget_loop();
    delay(1);
    readSensor();
}