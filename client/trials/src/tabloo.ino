#define DATA_LOADING_INTERVAL 60000         //in milliseconds
#define REFRESH_BUS_LIST_INTERVAL 60000     //in milliseconds
#define SCROLL_SPEED 10                     //letters per second
#define DEFAULT_BUS_LIST_COUNT 3

unsigned long int timeToLoadData;
unsigned long int timeToRefreshBusList;

char* stringToScroll = nullptr;
unsigned long int lineNoToScroll = 0;
unsigned long int scrollPos = 0;	//scroll to left only!
unsigned long int scrollStarted = 0;

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

    unsigned int getScrollHeaderLength()
    {
        return 6 + strlen(line->shortName);
    }

    unsigned int getScrollBodyLength()
    {
        return strlen(line->longName) + strlen(line->headSign) + 1;
    }
};

unsigned int loadedSecs;
unsigned int deltaSecs;
unsigned int timesCount;
LineData* lines = nullptr;
TimeData* times = nullptr;
char** busList = nullptr;
char** timesList = nullptr;
unsigned int busListCount = 0;


char* downloadTimetable()
{
    //Stub
    //TODO: rewrite for actual device
    return getDataFromInternet();
}

void loadTimes()
{
    char* data = downloadTimetable();

    if(lines != nullptr)
        delete[] lines;
    if(times != nullptr)
        delete[] times;

    loadedSecs = data[0] * 3600 + data[1] * 60 + data[2];
    deltaSecs = millis() / 1000;

    char* p = data + 3;

    unsigned char lineCount = *p;
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

unsigned int startTime = 0;
char timeBuffer[6];

unsigned int currentTimeSeconds()
{
    return millis() / 1000 - deltaSecs + loadedSecs;
}

void encodeTime(char* buffer, unsigned char hours, unsigned char minutes)
{
    sprintf(buffer, "%02d:%02d", hours, minutes);
}

char* getCurrentTimeStr()
{
    unsigned int msc = currentTimeSeconds();
    int msc2 = msc / 60;
    encodeTime(timeBuffer, msc2 / 60, msc2 % 60);
    if(msc % 2)
        timeBuffer[2] = ' ';
    return timeBuffer;
}

void scroll()
{
    if(busListCount < 1)
	{
		lcd->setCursor(0, 0);
		lcd->print("** no buses");
		return;
	}

    if(stringToScroll == nullptr)
	{
		stringToScroll = busList[lineNoToScroll];
		//cout << endl << "*** new stringToScroll: '" << stringToScroll << "'" << endl;
	}

    if(scrollStarted == 0)
		scrollStarted = millis();

    size_t newScrollPos = SCROLL_SPEED * (millis() - scrollStarted) / 1000;
	if(newScrollPos != scrollPos)
	{
		scrollPos = newScrollPos;
		if(scrollPos > strlen(stringToScroll) - lcd->width)
		{
			lineNoToScroll = (lineNoToScroll + 1) % busListCount;
			stringToScroll = busList[lineNoToScroll];
			scrollPos = 0;
			scrollStarted = 0;
			return;
		}
        lcd->setCursor(0, 0);
        lcd->print(timesList[lineNoToScroll]);
		lcd->setCursor(6, 0);
		lcd->print(stringToScroll + scrollPos);

        lcd->setCursor(0, 1);
        lcd->print(getCurrentTimeStr());
	}

}

void checkDataLoaded()
{
	if(timeToLoadData < millis())
	{
		loadTimes();
		timeToLoadData = millis() + DATA_LOADING_INTERVAL;
        timeToRefreshBusList = millis();
	}
}

void makeBusList()
{
    if(busList != nullptr)
    {
        for(unsigned int i = 0; i < busListCount; i++)
        {
            delete busList[i];
            delete timesList[i];
        }
        delete[] busList;
        delete[] timesList;
    }

    unsigned int cs = currentTimeSeconds();

    busList = new char*[DEFAULT_BUS_LIST_COUNT];
    timesList = new char*[DEFAULT_BUS_LIST_COUNT];
    unsigned int bc = 0;
    for(unsigned int i = 0; i < timesCount; i++)
        if(times[i].secs() > cs)
        {
            timesList[bc] = new char[6];
            encodeTime(timesList[bc], times[i].hours, times[i].minutes);
            unsigned int ll = times[i].getScrollHeaderLength() + times[i].getScrollBodyLength() + 2;
            busList[bc] = new char[ll];
            for(unsigned int j = 0; j < ll; j++)
                busList[bc][j] = ' ';
            memcpy(busList[bc], times[i].line->shortName, strlen(times[i].line->shortName));
            memcpy(busList[bc] + strlen(times[i].line->shortName) + 2, times[i].line->longName, strlen(times[i].line->longName));
            busList[bc][ll - 1] = '\0';
            bc++;

            if(bc > DEFAULT_BUS_LIST_COUNT)
                break;
        }
    busListCount = DEFAULT_BUS_LIST_COUNT;

    for(unsigned int i = 0; i < busListCount; i++)
        cout << i << ": '" << busList[i] << "'" << endl;
}

void checkBusListMade()
{
    if(timeToRefreshBusList < millis())
	{
		makeBusList();
		lcd->cls();
        timeToRefreshBusList = millis() + REFRESH_BUS_LIST_INTERVAL;
	}
}

void setup()
{
	timeToLoadData = millis();
}

void loop()
{
	checkDataLoaded();
	checkBusListMade();
	scroll();
	//printTime();
}
