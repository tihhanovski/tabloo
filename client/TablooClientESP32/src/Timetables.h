/**
 * Tabloo - opensource bus stop display
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 * Timetable format handler
 * 
 * downloaded timetable format description (in estonian)
 * https://github.com/tihhanovski/tabloo/wiki/Server
 * 
 * Short format description
 *  current date and time (3 bytes)
 *      hours 0 - 23 (1 byte)
 *      minutes 0 - 59 (1 byte)
 *      seconds 0 - 59 (1 byte)
 * count of lines (1 byte)
 * Line names. Three zero terminated c strings for each line (x count of lines)
 *      Line short name ie line number (zero terminated c string)
 *      Line long name (zero terminated c string)
 *      Direction (zero terminated c string)
 * Times count (2 bytes): times count = <Times count MSB> * 256 + <Times count LSB>
 *      Times count MSB
 *      Times count LSB
 * Timetable
 *      Arrival time hours 0 - 23       (1 byte)
 *      Arrival time minutes 0 - 59     (1 byte)
 *      Line index (from line names)    (1 byte)
 */

#include <Arduino.h>
#define LINE_INDEX_NOT_FOUND 255
//#define DEBUG_TIMETABLES 1    // Uncomment to get debug information to Serial

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
class Timetable {
    size_t timesCount;              // Count of times in timetable
    char* timetable = nullptr;      // Pointer to first byte of times massive
public:
    LineData* lines = nullptr;      // Line data array
    uint8_t lineCount;              // Lines count


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
     * 
     * Find next bus arrival time for given line from timetable and return difference between it and current time in minutes.
     * If no next time found, use first time from timetable as next days time
     * TODO will produce wrong result if tomorrow times are different (for example weekend ws weekday schedule)
     */
    uint16_t getWaitingTime(uint8_t lineIndex, uint8_t h, uint8_t m)
    {
        uint16_t currentMin = h * 60 + m;               // Convert current time to minutes

        #ifdef DEBUG_TIMETABLES
            Serial.print("Looking for next time ");
            Serial.print(lineIndex);
            Serial.print("\t");
            Serial.print(h);
            Serial.print(":");
            Serial.print(m);
            Serial.print(" --> ");
            Serial.print(currentMin);
            Serial.println(":");
        #endif

        uint16_t ret = 0;                               // Will return it
        bool firstTimeSet = false;                  
        char* p = timetable;                            // Timetable start in memory
        for(size_t i = 0; i < timesCount; i++)          // Run through timetable
        {
            #ifdef DEBUG_TIMETABLES
                Serial.print("{t:");
                Serial.print((uint8_t)*(p));
                Serial.print(":");
                Serial.print((uint8_t)*(p+1));
                Serial.print(" L");
                Serial.print((uint8_t)*(p+2));
                Serial.print("}\t");
            #endif
            if(*(p + 2) == lineIndex) {                 // Check bus line index
                uint16_t t = (*p) * 60 + (*(p + 1));    // Calculate arrival time in minutes since midnight
                #ifdef DEBUG_TIMETABLES
                    Serial.print(t);
                    Serial.print("\t");
                #endif
                if(!firstTimeSet) {                     // Save first bus time for case if we have no next time
                    ret = t;
                    firstTimeSet = true;
                }

                if(t >= currentMin) {                  // Found first bus arrival since now
                    #ifdef DEBUG_TIMETABLES
                        Serial.println("OK");
                    #endif
                    return t - currentMin;             // Return the difference
                }

            }
            p += 3;                                    // Jump to the next time in timetable
        }

        #ifdef DEBUG_TIMETABLES
            Serial.println("def:");
        #endif
        return 24 * 60 - currentMin + ret;             // Use first time as tomorrows first time
    }

    /**
     * Initialize timetable with data received from server
     * @param data pointer to buffer fetched from server
     * Creates LineData instances for every line with pointers to names of lines
     * Saves address of beginning of timetable in buffer
     */
    void loadTimes(char* data)
    {
        if(data == nullptr)
        {
            lineCount = 0;
            timesCount = 0;
            return;
        }

        if(lines != nullptr)
            delete[] lines;

        char* p = data + 3;

        lineCount = *p;
        lines = new LineData[lineCount];
        p++;

        for(unsigned int i = 0; i < lineCount; i++)
        {
            lines[i].shortName = p;
            p = strchr(p, '\0') + 1;
            lines[i].longName = p;
            p = strchr(p, '\0') + 1;
            lines[i].headSign = p;
            p = strchr(p, '\0') + 1;
        }

        unsigned char timeCountHi = *p++;
        unsigned char timeCountLo = *p++;
        timesCount = 256 * timeCountHi + timeCountLo;

        timetable = p;  //Save timetable start address
    }
};
