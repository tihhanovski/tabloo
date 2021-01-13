//arduino stubs
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
#include <curl/curl.h>

#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#define OUTPUT_SLEEP_MICROSEC 1000

using namespace std;

unsigned long msst = 0;
unsigned long millis()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	unsigned long r = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	if(msst == 0)
		msst = r;
	return r - msst;
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

void debug(string s)
{
    cout << s << endl;
}

size_t writeCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

char* getDataFromInternet()
{
    curl_global_init(CURL_GLOBAL_ALL);

    CURL* easyhandle = curl_easy_init();
    std::string readBuffer;

    curl_easy_setopt(easyhandle, CURLOPT_URL, "https://dev.intellisoft.ee/tabloo/ask/?c=7820162-1");
    //curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);
    //curl_easy_setopt(easyhandle, CURLOPT_PROXY, "http://my.proxy.net");   // replace with your actual proxy
    //curl_easy_setopt(easyhandle, CURLOPT_PROXYPORT, 8080L);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &readBuffer);

    curl_easy_perform(easyhandle);

    size_t len = readBuffer.length();
	char* data = new char[len + 1];
	readBuffer.copy(data, len, 0);
    return data;
}
