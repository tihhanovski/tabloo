//HTTP: See https://randomnerdtutorials.com/esp32-http-get-post-arduino/


#include <WiFi.h>
//#include <ezTime.h>   // Search for "ezTime" in the Arduino Library manager: https://github.com/ropg/ezTime
#include <HTTPClient.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "Marquee.h"
#include "TalTechLogo.h"

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

MatrixPanel_I2S_DMA *display = nullptr;

const uint16_t black = display->color444(0, 0, 0);
const uint16_t white = display->color444(15, 15, 15);
const uint16_t red = display->color444(15, 0, 0);

#define WIFI_SSID "Tartu Kolledž"
#define WIFI_PASS "Puie5tee"

#define HTTP_SERVER "https://dev.intellisoft.ee/tabloo/ask/?c="
#define BUSSTOP_ID "7820161-1"

Marquees scrolls;

void setup() {

  Serial.begin(115200);

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

  // Connect to WiFi
  display->setCursor(0, 0);
  display->print("WiFi");
  //display->print(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    display->print(".");
    delay(500);
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
  display->setTextColor(display->color444(15,0,0));
  display->fillRect(0, 0, display->width(), display->height(), black);

  display->drawBitmap(0, 0, TalTechLogo, 64, 32, black, display->color444(15,15,15));

  delay(1500);

  loadData();
  
  display->fillRect(0, 0, display->width(), display->height(), black);

  scrolls.add(
    new Marquee(
      "Hello display", display, 
      0, 
      0, 
      display->width(), 
      8, 
      100, 
      black
    ));

  scrolls.add(
    new Marquee(
      "Ringtee - ERMi - Pärna", display, 
      0, 
      8, 
      display->width(), 
      8, 
      200, 
      black
      ));

  scrolls.add(
    new Marquee(
      "Nagu näha, täpitähed üldse ei toimi siin", display, 
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

struct LineData {
  const char* no;
  const char* name;
  const char* dir;
};

void loadData() {
  HTTPClient http;
  //TODO 
  String serverPath = String(HTTP_SERVER) + String(BUSSTOP_ID);
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String sPayload = http.getString();
    const char* payload = sPayload.c_str();

    Serial.println("Payload -----------");
    Serial.println(payload);
    Serial.println("-------------------");


    setTime(payload[0], payload[1], payload[2]);

    LineData* lines = new LineData[payload[3]];

    const char* addr = payload + 4;
    for(uint8_t i = 0; i < payload[3]; i++)
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
    
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

unsigned long timeDelta;

void setTime(uint8_t h, uint8_t m, uint8_t s) {
  timeDelta = 1000 * (h * 3600 + m * 60 + s) - millis();
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
  delay(50);
}
