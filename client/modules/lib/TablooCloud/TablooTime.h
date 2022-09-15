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


// TODO: Bad code, read about timezones and write better
void setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    rtc.setTime(second, minute, hour, day, month, 2000 + year);
    unsigned long le = rtc.getLocalEpoch();
    le = le - rtc.offset;
    rtc.setTime(le);
}

void setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t offset) {
    rtc.offset = 900 * offset;
    setDateTime(year, month, day, hour, minute, second);
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

/*

unsigned long timeDelta;    // Difference (in msec) between microcontroller millis and actual current local time got from server

void setTime(uint8_t h, uint8_t m, uint8_t s) {
    timeDelta = 1000 * (h * 3600 + m * 60 + s) - millis();
}

void getCurrentHoursAndMinutes(uint8_t& h, uint8_t& m)
{
    unsigned long tt = (timeDelta + millis()) / 60000;
    h = (tt / 60) % 24;
    m = tt % 60;
}
*/

#endif