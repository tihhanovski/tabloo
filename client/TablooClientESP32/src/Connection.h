#include <Arduino.h>

#include <WiFi.h>
//#include <ezTime.h>   // Search for "ezTime" in the Arduino Library manager: https://github.com/ropg/ezTime
#include <HTTPClient.h>

//#define WIFI_SSID "Tartu Kolledž"
//#define WIFI_PASS "Puie5tee"
//#define WIFI_SSID "k10-firstfloor"      // your network SSID (name)
//#define WIFI_PASS "aPustiKaV1nternet"   // your network key
//#define WIFI_SSID "Telia-86EB42"        // your network SSID (name)
//#define WIFI_PASS "EMPBNVJUMMHXRR"      // your network key


#define HTTP_SERVER "https://dev.intellisoft.ee/tabloo/ask/?c="

//HTTP: See https://randomnerdtutorials.com/esp32-http-get-post-arduino/

void startConnection(const char* ssid, const char* password)
{
    if(ssid == nullptr)
    {
        Serial.print("Wifi SSID not given");
        return;
    }
    Serial.print("WiFi: Connecting to ");
    Serial.print(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

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