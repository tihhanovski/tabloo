#include <iostream>

using namespace std;


class LineData
{
public:
    char* shortName;
    char* longName;
    char* headSign;
};

class TimeData
{
public:
    unsigned char hours;
    unsigned char minutes;
    LineData* line;

    unsigned int secs()
    {
        return hours * 3600 + minutes * 60;
    }

};

LineData* lines = nullptr;
TimeData* times = nullptr;
unsigned int timesCount;
uint8_t lineCount;

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

    //loadedSecs = data[0] * 3600 + data[1] * 60 + data[2];
    //deltaSecs = millis() / 1000;

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

    times = new TimeData[timesCount];
    for(size_t i = 0; i < timesCount; i++)
    {
        times[i].hours = *p++;
        times[i].minutes = *p++;
        times[i].line = lines + *p++;
    }
}