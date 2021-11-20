#include <iostream>
#include <fstream>

#define EEPROM_SIZE 50

#include "Storage.h"


using namespace std;

Storage s(10);

void outputStats()
{
  char* c = s.getBuffer();
  cout << "{";
  for(uint8_t i = 0; i < EEPROM_SIZE; i++)
    cout << (c[i] == 0 ? '_' : c[i]);
  cout << "} S=" << s.getStorageUsed() << "/" << EEPROM_SIZE 
      << "; K=" << s.getKeysUsed() << "/" << s.getKeyCount() << "\n ";
  for(keypos_t i = 0; i < s.getKeysUsed(); i++)
    cout << s.keys[i] << "=" << s.values[i] << ";";
  cout << "\n";
}

void set(char* key, char* value) {
  cout << "set('" << key << "', '" << value << "') "
    << (s.set(key, value) ? "success" : "fail") << "\n";
  outputStats();
}

int main(){ 
  cout << "Storage test\n";
  
  s.load();
  outputStats();
  
  set((char*)"a", (char*)"");

  set((char*)"a", (char*)"b");
  set((char*)"key1", (char*)"value1");
  set((char*)"c", (char*)"ddd");  
  set((char*)"a", (char*)"ololo");
  
  s.del((char*)"a");
  outputStats();
  s.del((char*)"c");
  outputStats();
  s.del((char*)"key1");
  outputStats();

  set((char*)"a", (char*)"b");
  set((char*)"key1", (char*)"value1");
  set((char*)"c", (char*)"ddd");  
  set((char*)"a", (char*)"ololo");
  set((char*)"a", (char*)"one");
  set((char*)"a", (char*)"two");
  set((char*)"a", (char*)"three");
  set((char*)"a", (char*)"four");
  
  outputStats();

  cout << "get a: " << s.get((char*)"a") << "\n";
  cout << "get key1: " << s.get((char*)"key1") << "\n";


  
//  cout << "set a = b " << storage.set((char*)"a", (char*)"b") ? "ok";
  
  s.save();
  
  return 0;
}
