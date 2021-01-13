#include <iostream>
#include <string>
#include <curl/curl.h>
#include <string.h>

//code from here
//https://stackoverflow.com/questions/44994203/how-to-get-the-http-response-string-using-curl-in-c/45017565

//Compile it with -lcurl:
//g++ src/testcurl.cpp -o bin/testcurl -Wall -lcurl

using namespace std;

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

class LineData
{
public:
    char* shortName;
    char* longName;
    char* headSign;

    void print()
    {
        cout << shortName << " " << longName << " -> " << headSign << endl;
    }
};

struct TimeData
{
    char hours;
    char minutes;
    LineData* line;
};

int main(int argc, char * argv[])
{
    char* data = getDataFromInternet();

    unsigned char lineCount = *data;

    cout << "lines: " << (int)lineCount << endl;

    LineData* lines = new LineData[lineCount];
    char* p = data + 1;
    //char* p1;
    for(size_t i = 0; i < lineCount; i++)
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
    size_t timesCount = 256 * timeCountHi + timeCountLo;

    cout << "times:" <<  (int)timeCountHi << " * 256 + " << (int)timeCountLo << " = " << timesCount << endl;

    TimeData* times = new TimeData[timesCount];
    for(size_t i = 0; i < timesCount; i++)
    {
        times[i].hours = *p++;
        times[i].minutes = *p++;
        times[i].line = lines + *p++;
    }


    for(size_t i = 0; i < timesCount; i++)
        cout << (int)times[i].hours << ":" << (int)times[i].minutes << " "
        << times[i].line->shortName << " " << times[i].line->longName << " -> " << times[i].line->headSign
        << endl;

    return 0;
}
