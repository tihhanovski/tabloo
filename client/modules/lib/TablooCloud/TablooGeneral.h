/**
 * Tabloo - opensource bus stop display
 * General types and utility functions
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 */

#ifndef TABLOO_GENERAL_INCLUDED
#define TABLOO_GENERAL_INCLUDED true

#define UART_PACKET_TYPE_TIMETABLE 1
#define UART_PACKET_TYPE_CURRENTTIME 2
#define UART_PACKET_TYPE_COMMAND 3


#include <Arduino.h>

struct SimpleTime {
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
};

String format(SimpleTime t) {
    return (String)("") + t.hours + ":" + t.minutes + ":" + t.seconds;
}

#endif