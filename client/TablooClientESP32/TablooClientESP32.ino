#include <WiFi.h>
#include <ezTime.h>   // Search for "ezTime" in the Arduino Library manager: https://github.com/ropg/ezTime


#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "Marquee.h"
#include "TalTechLogo.h"

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

MatrixPanel_I2S_DMA *display = nullptr;

uint16_t black = display->color444(0, 0, 0);

Timezone tz;

//#define WIFI_SSID "Telia-86EB42"        // your network SSID (name)
//#define WIFI_PASS "EMPBNVJUMMHXRR"      // your network key

//#define WIFI_SSID "k10-firstfloor"      // your network SSID (name)
//#define WIFI_PASS "aPustiKaV1nternet"   // your network key

#define WIFI_SSID "Intellisoft_public"    // your network SSID (name)
#define WIFI_PASS "Profit201704"          // your network key

#define TIMEZONE "EE"                   // "Europe/Tallinn"

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

  /*
  // fix the screen with green
  display->fillRect(0, 0, display->width(), display->height(), display->color444(0, 15, 0));
  delay(500);

  // draw a box in yellow
  display->drawRect(0, 0, display->width(), display->height(), display->color444(15, 15, 0));
  delay(500);

  // draw an 'X' in red
  display->drawLine(0, 0, display->width()-1, display->height()-1, display->color444(15, 0, 0));
  display->drawLine(display->width()-1, 0, 0, display->height()-1, display->color444(15, 0, 0));
  delay(500);

  // draw a blue circle
  display->drawCircle(10, 10, 10, display->color444(0, 0, 15));
  delay(500);

  // fill a violet circle
  display->fillCircle(40, 21, 10, display->color444(15, 0, 15));
  delay(500);

  // fill the screen with 'black'
  display->fillScreen(display->color444(0, 0, 0));

  */

  display->setTextSize(1);     // size 1 == 8 pixels high

  display->setTextWrap(true); // Don't wrap at end of line - will do ourselves
  display->setTextColor(display->color444(15,15,15));

  /*
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

  delay(1000);

  display->clearScreen();
  display->fillScreen(black);
  display->setCursor(0, 0);
  display->print("NTP...");

  // Setup EZ Time
  tz.setLocation(TIMEZONE);

  setDebug(INFO);
  waitForSync();

  Serial.print("Time in your set timezone:         ");
  Serial.println(tz.dateTime());

  Serial.println();
  Serial.println("Time in various internet standard formats ...");
  Serial.println();
  Serial.println("ATOM:        " + tz.dateTime(ATOM)); 
  Serial.println("COOKIE:      " + tz.dateTime(COOKIE));
  Serial.println("IS8601:      " + tz.dateTime(ISO8601));
  Serial.println("RFC822:      " + tz.dateTime(RFC822));
  Serial.println("RFC850:      " + tz.dateTime(RFC850));
  Serial.println("RFC1036:     " + tz.dateTime(RFC1036));
  Serial.println("RFC1123:     " + tz.dateTime(RFC1123));
  Serial.println("RFC2822:     " + tz.dateTime(RFC2822));
  Serial.println("RFC3339:     " + tz.dateTime(RFC3339));
  Serial.println("RFC3339_EXT: " + tz.dateTime(RFC3339_EXT));
  Serial.println("RSS:         " + tz.dateTime(RSS));
  Serial.println("W3C:         " + tz.dateTime(W3C));
  Serial.println();
  Serial.println(" ... and any other format, like \"" + tz.dateTime("G:i") + "\"");
  */

  display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
  display->setTextColor(display->color444(15,0,0));
  display->fillRect(0, 0, display->width(), display->height(), black);

  display->drawBitmap(0, 0, TalTechLogo, 64, 32, black, display->color444(15,15,15));

  delay(1500);
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
      "Ringliin p\xE4rna märna kaubamärna üks kaks kolm šaakal", display, 
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

unsigned long nextTimeTime = 0;
bool timeColon = false;
void clock_show()
{
  unsigned long t = millis();
  if(t >= nextTimeTime)
  {
    nextTimeTime = t + 1000;
    timeColon = !timeColon;
    int x = 36;
    display->fillRect(x, 24, 30, 8, black);
    display->setCursor(x, 24);
    display->print(tz.dateTime(timeColon ? "GG:i" : "GG i"));
  }
}

void loop() {
  scrolls.loop();
  clock_show();
  delay(50);
}
