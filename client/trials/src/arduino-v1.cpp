#include <iostream>
#include <string>
#include "ardstubs.cpp"

#include <string.h>
#include "TimeTableRow.hpp"
#define SIZE_T unsigned long int

using namespace std;


LCD* lcd;

TimeTableRow* timetable;

void loadTimes()
{
	//delete list
	TimeTableRow* t = timetable;
	while(t != nullptr)
	{
		TimeTableRow* t1 = t->next;
		delete t;
		t = t1;
	}

	char* s = (char*)getTimes();
	cout << s << endl << endl;
	TimeTableRow* last = nullptr;

	char* pch = s;
	SIZE_T l1;
	SIZE_T l = strlen(s);

    while (pch != nullptr)
    {
		pch++;
		char* pch1 = strchr(pch, '\n');

		if(pch1 == nullptr)
			l1 = s - pch + l;
		else
			l1 = pch1 - pch;

		if(l1 > 0)
		{
			char* line = new char[l1 + 1];
			memcpy(line, pch, l1);
			line[l1] = '\0';
			TimeTableRow* n = new TimeTableRow(line);
			if(timetable == nullptr)
				timetable = n;
			if(last != nullptr)
				last->next = n;
			last = n;
		}

		pch = pch1;
    }

	outputMemStats();
}

void outputAllTimes()
{
	int cnt = 0;
	TimeTableRow* p = timetable;
	while(p != nullptr)
	{
		cout << (++cnt) << "\t" << p->toString() << endl;
		p = p->next;
	}
}

void outputForCurrentTime(const string& fromTime, int count)
{
	int cnt = 0;
	for(TimeTableRow* p = timetable; p != nullptr && cnt < count; p = p->next)
	{
		if(p->time >= fromTime)
		{
			cout << p->toString() << endl;
			cnt++;
		}
	}
}

#define BUS_LIST_SIZE 3

unsigned long int timeToLoadData;
unsigned long int dataLoadingInterval = 1000;	//2 sec
string* busList;
size_t busListCount;

void setup()
{
	timeToLoadData = millis();
	timetable = nullptr;
}

void makeBusList(const string& fromTime, size_t count)
{
	if(busList != nullptr)
		delete[] busList;
	busList = new string[count];
	busListCount = 0;
	for(TimeTableRow* p = timetable; p != nullptr && busListCount < count; p = p->next)
	{
		if(p->time >= fromTime)
			busList[busListCount++] = p->toString() + getSpace(lcd->width);
	}
}

void checkDataLoaded()
{
	if(timeToLoadData < millis())
	{
		loadTimes();
		timeToLoadData = millis() + dataLoadingInterval;
	}
}

unsigned long int timeToRefreshBusList;
unsigned long int refreshBusListInterval = 30000;	//30 sec

string stringToScroll = "";
size_t lineNoToScroll = 0;
int scrollSpeed = 10;	//letter per second
size_t scrollPos = 0;	//scroll to left only!
unsigned long int scrollStarted = 0;

void scroll()
{
	if(busListCount < 1)
	{
		lcd->setCursor(0, 0);
		lcd->print("** no buses");
		return;
	}

	if(stringToScroll == "")
	{
		stringToScroll = busList[lineNoToScroll];
		cout << endl << "*** new stringToScroll: '" << stringToScroll << "'" << endl;
	}

	if(scrollStarted == 0)
		scrollStarted = millis();

	size_t newScrollPos = scrollSpeed * (millis() - scrollStarted) / 1000;
	if(newScrollPos != scrollPos)
	{
		scrollPos = newScrollPos;
		if(scrollPos > stringToScroll.length() - lcd->width)
		{
			//cout << endl << "*** switch bus" << endl;
			lineNoToScroll = (lineNoToScroll + 1) % busListCount;
			stringToScroll = getSpace(lcd->width) + busList[lineNoToScroll];
			//cout << "*** new stringToScroll: '" << stringToScroll << "'" << endl;
			scrollPos = 0;
			scrollStarted = 0;
			return;
		}
		lcd->setCursor(0, 0);
		lcd->print(stringToScroll.substr(scrollPos));
	}
}

void printTime()
{
	lcd->setCursor(0, 1);
	lcd->print(getTime() + " " +
		to_string(lineNoToScroll) + "/" + to_string(busListCount)
		+ " " + stringToScroll.substr(9, 2)
	);
	//lcd->print(" #");
	//lcd->print(lineNoToScroll);
}


void checkBusListMade()
{
	if(timeToRefreshBusList < millis())
	{
		timeToRefreshBusList = millis() + refreshBusListInterval;
		makeBusList(getTime(), BUS_LIST_SIZE);
		lcd->cls();
	}

	//lcd->setCursor(0, 0);
	//lcd->print(busList[0]);
}


void loop()
{
	checkDataLoaded();
	checkBusListMade();
	scroll();
	printTime();
}

int main()
{
	//char mystr[] = "test string";
	//cout << mystr << " : " << strlen(mystr) << endl;



	lcd = new LCD();
	loadTimes();
	//outputAllTimes();
	cout << getTime() << endl;
	cout << "1 % 3 = " << (1 % 3) << endl;
	//outputForCurrentTime(getTime(), 3);
	makeBusList(getTime(), 3);
	//debug
	cout << endl;
	for(size_t c = 0; c < busListCount; c++)
		cout << busList[c] << endl;

	cout << "-----------------------------------------\n";

	setup();
	while(true)
	try
	{
		loop();
		lcd->output();
	}
	catch(int e)
	{}

	cout << "\n--------------------------\n";

	return 0;
}
