#include <Arduino.h>

#include "SPIFFS.h"

#define EEPROM_SIZE 100     // Setup EEPROM storage size
#include "Settings.h"
Settings settings(10);      // TODO set proper max keys count

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "Connection.h"
#include <HTTPClient.h>

#include "Marquee.h"
#include "TalTechLogo.h"
#include "Util.h"
#include "BLESetup.h"

#include "Timetables.h"

/**
 * Settings could be done via serial port: 
 * Commands:
 *  setup list - return setup variables
 *  setup set <var name>=<var value> - set a variable
 *  setup set <var name> - delete variable
 *  setup save - save and restart
 * 
 * Setup variables:
 *  si - bus stop ID
 *  ws - wifi SSID
 *  wp - wifi password
 */

#define MAX_SERIAL_COMMAND_SIZE 100

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32      // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1       // Total number of panels chained one to another

MatrixPanel_I2S_DMA *display = nullptr;

const uint16_t black = display->color444(0, 0, 0);

//#define BUSSTOP_ID "7820161-1"

/** @brief Buffer for data retrieved from server
 */
char* data;
size_t dataSize; /** @brief size of data */

Marquees scrolls;

Timetable timetable;

unsigned long timeDelta;

void setTime(uint8_t h, uint8_t m, uint8_t s) {
  timeDelta = 1000 * (h * 3600 + m * 60 + s) - millis();
}

const char* SETUP_BUSSTOP_ID = "si";
const char* SETUP_WIFI_SSID = "ws";
const char* SETUP_WIFI_PASSWORD = "wp";
//const char* S

void setDemoData() {
    scrolls.add(
    new Marquee(
      "Tere, Marta!", display, 
      0, 
      0, 
      display->width(), 
      8, 
      100, 
      black
    ));

  scrolls.add(
    new Marquee(
      "Kuidas sul läheb?", display, 
      0, 
      8, 
      display->width(), 
      8, 
      200, 
      black
      ));

  scrolls.add(
    new Marquee(
      "Loll koll pull pall", display, 
      0, 
      16, 
      display->width(), 
      8, 
      300, 
      black
      ));

  scrolls.add(
    new Marquee(
      "Short scroller", display, 
      0, 
      24, 
      36, 
      8,
      50,
      black
      ));

}

void displayError(const char* msg)
{
  scrolls.clear();
  display->fillRect(0, 0, display->width(), display->height(), black);
  scrolls.add(new Marquee(msg, display, 0, 0, display->width(), 8, 300, black));
}

void loadData() {

  char* stopId = settings.get(SETUP_BUSSTOP_ID);
  if(stopId == nullptr)
  {
    setTime(0, 0, 0);
    displayError("No stop ID in setup, please set");
    return;
  }

  fetchData(stopId, data, dataSize);

  Serial.print("Setting time ... ");
  setTime(data[0], data[1], data[2]);
  Serial.println("OK");

  timetable.loadTimes(data);

  //LineData* lines = new LineData[data[3]];

  //const char* addr = data + 4;
  scrolls.clear();
  display->setTextColor(display->color444(0,0,15));
  for(uint8_t i = 0; i < timetable.lineCount; i++)
  {
    Serial.print("Line ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(timetable.lines[i].shortName);
    Serial.print("\t");
    Serial.print(timetable.lines[i].longName);
    Serial.print("\t");
    Serial.print(timetable.lines[i].headSign);
    Serial.print("\n");

    display->fillRect(0, i * 8, 12, 8, black);
    display->setCursor(0, i * 8);
    display->print(timetable.lines[i].shortName);
    
    scrolls.add(new Marquee(
      timetable.lines[i].longName, display, 
      12, 
      i * 8,  //TODO 
      display->width() - 24, 
      8, 
      300, 
      black
      ));
  }

  outputMemoryData();
}

void start()
{
  Serial.println("Starting");
  // Show beautiful splash screen with taltech logo
  display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
  display->setTextColor(display->color444(15,0,0));
  display->fillRect(0, 0, display->width(), display->height(), black);
  display->drawBitmap(0, 0, TalTechLogo, 64, 32, black, display->color444(15,15,15));
  delay(1500);

  // Clear the screen 
  display->fillRect(0, 0, display->width(), display->height(), black);
  
  loadData(); 

}

bool startSuccessfull;

void setup() {

  startSuccessfull = false;

  Serial.begin(115200);

  // Start internal filesystem
  // Settings (and more) is stored here
  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS Mount Failed");
  }

  outputMemoryData();

  // Module configuration
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN    // Chain length
  );

  mxconfig.gpio.e = 18;
  mxconfig.clkphase = false;
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  // Display Setup
  display = new MatrixPanel_I2S_DMA(mxconfig);
  display->begin();
  display->setBrightness8(30); //0-255
  display->clearScreen();
  display->fillScreen(black);

  display->setTextSize(1);     // size 1 == 8 pixels high
  display->setTextWrap(true); // Don't wrap at end of line - will do ourselves
  display->setTextColor(display->color444(15,15,15));

  startBleSetup();


  display->setCursor(0, 0);
  display->print("Connecting...");

  const char* wifiSSID = settings.get(SETUP_WIFI_SSID);
  if(wifiSSID == nullptr)
  {
    Serial.println("No Wifi SSID found in settings.");
    return;
  }

  // Start Wifi connection
  startConnection(wifiSSID, settings.get(SETUP_WIFI_PASSWORD));  
  start();

  startSuccessfull = true;
  //display->fillRect(0, 0, display->width(), display->height(), black);

}

