#include <Arduino.h>

class LineData {
public:
    char* shortName;
    char* longName;
    char* headSign;
};

class TimeData {
public:
    unsigned char hours;
    unsigned char minutes;
    LineData* line;

    unsigned int secs() {
        return hours * 3600 + minutes * 60;
    }

};

class Timetable {
public:
    LineData* lines = nullptr;
    TimeData* times = nullptr;
    unsigned int timesCount;
    uint8_t lineCount;
    char* timetable = nullptr;

    uint8_t getLineIndex(char* shortName)
    {
        for(uint8_t i = 0; i < lineCount; i++)
            if(!strcmp(shortName, lines[i].shortName))
                return i;
        return 255; // TODO replace with constant
    }

    uint16_t getWaitingTime(uint8_t lineIndex, uint8_t h, uint8_t m)
    {
        uint16_t currentMin = h * 60 + m;

        Serial.print("Looking for next time ");
        Serial.print(lineIndex);
        Serial.print("\t");
        Serial.print(h);
        Serial.print(":");
        Serial.print(m);
        Serial.print(" --> ");
        Serial.print(currentMin);
        Serial.println(":");

        uint16_t ret = 0;
        bool firstTimeSet = false;
        char* p = timetable;
        for(size_t i = 0; i < timesCount; i++)
        {
            Serial.print("{t:");
            Serial.print((uint8_t)*(p));
            Serial.print(":");
            Serial.print((uint8_t)*(p+1));
            Serial.print(" L");
            Serial.print((uint8_t)*(p+2));
            Serial.print("}\t");
            if(*(p + 2) == lineIndex)
            {
                uint16_t t = (*p) * 60 + (*(p + 1));
                Serial.print(t);
                Serial.print("\t");
                if(!firstTimeSet)
                {
                    ret = t;
                    firstTimeSet = true;
                }

                if(t >= currentMin)
                {
                    Serial.println("OK");
                    return t - currentMin;
                }

            }
            p += 3;
        }

        Serial.println("def:");
        return 24 * 60 - currentMin + ret;
    }

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
        if(times != nullptr)
            delete[] times;

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

        times = new TimeData[timesCount];
        for(size_t i = 0; i < timesCount; i++)
        {
            times[i].hours = *p++;
            times[i].minutes = *p++;
            times[i].line = lines + *p++;
        }
    }
};
