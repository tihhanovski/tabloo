/**
 * Tabloo - opensource bus stop display
 * key-value storage
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com> 
 * 
 * TODO: really bad bug when overwrite existing keys value. Data gets corrupted
 * TODO: just use third party working key-value class?
 */

#include "FS.h"
#include "SPIFFS.h"

#ifndef TABLOO_SETTINGS

#define TABLOO_SETTINGS included

// If storage size is not defined, set it to default value
#ifndef STORAGE_SIZE
  #define STORAGE_SIZE 200
#endif

// Set storage path to default value if not set up yet
#ifndef SETTINGS_STORAGE_PATH
  #define SETTINGS_STORAGE_PATH "/settings.data"
#endif

// Internal types definitions
#define keypos_t uint8_t
#define keysize_t uint8_t
#define valsize_t uint8_t

// Used when key not found
#define IMPOSSIBLE_KEYPOS 255

/**
 * Storage class
 * Storage is loaded from SPIFFS as sequence of keys and values separated by \\0 character.
 * 
 * Example:
 * key1\0value1\0key2\0value2\0
 * After loading internal arrays of pointers to each key and value created.
 */
class Settings {

  char buffer[STORAGE_SIZE];  // Buffer for keys and values
  size_t keysUsed;            // keys counter
  size_t storageUsed;         // storage used in bytes
  keypos_t keyCount;          // max keys/values count in storage
  bool loaded = false;

  /** 
   * Find the position of given key
   * @param key key as c string
   * @return position of key/value in keys array or IMPOSSIBLE_KEYPOS if not found
   */
  keypos_t getPos(const char* key) {
    for(keypos_t i = 0; i < keysUsed; i++)
      if(!strcmp(key, keys[i]))
        return i;
    return IMPOSSIBLE_KEYPOS;
  }

  /**
   * Delete pair of key/value on given position
   * @param pos position in keys/values array
   */
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

  /**
   * Initialize keys and values arrays after buffer was loaded.
   */
  void map()
  {
    bool expectingKey = true;
    char* pos = buffer;
    keysUsed = 0;
    storageUsed = 0;
    while(pos < buffer + STORAGE_SIZE)
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
  
  // Getters for some internal stuff
  size_t getKeysUsed() { return keysUsed; }
  size_t getStorageUsed() { return storageUsed; }
  size_t getKeyCount() { return keyCount; }
  char* getBuffer() { return buffer; }
  
  //TODO move it to private part
  char** keys;      // pointers to keys
  char** values;    // pointers to characters

  /**
   * Empty the buffer.
   * Initialize the storage.
   */
  void clear() {
    memset(buffer, 0, STORAGE_SIZE);
    keysUsed = 0;
    storageUsed = 0;
  }

  /**
   * @param keyCount maximum amount of keys/values
   */
  Settings(keypos_t keyCount) {
    this->keyCount = keyCount;
    keys = new char*[keyCount];
    values = new char*[keyCount];
    clear();
  }
  
  ~Settings() {
    // Free arrays memory
    delete[] keys;
    delete[] values;
  }

  /** Get the value for the given key
   * @param key
   * @return value or nullptr if value does not exist
   */
  char* get(const char* key) {

    if(!loaded)
      load();

    keypos_t i = getPos(key);
    return i == IMPOSSIBLE_KEYPOS ? nullptr : values[i];
  }
      
  /**
   * Delete given key and corresponding value
   * @param key key to delete (c string)
   */
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
   * Delete key/value, if value is empty
   * @param key null terminated cstring
   * @param value null terminated cstring
   * @return true if succeeded, false otherwise
  */
  bool set(char* key, char* value) {  //Set value for given key

    if(!loaded)
      load();

    if(!(*value))               // TODO is it ok to check like this?
      return del(key);
    keypos_t i = getPos(key);
    valsize_t valSize = strlen(value);
    if(i == IMPOSSIBLE_KEYPOS)  // 
    {
      if(keysUsed > keyCount)   // no more place for keys
        return false;
      keysize_t keySize = strlen(key);        
      if(storageUsed + keySize + valSize + 2 > STORAGE_SIZE) // no more storage place
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
        if(storageUsed + delta > STORAGE_SIZE)
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

  /**
   * Load buffer from SPIFFS
   */
  void load()
  {
    Serial.println("*Loading storage");
    clear();
    File file = SPIFFS.open(SETTINGS_STORAGE_PATH);
    if(!file || file.isDirectory()){
      Serial.println("- failed to open file for reading");
      loaded = true;
      return;
    }

    size_t i = 0;

    Serial.println("- read from file:");
    while(file.available()){
      buffer[i++] = file.read();
      if(i > STORAGE_SIZE)
        break;
    }
    file.close();

    Serial.println("\n*Loaded");
    map();
    loaded = true;
  }

  /**
   * Load buffer to SPIFFS
   */
  void save()
  {
    Serial.println("*Saving storage");

    File file = SPIFFS.open(SETTINGS_STORAGE_PATH, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    for(size_t i = 0; i < STORAGE_SIZE; i++)
        file.write(buffer[i]);
    file.close();

    Serial.println("*Storage saved");
  }

  bool isLoaded() { return loaded; }
};

#endif