// Will repaint clock area twice per second
unsigned long nextTimeTime = 0;
bool timeColon = false;
void clock_show()
{
  unsigned long t = millis();
  if(t >= nextTimeTime)
  {
    nextTimeTime = t + 500;
    timeColon = !timeColon;
    int x = 39;
    display->fillRect(x, 24, 30, 8, black);
    //display->setCursor(x, 24);
    //display->print(tz.dateTime(timeColon ? "GG:i" : "GG i"));

    unsigned long tt = (timeDelta + t) / 60000;

    uint8_t h = (tt / 60) % 24;
    uint8_t m = tt % 60;

    String hh = (h < 10 ? "0" : "") + String(h);
    String mm = (m < 10 ? "0" : "") + String(m);

    display->setTextColor(display->color444(0,15,0));
    display->setCursor(x, 24);
    display->print(hh);
    if(timeColon)
    {
      display->setCursor(x + 10, 24);
      display->print(":");
    }
    display->setCursor(x + 14, 24);
    display->print(mm);
  }
}

char serialCommandBuffer[MAX_SERIAL_COMMAND_SIZE] = {0};
char* serialCommandCurrentByte = serialCommandBuffer;

const char* CMD_SETUP_SAVE = "setup save";
//const size_t CMD_SETUP_SAVE_LEN = strlen(CMD_SETUP_SAVE);
const char* CMD_SETUP_SET = "setup set ";
const size_t CMD_SETUP_SET_LEN = strlen(CMD_SETUP_SET);
const char* CMD_SETUP_LIST = "setup list";
//const size_t CMD_SETUP_LIST_LEN = strlen(CMD_SETUP_LIST);

void processCommand(char* command) {
  char* arg1;
  char* arg2;

  Serial.print("Received command '");
  Serial.print(command);
  Serial.println("'");

  if(!strcmp(CMD_SETUP_LIST, command)) {
    if(!settings.isLoaded())
      settings.load();
    Serial.println("\tSetup:");
    for(keypos_t i = 0; i < settings.getKeysUsed(); i++)
    {
      Serial.print("\t");
      Serial.print(settings.keys[i]);
      Serial.print(" = '");
      Serial.print(settings.values[i]);
      Serial.println("'");
    }
  }

  if(!strcmp(CMD_SETUP_SAVE, command)) {
    Serial.print("\tSaving setup");
    settings.save();
    return start();
  }

  if(!memcmp(CMD_SETUP_SET, command, CMD_SETUP_SET_LEN)) {
    Serial.print("\tSetup set '");
    arg1 = command + CMD_SETUP_SET_LEN;
    arg2 = arg1;
    while(*arg2 != 0) {
      if(*arg2 == '=') {
        *arg2 = 0;
        arg2++;
        break;
      }
      arg2++;
    }
    Serial.print(arg1);
    Serial.print("' to '");
    Serial.print(arg2);
    Serial.print("': ");
    if(strlen(arg1))
      Serial.println(settings.set(arg1, arg2) ? "OK" : "Failed");
    else
      Serial.println("Arguments not given");
  }
}

void processSerialCommand() {
  *serialCommandCurrentByte = 0;
  processCommand(serialCommandBuffer);
}

void loop() {

  if(startSuccessfull)
  {
    scrolls.loop();
    clock_show();
    ble_loop();
  }

  while(Serial.available())
  {
    uint8_t incomingByte = Serial.read();
    if(incomingByte == 10) {
      processSerialCommand();
      serialCommandCurrentByte = serialCommandBuffer;
    }
    if((incomingByte > 31) && (serialCommandCurrentByte - serialCommandBuffer < MAX_SERIAL_COMMAND_SIZE))
      *(serialCommandCurrentByte++) = incomingByte;
  }

  delay(50);
}