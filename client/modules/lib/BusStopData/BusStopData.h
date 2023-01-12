#ifndef _BUSSTOPDATA_H_
#define _BUSSTOPDATA_H_

/**
 * Tabloo - opensource bus stop display
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 * Timetable format handler
 * 
 * Short format description
 * Header
 *      count of lines (1 byte)
 *      Times count (2 bytes): times count = <Times count MSB> * 256 + <Times count LSB>
 *          Times count MSB
 *          Times count LSB
 *      TZ -    timezone byte
 *      DST -   Daylight Saving data
 * Stop name (CString)
 * Line names. Three CStrings for each line (x count of lines)
 *      Line short name ie line number (CString)
 *      Line long name (CString)
 *      Direction (CString)
 * Timetable (4 bytes ber entry)
 *      Arrival time hours 0 - 23       (1 byte)
 *      Arrival time minutes 0 - 59     (1 byte)
 *      Line index (from line names)    (1 byte)
 *      Weekday mask                    (1 byte)
 */

#include <Arduino.h>
#define LINE_INDEX_NOT_FOUND 255
//#define DEBUG_TIMETABLES 1    // Uncomment to output debug information to Serial

/**
 * Bus line information
 */
class LineData {
public:
    char* shortName;
    char* longName;
    char* headSign;
};

/**
 * Timetable for one bus stop
 */
class BusStopData {
    size_t timesCount = 0;              // Count of times in timetable
    char* timetable = nullptr;          // Pointer to first byte of times massive
    char* stopName = nullptr;
public:
    LineData* lines = nullptr;          // Line data array
    uint8_t lineCount = 0;              // Lines count
    uint8_t tz = 0;                     // Timezone data with 15 minutes precision. tz * 900 = offset in seconds
    uint8_t dst = 0;                    // DST type
    char* tzName = nullptr;

    // TODO is it correct definition of DST?
    // #define	DST_NONE	0	/* not on dst */
    // #define	DST_USA		1	/* USA style dst */
    // #define	DST_AUST	2	/* Australian style dst */
    // #define	DST_WET		3	/* Western European dst */
    // #define	DST_MET		4	/* Middle European dst */
    // #define	DST_EET		5	/* Eastern European dst */
    // #define	DST_CAN		6	/* Canada */


    /** 
     * get Line index for given line number
     * @param shortName
     * @return line index or LINE_INDEX_NOT_FOUND if line with given number was not found
     */
    uint8_t getLineIndex(char* shortName)
    {
        for(uint8_t i = 0; i < lineCount; i++)
            if(!strcmp(shortName, lines[i].shortName))
                return i;
        return LINE_INDEX_NOT_FOUND;
    }

    /**
     * Calculate time to wait for bus to arrive
     * @param lineIndex line index in data retrieved from server
     * @param h current time hours (0 - 23)
     * @param m current time minutes (0 - 59)
     * @param dow day of week (0 - 6);
     * 
     * Find next bus arrival time for given line from timetable and return difference between it and current time in minutes.
     * If no next time found, use first time from timetable as next days time
     * TODO will produce wrong result if tomorrow times are different (for example weekend ws weekday schedule)
     */
    uint16_t getWaitingTime(uint8_t lineIndex, uint8_t h, uint8_t m, uint8_t dow)
    {
        uint16_t currentMin = h * 60 + m;               // Convert current time to minutes
        uint8_t dowMask = 1 << dow;

        #ifdef DEBUG_TIMETABLES
        log_v("Looking for next time %d\t%d:%d dow=%d, dowMask=%d, --> %d", lineIndex, h, m, dow, dowMask, currentMin);
        log_v("times count: %d", timesCount);
        #endif

        uint16_t ret = 0;                               // Will return it
        bool firstTimeSet = false;                  
        char* p = timetable;                            // Timetable start in memory
        for(size_t i = 0; i < timesCount; i++)          // Run through timetable
        {
            #ifdef DEBUG_TIMETABLES
            log_v("{t:%d:%d L%d}", (uint8_t)*(p), (uint8_t)*(p+1), (uint8_t)*(p+2));
            #endif
            if((*(p + 2) == lineIndex) && (*(p + 3) & dowMask)) {                 // Check bus line index and day of week mask
                uint16_t t = ((uint8_t)(*p)) * 60 + (uint8_t)(*(p + 1));    // Calculate arrival time in minutes since midnight
                #ifdef DEBUG_TIMETABLES
                log_v("compare with %d", t);
                #endif
                if(!firstTimeSet) {                     // Save first bus time for case if we have no next time
                    ret = t;
                    firstTimeSet = true;
                }

                if(t >= currentMin) {                  // Found first bus arrival since now
                    #ifdef DEBUG_TIMETABLES
                        log_v("OK");
                    #endif
                    return t - currentMin;             // Return the difference
                }
            }
            p += 4;                                    // Jump to the next time in timetable
        }

        #ifdef DEBUG_TIMETABLES
            log_v("def: %d", 24 * 60 - currentMin + ret);
        #endif
        return 24 * 60 - currentMin + ret;             // Use first time as tomorrows first time
    }

    void cleanup() {
        if(lines != nullptr) {
            delete[] lines;
            lines = nullptr;
        }
        timetable = nullptr;
        lineCount = 0;
        timesCount = 0;
        //sensors = nullptr;
    }

    /**
     * Initialize timetable with data received from server
     * @param data pointer to buffer fetched from server
     * Creates LineData instances for every line with pointers to names of lines
     * Saves address of beginning of timetable in buffer
     */
    void initialize(char* data) {
        cleanup();
        if(data == nullptr)
            return;

        char* p = data;

        lineCount = *p++;                               // 0    lines count (0 - 255)
        uint8_t timeCountHi = *p++;                     // 1    timesCount MSB
        uint8_t timeCountLo = *p++;                     // 2    timesCount LSB
        tz = *p++;                                      // 3    timezone
        dst = *p++;                                     // 4    DST
        timesCount = 256 * timeCountHi + timeCountLo;
        lines = new LineData[lineCount];

        log_v("lines: %d, times: %d, TZ: %d, DST: %d", lineCount, timesCount, tz, dst);

        stopName = p;                                   // 5    stop name begins
        log_v("Stop name: '%s'", stopName);
        p = strchr(p, '\0') + 1;
        tzName = p;
        p = strchr(p, '\0') + 1;

        for(unsigned int i = 0; i < lineCount; i++) {   //      line names
            lines[i].shortName = p;
            p = strchr(p, '\0') + 1;
            lines[i].longName = p;
            p = strchr(p, '\0') + 1;
            lines[i].headSign = p;
            p = strchr(p, '\0') + 1;

            log_v("Line #%d: '%s': '%s' / '%s'", i, lines[i].shortName, lines[i].longName, lines[i].headSign);
        }


        timetable = p;                                  //      Save timetable start address

    }
};

#endif