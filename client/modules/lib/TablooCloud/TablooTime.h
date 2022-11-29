/**
 * Tabloo - opensource bus stop display
 * General types and utility functions
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 */

#define TIME_PACKET_SIZE 5

#ifndef _TABLOO_TIME_H_
#define _TABLOO_TIME_H_

#ifndef TIME_TIMEZONE_OFFSET_GMT_SEC
#define TIME_TIMEZONE_OFFSET_GMT_SEC 7200
#endif

#ifndef TIME_DST_OFFSET_SEC
#define TIME_DST_OFFSET_SEC 0
#endif

#include <Arduino.h>
#include <ESP32Time.h>

ESP32Time rtc(TIME_TIMEZONE_OFFSET_GMT_SEC);
bool rtc_time_initialized = false;

void getHourAndMinute(uint8_t& h, uint8_t& m) {
    m = rtc.getMinute();
    h = rtc.getHour(true);
}

/**
 * @brief packs unix epoch time, tz and dst offset to 5 bytes where:
 * bytes 0 - 3 - timestamp,
 * byte 4:
 *      bit 7 - dst on/off = dst / 3600;
 *      other bits - tz offset in seconds / 900 ie TZ offset with 15 minutes precision
*/
void time_setup_packet(uint8_t packet[TIME_PACKET_SIZE]) {
    time_t now;     //time_t should be 4 bytes long
    time(&now);
    for(int8_t i = 3; i >= 0; i--) {
        packet[i] = now & 255;
        now = now >> 8;
    }

    packet[4] = (TIME_TIMEZONE_OFFSET_GMT_SEC / 900)    // timezone offst 0 .. 96
        | (TIME_DST_OFFSET_SEC ? 128 : 0);

    log_v("time packet %02x %02x %02x %02x %02x", packet[0], packet[1], packet[2], packet[3], packet[4], packet[5]);

    // timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_wday, 
}

/**
 * @brief unpacks time packet to setup current datetime
*/
void time_init_by_packet(uint8_t packet[TIME_PACKET_SIZE]) {
    unsigned long epoch = 0;
    for(int8_t i = 0; i < 4; i++)
        epoch = (epoch << 8) | packet[i];

    struct timezone tz;
    tz.tz_minuteswest = 900 * (int)(packet[4] & 127);
    tz.tz_dsttime = DST_EET;

    
    // long epoch = SOME_TIME;
    struct timeval tv;
    if (epoch > 2082758399){
        // overflow = true;
        tv.tv_sec = epoch - 2082758399;  // epoch time (seconds)
    } else {
        tv.tv_sec = epoch;  // epoch time (seconds)
    }
    tv.tv_usec = 0;    // microseconds
    settimeofday(&tv, &tz);

    log_i("current time set to %d, tz offset=%d, DST=%d", epoch, tz.tz_minuteswest, tz.tz_dsttime);

    rtc_time_initialized = true;

    uint8_t m, h;
    getHourAndMinute(h, m);
    log_i("time is %02d:%02d", h, m);

    // TODO proper DST setup
    // struct timezone {
    // 	int	tz_minuteswest;	/* minutes west of Greenwich */
    // 	int	tz_dsttime;	/* type of dst correction */
    // };
    // #define	DST_NONE	0	/* not on dst */
    // #define	DST_USA		1	/* USA style dst */
    // #define	DST_AUST	2	/* Australian style dst */
    // #define	DST_WET		3	/* Western European dst */
    // #define	DST_MET		4	/* Middle European dst */
    // #define	DST_EET		5	/* Eastern European dst */
    // #define	DST_CAN		6	/* Canada */
    
}



struct SimpleDateTime {
    uint8_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
    uint8_t offset = 0;

public:

    void clear() {
        year = 0;
        month = 0;
        day = 0;
        hours = 0;
        minutes = 0;
        seconds = 0;
        offset = 0;
    }

    bool isValid() {
        return year > 4;
    }

    void setupByRTC() {
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo)){
            log_e("Failed to obtain time");
            clear();
            return;
        }

        year = rtc.getYear() - 2000;
        month = 1 + rtc.getMonth();
        day = rtc.getDay();
        hours = rtc.getHour(true);
        minutes = rtc.getMinute();
        seconds = rtc.getSecond();
        offset = rtc.offset / 900;

        log_v("time set: %d-%d-%d %02d:%02d:%02dZ%d", year, month, day, hours, minutes, seconds, offset);
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


bool isTimeInitialized() {
    return rtc_time_initialized;
}


// TODO: Bad code, read about timezones and write better
void setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {

    log_v("setDateTime %d-%d-%d %d:%d:%d+%d", year, month, day, hour, minute, second, rtc.offset);
    if(year <= 4) {
        log_e("Invalid time detected. Skip RTC initialization");
        return;
    }

    rtc.setTime(second, minute, hour, day, month, 2000 + year);

    //weird offset hack:

    uint8_t h, m;
    getHourAndMinute(h, m);
    log_v("Time after setup: %d.%d <-- %d", h, m, rtc.getLocalEpoch());

    unsigned long le = rtc.getLocalEpoch();
    le = le - rtc.offset;
    rtc.setTime(le);

    getHourAndMinute(h, m);
    log_v("Time after hack: %d.%d <-- %d", h, m, rtc.getLocalEpoch());

    rtc_time_initialized = true;
}

void setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t offset) {
    rtc.offset = 900 * offset;
    log_v("Setting rtc offset to 900 * %d = %d ", offset, rtc.offset);
    setDateTime(year, month, day, hour, minute, second);
}

void getDateTime(uint8_t& year, uint8_t& month, uint8_t& day, uint8_t& hour, uint8_t& minute, uint8_t& second, uint8_t& offset) {
    year = rtc.getYear() - 2000;
    month = 1 + rtc.getMonth();
    day = rtc.getDay();
    hour = rtc.getHour(true);
    minute = rtc.getMinute();
    second = rtc.getSecond();
    offset = rtc.offset / 900;
}


/**
 * @brief Returns current day of week (0=sunday, 6=saturday)
 */
uint8_t getDayOfWeek() {
    return rtc.getDayofWeek();
}

#endif