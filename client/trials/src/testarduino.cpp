#include <string.h>
#include "arduino-stubs.hpp"

//Compile it with -lcurl:
//g++ src/testcurl.cpp -o bin/testcurl -Wall -lcurl

LCD* lcd;

#include "tabloo.ino"

int main()
{
	lcd = new LCD();

	setup();
	while(true)
    	try
    	{
    		loop();
    		lcd->output();
    	}
    	catch(int e)
    	{
            debug("***got exception " + to_string(e));
        }

	return 0;
}
