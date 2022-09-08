#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <curl/curl.h>

#include "sys/types.h"
#include "sys/sysinfo.h"

#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#define OUTPUT_SLEEP_MICROSEC 1000
#define DATAFILE "d3.txt"

using namespace std;

unsigned long msst = 0;
unsigned long millis();

long long vmuStart = 0;

void outputMemStats()
{
	struct sysinfo memInfo;

	sysinfo (&memInfo);
	/*long long totalVirtualMem = memInfo.totalram;
	//Add other values in next statement to avoid int overflow on right hand side...
	totalVirtualMem += memInfo.totalswap;
	totalVirtualMem *= memInfo.mem_unit;*/

	long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
	if(vmuStart == 0)
		vmuStart = virtualMemUsed;

	cout << "*** memory delta: " << (virtualMemUsed - vmuStart) << endl;
}

string getSpace(size_t i)
{
	string s = "                                            ";
	return s.substr(0, i);
}

class LCD
{
public:
	size_t cnt = 0;

	string* lines;
	size_t width;
	size_t height;

	size_t posX = 0;
	size_t posY = 0;

	void setCursor(size_t x, size_t y)
	{
		posX = x;
		posY = y;
	}

	void print(string s)
	{
		size_t l = width < s.length() ? width : s.length();
		size_t c = 0;
		for(size_t x = posX; x < l; x++)
			lines[posY][posX++] = s[c++];
		lines[posY] = lines[posY].substr(0, width);
	}


	void print(unsigned long int i)
	{
		stringstream ss;
		ss << i;
		print(ss.str());
	}

	void cls()
	{
		string s = "";
		for(size_t x = 0; x < width; x++)
			s += " ";
		for(size_t x = 0; x < height; x++)
			lines[x] = s;

	}

	LCD(size_t w = LCD_WIDTH, size_t h = LCD_HEIGHT)
	{
		width = w;
		height = h;
		lines = new string[h];
		cls();
	}

	void output()
	{
		cnt++;
		cout << "LCD:|";

		for(size_t x = 0; x < height; x++)
			cout << lines[x] << "|";

		cout << "count: " << cnt << "; millis: " << millis();

		cout << "\r";
		usleep(OUTPUT_SLEEP_MICROSEC);
	}
};

unsigned long millis()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	unsigned long r = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	if(msst == 0)
		msst = r;
	return r - msst;
}

string getTimes2()
{
	string ret = "";
	std::ifstream i(DATAFILE, std::ios::in);
    if(i.is_open())
    {
        string line;
        while(getline(i, line))
            ret = ret + line + "\n";
		i.close();
	}
	return ret;
}

const char* getTimes()
{
	string s = getTimes2();
	size_t l = s.length();
	char* ret = new char[l + 1];
	s.copy(ret, l, 0);
	//ret[l] = '\0';
	return ret;
}

string getTime()
{
	time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buffer,80,"%H:%M:%S",timeinfo);

	return string(buffer);
}

string getTime1()
{
	auto t = chrono::system_clock::now();
	time_t tt = chrono::system_clock::to_time_t(t);
	return ctime(&tt);
}
