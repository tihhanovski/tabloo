/**
 * Tabloo - opensource bus stop display
 * General types and utility functions
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 */

#ifndef _TABLOO_TIME_H_
#define _TABLOO_TIME_H_

#ifndef TABLOOTIME_DEFAULT_OFFSET
#define TABLOOTIME_DEFAULT_OFFSET 3600 * 3 // Estonia = UTC+3
#endif

#include <Arduino.h>
#include <ESP32Time.h>

struct SimpleDateTime {
    uint8_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
    uint8_t offset = 0;

    bool isValid() {
        return year > 4;
    }
};


struct SimpleTime {
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
};

String format(SimpleTime t) {
    return (String)("") + t.hours + ":" + t.minutes + ":" + t.seconds;
}

String format(SimpleDateTime t) {
    return (String)("") + t.hours + ":" + t.minutes + ":" + t.seconds;
}

ESP32Time rtc(TABLOOTIME_DEFAULT_OFFSET);

bool rtc_time_initialized = false;

bool isTimeInitialized() {
    return rtc_time_initialized;
}

// TODO: Bad code, read about timezones and write better
void setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {

    log_v("setDateTime %d-%d-%d %d:%d:%d", year, month, day, hour, minute, second);
    if(year <= 4) {
        log_e("Invalid time detected. Skip RTC initialization");
        return;
    }

    rtc.setTime(second, minute, hour, day, month, 2000 + year);
    unsigned long le = rtc.getLocalEpoch();
    le = le - rtc.offset;
    rtc.setTime(le);
    rtc_time_initialized = true;
}

void setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t offset) {
    rtc.offset = 900 * offset;
    setDateTime(year, month, day, hour, minute, second);
}

void getDateTime(uint8_t& year, uint8_t& month, uint8_t& day, uint8_t& hour, uint8_t& minute, uint8_t& second, uint8_t& offset) {
    year = rtc.getYear() - 2000;
    month = 1 + rtc.getMonth();
    day = rtc.getDay();
    hour = rtc.getHour();
    minute = rtc.getMinute();
    second = rtc.getSecond();
    offset = rtc.offset / 900;
}


void getHourAndMinute(uint8_t& h, uint8_t& m) {
    m = rtc.getMinute();
    h = rtc.getHour(true);
}

/**
 * @brief Returns current day of week (0=sunday, 6=saturday)
 */
uint8_t getDayOfWeek() {
    return rtc.getDayofWeek();
}

#endif