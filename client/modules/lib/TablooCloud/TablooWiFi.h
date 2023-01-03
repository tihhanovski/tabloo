/**
 * Tabloo - opensource bus stop display
 * Connection code for WiFi 
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 *
 */

#ifndef _TABLOOWIFI_H_
#define _TABLOOWIFI_H_

#define WIFI_NETWORKING_CONNECT_TIMEOUT 10000

#ifndef TIME_NTP_SERVER
#define TIME_NTP_SERVER "pool.ntp.org"
#endif

#ifndef TIME_TIMEZONE_OFFSET_GMT_SEC
#define TIME_TIMEZONE_OFFSET_GMT_SEC 7200
#endif

#ifndef TIME_DST_OFFSET_SEC
#define TIME_DST_OFFSET_SEC 0
#endif

#include <Arduino.h>
//#include <ArduinoHttpClient.h>  //see https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
#include <TablooGeneral.h>      //Various utilities
#include <TablooTime.h>
#include <TablooSetup.h>        //Settings storage
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "time.h"

WiFiClientSecure client;

void networking_start() {

    char* ssid = setup_getStringValue(SETUP_KEY_WIFI_SSID);
    char* pass = setup_getStringValue(SETUP_KEY_WIFI_PASS);
    if(strlen(ssid)) {
        log_v("Connecting to WiFi: %s", ssid);
        client.setInsecure();
        WiFi.begin(ssid, pass);
    } else
        log_e("No SSID in provided, please setup and restart");
}

void printLocalTime(){
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Day of week: ");
    Serial.println(&timeinfo, "%A");
    Serial.print("Month: ");
    Serial.println(&timeinfo, "%B");
    Serial.print("Day of Month: ");
    Serial.println(&timeinfo, "%d");
    Serial.print("Year: ");
    Serial.println(&timeinfo, "%Y");
    Serial.print("Hour: ");
    Serial.println(&timeinfo, "%H");
    Serial.print("Hour (12 hour format): ");
    Serial.println(&timeinfo, "%I");
    Serial.print("Minute: ");
    Serial.println(&timeinfo, "%M");
    Serial.print("Second: ");
    Serial.println(&timeinfo, "%S");

    Serial.println("Time variables");
    char timeHour[3];
    strftime(timeHour,3, "%H", &timeinfo);
    Serial.println(timeHour);
    char timeWeekDay[10];
    strftime(timeWeekDay,10, "%A", &timeinfo);
    Serial.println(timeWeekDay);
    Serial.println();
}

/**
 * @brief setup RTC
*/
void networking_request_datetime() {

    //sync time using NTP
    configTime(TIME_TIMEZONE_OFFSET_GMT_SEC, TIME_DST_OFFSET_SEC, TIME_NTP_SERVER);
    printLocalTime();
    log_v("time requested");
    rtc_time_initialized = true;
    // SimpleDateTime ret;
    // ret.setupByRTC();
    // return ret;
}

boolean networking_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

boolean networking_connect() {
    log_v("start");
    if(WiFi.SSID().length()) {
        if(networking_is_connected())
            return true;
        else {
            uint32_t timeout = millis() + WIFI_NETWORKING_CONNECT_TIMEOUT;
            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
                if(millis() > timeout)
                    break;
            }
            if(networking_is_connected()) {
                log_i("WiFi connected, IP: %s", WiFi.localIP());
                return true;
            } else {
                log_e("WiFi connection failed");
                return false;
            }
        }
    }
    log_v("finish");
    return false;
}


#endif