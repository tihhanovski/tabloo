#define DATA_LOADING_INTERVAL 60000          //in milliseconds
#define REFRESH_BUS_LIST_INTERVAL 60000     //in milliseconds
#define SCROLL_SPEED 10                     //letters per second
#define DEFAULT_BUS_LIST_COUNT 3

unsigned long int timeToLoadData;
unsigned long int timeToRefreshBusList;
unsigned int busListCount = 0;

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
    char hours;
    char minutes;
    LineData* line;

    unsigned int getScrollHeaderLength()
    {
        return 6 + strlen(line->shortName);
    }

    unsigned int getScrollBodyLength()
    {
        return strlen(line->longName) + strlen(line->headSign) + 1;
    }
};

unsigned int timesCount;
LineData* lines = nullptr;
TimeData* times = nullptr;
char** busList = nullptr;


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

    unsigned char lineCount = *data;

    //cout << "lines: " << (int)lineCount << endl;

    lines = new LineData[lineCount];
    char* p = data + 1;
    //char* p1;
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

    //cout << "times:" <<  (int)timeCountHi << " * 256 + " << (int)timeCountLo << " = " << timesCount << endl;

    times = new TimeData[timesCount];
    for(size_t i = 0; i < timesCount; i++)
    {
        times[i].hours = *p++;
        times[i].minutes = *p++;
        times[i].line = lines + *p++;
    }


    /*
    for(size_t i = 0; i < timesCount; i++)
        cout << (int)times[i].hours << ":" << (int)times[i].minutes << " "
        << times[i].line->shortName << " " << times[i].line->longName << " -> " << times[i].line->headSign
        << endl;
    */

    cout << "Loaded data " << endl;
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
		cout << endl << "*** new stringToScroll: '" << stringToScroll << "'" << endl;
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
		lcd->print(stringToScroll + scrollPos);
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
            delete busList[i];
        delete[] busList;
    }

    busList = new char*[DEFAULT_BUS_LIST_COUNT];
    unsigned int bc = 0;
    for(unsigned int i = 0; i < timesCount; i++)
    {
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
