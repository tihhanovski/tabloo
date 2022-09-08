/**
 * Tabloo - opensource bus stop display
 * Connection code
 * This is used only for testing of prototype: wifi connection, data fetch from HTTP
 * In actual system other medium (maybe LoRa) and protocol will be used.
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 */

#include <Arduino.h>

#include <WiFi.h>
//#include <ezTime.h>   // Search for "ezTime" in the Arduino Library manager: https://github.com/ropg/ezTime
#include <HTTPClient.h>

//#define WIFI_SSID "Tartu Kolledž"
//#define WIFI_PASS "..."

#define HTTP_SERVER "https://dev.intellisoft.ee/tabloo/ask/?c="

//HTTP: See https://randomnerdtutorials.com/esp32-http-get-post-arduino/

/**
 * Start WiFi connection with given SSID and password
 */
bool startConnection(const char* ssid, const char* password)
{
    if(ssid == nullptr)
    {
        Serial.print("Wifi SSID not given");
        return false;
    }
    Serial.print("WiFi: Connecting to ");
    Serial.print(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    //TODO do something with hardcoded timeout
    unsigned long wifiConnectionTimeout = millis() + 10000;

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        if(millis() > wifiConnectionTimeout)
        {
            Serial.println(" failed");
            return false;
        }
        delay(500);
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

/**
 * Get the data from server
 * @param stopId null terminated c string, Bus stop ID from GTFS database, for example 7820161-1
 * @param data address of buffer, where fetched data will be copied
 * @param dataSize size of data fetched
 * @return true if succeeded
 */
bool fetchData(const char* stopId, char*& data, size_t& dataSize){

    if(WiFi.status() != WL_CONNECTED)
    {
        Serial.println("No Wifi connection. Can not fetch");
        return false;
    }

    size_t sl = strlen(HTTP_SERVER);
    char* url = new char[sl + strlen(stopId) + 1];
    memcpy(url, HTTP_SERVER, sl);
    strcpy(url + sl, stopId);

    bool ret = false;

    if(data != nullptr)
        delete[] data;

    Serial.print("URL: ");
    Serial.println(url);

    HTTPClient http;
    http.begin(url);
  
    int httpResponseCode = http.GET();
    if (httpResponseCode>0) {
        String sPayload = http.getString();
        dataSize = sPayload.length();
        data = new char[dataSize];
        memcpy(data, sPayload.c_str(), dataSize);

        ret = true;

        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.print("Size: ");
        Serial.println(dataSize);
        Serial.println("-------------");
        for(size_t i = 0; i < dataSize; i++)
        {
            Serial.print(i);
            Serial.print(":");
            Serial.print(data[i], HEX);
            Serial.print("\t");
        }
        Serial.println("\n-------------");
    }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }

    // Free resources
    delete url;

    http.end();
    return ret;
}