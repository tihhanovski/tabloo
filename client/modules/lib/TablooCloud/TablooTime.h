/**
 * Tabloo - opensource bus stop display
 * General types and utility functions
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 */

/*
Ekraanimooduli jaoks vajalike ajavööndi (TZ) ja suveaja infot (DST) edastatakse koos bussisõiduplaani päises (vt joonis 4.7).

Kellaaja andmete pakett koosneb viiest baidist.
Esimesed neli baiti kodeerivad kellaaega unixi ajaga (sekundite arv alates 01.01.1970 UTC) võrdsena neljabaidise täisarvuna. 

Viies bait sisaldab ajavööndi ja suveaja informatsiooni. Minimaalne kellaajavööndi vahe on 15 minutit. Seega vahet salvestamiseks piisab korrutamiseks tundide arvu neljaga. Maksimaalne timezone offset on väiksem 24 tunnist, mis teeb offset välja maksimaalseks väärtuseks 95. Seda arvu salvestatakse seitsmesse madalamasse bittidesse [46].
Suveaja infot kodeerib viienda baiti kõrgeim bit (0 tähendab DST puudumist, 1 – selle olemasolu). Kuna erinevates kohtades suveaeg algab ja lõpeb erinevatel päevadel, ühest bitist ei piisa ja see tingib selle info eraldi baiti viimist [47].
(ei ole enam vaja)
*/

#define TIME_PACKET_SIZE 5

#ifndef _TABLOO_TIME_H_
#define _TABLOO_TIME_H_

// #ifndef TIME_TIMEZONE_OFFSET_GMT_SEC
// #define TIME_TIMEZONE_OFFSET_GMT_SEC 7200
// #endif

// #ifndef TIME_DST_OFFSET_SEC
// #define TIME_DST_OFFSET_SEC 0
// #endif

#include <Arduino.h>
#include <ESP32Time.h>

ESP32Time rtc(0);   // use UTC //was TIME_TIMEZONE_OFFSET_GMT_SEC
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

    packet[4] = 0;
    // All modules excluding display module, lives on UTC
    // packet[4] = (0 / 900)    // timezone offst 0 .. 96
    //      | (0 ? 128 : 0);
    // packet[4] = (TIME_TIMEZONE_OFFSET_GMT_SEC / 900)    // timezone offst 0 .. 96
    //     | (TIME_DST_OFFSET_SEC ? 128 : 0);

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

    // TODO remove it, but check display module
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


bool time_is_initialized() {
    return rtc_time_initialized;
}

/**
 * @brief Returns current day of week (0=sunday, 6=saturday)
 */
uint8_t time_get_day_of_week() {
    // TODO test it!
    // Code copypasted from ESP32Time.cpp
    // return rtc.getDayofWeek();

    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    // time_t tt = mktime (&timeinfo);
    // struct tm * tn = localtime(&tt);

    return timeinfo.tm_wday;
}

#endif