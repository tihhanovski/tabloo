#include <iostream>
#include <fstream>
#include "Timetables.h"
#include <time.h>

#include <chrono>
#include <thread>

#include <sys/resource.h>

long getMemoryUsage() 
{
  struct rusage usage;
  if(0 == getrusage(RUSAGE_SELF, &usage))
    return usage.ru_maxrss; // bytes
  else
    return 0;
}
using namespace std;

char* buffer = nullptr;
size_t size = 0;

void loadData()
{
  if(buffer != nullptr)
    delete[] buffer;
  ifstream file("x.txt", ios::in|ios::binary|ios::ate);
  if(file.is_open())
  {
    size = file.tellg();
    buffer = new char[size];
    file.seekg (0, ios::beg);
    file.read(buffer, size);
    file.close();
  }  
}

int main(){
  int cnt = 0;
  time_t rawtime;
  struct tm * ptm;

  cout << cnt << ": mem: " << getMemoryUsage() << "\n";
  
//  loadData();
  
  while(true)
  {
    time ( &rawtime );
    ptm = gmtime ( &rawtime );  
    //cout << ptm->tm_hour << ":" << ptm->tm_min << "\n";
    unsigned int now = (ptm->tm_hour + 2) % 24 * 3600 
        + ptm->tm_min * 60;
    
    if(!(cnt % 1000))
    {
      cout << "*** LOAD DATA\n";
      loadData();
    }
    
    loadTimes(buffer);
    
    if(!(cnt % 100))
    {
      size_t maxCnt = 3;
      for(size_t i = 0; maxCnt && (i < timesCount); i++)
      {
        TimeData t = times[i];
        if(t.secs() >= now)
        {
          maxCnt--;
          cout << i << "\t" << (int)t.hours << ":" << (int)t.minutes << "\t" << t.secs() << "\t" << t.line->shortName << "\t" << t.line->longName << "\n";
        }
      }

      cout << cnt << ": mem: " << getMemoryUsage() << "\n";
      cout << "====================\n";      
    }

    cnt++;
    using namespace std::this_thread; // sleep_for, sleep_until
    using namespace std::chrono; // nanoseconds, system_clock, seconds

    sleep_for(nanoseconds(10));
    sleep_until(system_clock::now() + milliseconds(10));
    
  }
}