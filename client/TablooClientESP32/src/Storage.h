// see https://randomnerdtutorials.com/esp32-flash-memory/

#include <EEPROM.h>
#include <nvs_flash.h>

#ifndef EEPROM_SIZE
  #define EEPROM_SIZE 200
#endif

#define keypos_t uint8_t
#define keysize_t uint8_t
#define valsize_t uint8_t
#define IMPOSSIBLE_KEYPOS 255

class Storage {

  char buffer[EEPROM_SIZE];
  size_t keysUsed;
  size_t storageUsed;
  keypos_t keyCount;
  bool loaded = false;

  keypos_t getPos(const char* key) {
    for(keypos_t i = 0; i < keysUsed; i++)
      if(!strcmp(key, keys[i]))
        return i;
    return IMPOSSIBLE_KEYPOS;
  }

  void deletePair(keypos_t pos) {
    size_t bytesToClean;
    if(pos < keysUsed - 1) {
      size_t bytesToMove = buffer + storageUsed - keys[pos + 1];
      bytesToClean = keys[pos + 1] - keys[pos];
      memcpy(keys[pos], keys[pos + 1], bytesToMove);

      for(keypos_t i = pos; i < keysUsed - 1; i++) {
        keys[i] = keys[i + 1] - bytesToClean;
        values[i] = values[i + 1] - bytesToClean;
      }
    }
    else
      bytesToClean = storageUsed - (keys[pos] - buffer);
    
    memset(buffer + storageUsed - bytesToClean, 0, bytesToClean);

    storageUsed -= bytesToClean;
    keysUsed--;
  }

  void map()
  {
    bool expectingKey = true;
    char* pos = buffer;
    keysUsed = 0;
    storageUsed = 0;
    while(pos < buffer + EEPROM_SIZE)
    {
      if(*pos == 0)
        return;
      if(expectingKey)
      {
        keys[keysUsed] = pos;
        pos += strlen(pos) + 1;
        storageUsed = pos - buffer;
        expectingKey = false;
      }
      else
      {
        values[keysUsed] = pos;
        pos += strlen(pos) + 1;
        storageUsed = pos - buffer;
        keysUsed++;
        expectingKey = true;
      }
    }
  }

public:
  
  size_t getKeysUsed() { return keysUsed; }
  size_t getStorageUsed() { return storageUsed; }
  size_t getKeyCount() { return keyCount; }
  
  char** keys;
  char** values;

  void clear() {
    memset(buffer, 0, EEPROM_SIZE);
    keysUsed = 0;
    storageUsed = 0;
  }

  Storage(keypos_t keyCount) {

    //nvs_flash_init();

    this->keyCount = keyCount;
    EEPROM.begin(EEPROM_SIZE);
    keys = new char*[keyCount];
    values = new char*[keyCount];

    clear();
  }
    
  char* getBuffer() {
    return buffer;
  }

  ~Storage() {
    delete[] keys;
    delete[] values;
  }

  char* get(const char* key) {

    if(!loaded)
      load();

    keypos_t i = getPos(key);
    return i == IMPOSSIBLE_KEYPOS ? nullptr : values[i];
  }
      
  bool del(char* key) {

    if(!loaded)
      load();

    keypos_t p = getPos(key);
    if(p == IMPOSSIBLE_KEYPOS)
      return false;
    else
    {
      deletePair(p);
      return true;
    }
  }

  /**
   * Set value for given key
  */
  bool set(char* key, char* value) {  //Set value for given key

    if(!loaded)
      load();

    if(!(*value))
      return del(key);
    keypos_t i = getPos(key);
    valsize_t valSize = strlen(value);
    if(i == IMPOSSIBLE_KEYPOS)  // 
    {
      if(keysUsed > keyCount)   // no more place for keys
        return false;
      keysize_t keySize = strlen(key);        
      if(storageUsed + keySize + valSize + 2 > EEPROM_SIZE) // no more storage place
        return false;
      
      memcpy(buffer + storageUsed, key, keySize);
      memcpy(buffer + storageUsed + keySize + 1, value, valSize);
      
      keys[keysUsed] = buffer + storageUsed;
      values[keysUsed] = buffer + storageUsed + keySize + 1;

      keysUsed++;
      storageUsed += keySize + valSize + 2;
    }
    else
    {
      size_t vLen = strlen(value);
      size_t delta = vLen - strlen(values[i]);
      if(delta) {
        if(storageUsed + delta > EEPROM_SIZE)
          return false;
        if(i < keysUsed - 1) {
          memcpy(keys[i + 1] + delta, keys[i + 1], buffer + storageUsed - keys[i + 1]);
          if(delta < 0)
            memset(buffer + storageUsed + delta, 0, -delta);
        }
        storageUsed += delta;
      }
      memcpy(values[i], value, vLen + 1);
      
      for(keypos_t j = i + 1; j < keysUsed; j++)
      {
        keys[j] += delta;
        values[j] += delta;
      }
    }
    
    return true;
  }

  void load()
  {
    Serial.println("*Loading storage");
    for(size_t i = 0; i < EEPROM_SIZE; i++)
    {
      buffer[i] = EEPROM.read(i);
      Serial.print(buffer[i] == 0 ? '_' : buffer[i]);
    }
    Serial.println("\n*Loaded");
    map();
    loaded = true;
  }

  void save()
  {
    Serial.println("*Saving storage");
    for(size_t i = 0; i < EEPROM_SIZE; i++)
      EEPROM.write(i, buffer[i]);
  }

  bool isLoaded() { return loaded; }
};