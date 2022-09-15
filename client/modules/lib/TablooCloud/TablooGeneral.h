/**
 * Tabloo - opensource bus stop display
 * General types and utility functions
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 */

#ifndef _TABLOO_GENERAL_H_
#define _TABLOO_GENERAL_H_

#define UART_PACKET_TYPE_TIMETABLE 1
#define UART_PACKET_TYPE_CURRENTTIME 2
#define UART_PACKET_TYPE_COMMAND 3


#include <Arduino.h>

/*
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

unsigned long timeDelta;    // Difference (in msec) between microcontroller millis and actual current local time got from server
*/

/**
 * Setup the time delta
 * @param h hours (0 - 23)
 * @param m minutes (0 - 59)
 * @param s seconds (0 - 59)
 */
/*
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