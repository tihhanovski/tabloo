/**
 * 
 * Debug and OTA via local wifi network
 * 
 * Setup platformio.ini:
 *      upload_protocol = espota
 *      upload_port = <WIFIDEV_HOST>.local
 *      build_flags = 
 *      	-DCORE_DEBUG_LEVEL=5
 *      lib_deps = joaolopesf/RemoteDebug@^3.0.5
 * 
 * Define in main.cpp:
 *      WIFIDEV_SSID
 *      WIFIDEV_PASS
 *      WIFIDEV_HOST
 * 
 * 
 * in setup():
 *      wifidev_setup(WIFIDEV_SSID, WIFIDEV_PASS, WIFIDEV_HOST);
 * 
 * in loop():
 *      wifidev_loop();
 * 
 * Bug in websockets:
 *      replace #include <hwcrypto/sha.h> --> #include <esp32/sha.h>
 * 
 * To upload
 *          pio run -t upload
 * 
 * To debug use: 
 *          nc <WIFIDEV_HOST>.local 23
 * 
*/

#ifndef _WIFIDEV_H_
#define _WIFIDEV_H_

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <RemoteDebug.h>

RemoteDebug Debug;
#define log_v debugV

void wifidev_setup(const char* ssid, const char* password, const char* hostname) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    if(strlen(hostname))
        ArduinoOTA.setHostname(hostname);

    ArduinoOTA
        .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
        })
        .onEnd([]() {
        Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

    ArduinoOTA.begin();

    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());


    Debug.begin(hostname);
    Debug.setResetCmdEnabled(true);
    Debug.showProfiler(true);
    Debug.showColors(true);

}

void wifidev_loop() {
    ArduinoOTA.handle();
    Debug.handle();
    yield();
}

#endif