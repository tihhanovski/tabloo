#include <Arduino.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "Connection.h"
#include <HTTPClient.h>

#include "Marquee.h"
#include "TalTechLogo.h"
#include "Util.h"
#include "BLESetup.h"

#define EEPROM_SIZE 200     // Setup EEPROM storage size
#include "Storage.h"

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

MatrixPanel_I2S_DMA *display = nullptr;

const uint16_t black = display->color444(0, 0, 0);

//#define BUSSTOP_ID "7820161-1"

char* data;
size_t dataSize;

Marquees scrolls;

Storage storage(10);      // TODO set proper max keys count

struct LineData {
  const char* no;
  const char* name;
  const char* dir;
};

unsigned long timeDelta;

void setTime(uint8_t h, uint8_t m, uint8_t s) {
  timeDelta = 1000 * (h * 3600 + m * 60 + s) - millis();
}

const char* STORAGE_BUSSTOP_ID = "si";

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

  char* stopId = storage.get(STORAGE_BUSSTOP_ID);
  if(stopId == nullptr)
  {
    setTime(0, 0, 0);
    displayError("No stop ID in setup, please set");
    return;
  }

  //String serverPath = String(HTTP_SERVER) + String(stopId);
  
  fetchData(stopId, data, dataSize);

  Serial.print("Setting time ... ");
  setTime(data[0], data[1], data[2]);
  Serial.println("OK");

  LineData* lines = new LineData[data[3]];

  const char* addr = data + 4;
  for(uint8_t i = 0; i < data[3]; i++)
  {
    addr += strlen(lines[i].no = addr);
    addr += strlen(lines[i].name = addr);
    addr += strlen(lines[i].dir = addr);          
    Serial.print("Line ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(lines[i].no);
    Serial.print("\t");
    Serial.print(lines[i].name);
    Serial.print("\t");
    Serial.print(lines[i].dir);
    Serial.print("\n");
  }

  outputMemoryData();
}

void start()
{
  // Show beautiful splash screen with taltech logo
  display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
  display->setTextColor(display->color444(15,0,0));
  display->fillRect(0, 0, display->width(), display->height(), black);
  display->drawBitmap(0, 0, TalTechLogo, 64, 32, black, display->color444(15,15,15));
  delay(1500);

  // Load data
  loadData(); 

  // Clear the screen 
  display->fillRect(0, 0, display->width(), display->height(), black);
}

void setup() {

  Serial.begin(115200);

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

  startConnection();  // Start Wifi connection

  start();
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

void loop() {
  scrolls.loop();
  clock_show();
  ble_loop();
  delay(50);
}