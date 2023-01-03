// based on GxEPD_Hello World Example by Jean-Marc Zingg

//
// Created by Jean-Marc Zingg based on demo code from Good Display,
// available on http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// The e-paper displays are available from:
//
// https://www.aliexpress.com/store/product/Wholesale-1-54inch-E-Ink-display-module-with-embedded-controller-200x200-Communicate-via-SPI-interface-Supports/216233_32824535312.html
//
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_8363&product_id=35120
// or https://www.aliexpress.com/store/product/E001-1-54-inch-partial-refresh-Small-size-dot-matrix-e-paper-display/600281_32815089163.html
//

#define DISPLAY_WAITINGTIMES_REFRESH_INTERVAL_MILLIS 30000;
#define DISPLAY_CLOCK_REFRESH_INTERVAL_MILLIS 30000;

#include <GxEPD.h>
#include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r
#include GxEPD_BitmapExamples

// FreeFonts from Adafruit_GFX
#include <Fonts/Picopixel.h>
#include <Fonts/Tiny3x3a2pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>


#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <BusStopData.h>

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4


void displayLine(LineData &l, uint8_t pos) {

    display.setRotation(1);

    log_v("\t%s:\t%s\t%s", l.shortName, l.longName, l.headSign);

    uint16_t x = 0;
    uint16_t y = pos * 20;
    uint16_t w = GxEPD_WIDTH;
    uint16_t h = 20;
    display.fillRect(x, y, w, h, GxEPD_WHITE);

    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(x + 2, y + 18);
    display.setTextColor(GxEPD_RED);
    display.print(l.shortName);

    display.setFont(&Picopixel);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x + 30, y + 8);
    display.print(l.longName);
    display.setCursor(x + 30, y + 16);
    display.print(l.headSign);
}


void display_showLines(BusStopData &timetable) {
    log_v("Output lines");
    display.fillScreen(GxEPD_WHITE);
    for(uint8_t i = 0; i < timetable.lineCount; i++)
        displayLine(timetable.lines[i], i);
    display.update();
    display.powerDown();
}

void display_splashscreen() {
    log_v("Displaying splashscreen");
    const char* HELLO_MSG = "Tabloo stardib...";

    display.setFont(&FreeMonoBold9pt7b);
    display.setRotation(1);
 
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(HELLO_MSG, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center bounding box by transposition of origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(HELLO_MSG);
    display.update();
    display.powerDown();
    log_v("Splashscreen displayed");
}

void displayLineWaitingTime(uint16_t minToWait, uint8_t pos) {

    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);

    char buf[5] = {0};
    sprintf(buf, "%d", minToWait);
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(buf, 0, 0, &tbx, &tby, &tbw, &tbh);

    uint16_t x = display.width() - 40;
    uint16_t y = pos * 20;
    uint16_t w = 40;
    uint16_t h = 20;

    display.setCursor(display.width() - tbw - 2, y + 18);
    display.setTextColor(GxEPD_BLACK);
    display.print(buf);
}
void display_showWaitingTimes(BusStopData& timetable) {

    static unsigned long nextWTUpdateTime = 0;
    unsigned long t = millis();
    if (t < nextWTUpdateTime)
        return;
    nextWTUpdateTime = t + DISPLAY_WAITINGTIMES_REFRESH_INTERVAL_MILLIS;

    if(time_is_initialized()) {
        uint8_t h;
        uint8_t m;
        uint8_t dow = time_get_day_of_week();
        getHourAndMinute(h, m);
        log_v("recalculating times for %d:%d, dow=%d", h, m, dow);

        uint16_t x = display.width() - 40;
        display.fillRect(x, 0, 40, 20 * timetable.lineCount, GxEPD_WHITE);
        for(uint8_t i = 0; i < timetable.lineCount; i++) {
            uint16_t minutesToWait = timetable.getWaitingTime(i, h, m, dow);
            displayLineWaitingTime(minutesToWait, i);
        }
        display.updateWindow(x, 0, 40, 20 * timetable.lineCount, true);
        display.powerDown();
    }
}

void display_message(const uint8_t* msg) {
    log_v("message will be displayed: '%s'", msg);

    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);

    uint16_t x = 0;
    uint16_t y = display.height() - 20;
    uint16_t w = display.width() - 60;
    uint16_t h = 20;
    display.fillRect(x, y - 20, w, h, GxEPD_WHITE);

    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds((char*)msg, 0, 0, &tbx, &tby, &tbw, &tbh);

    display.setCursor(x, y + tbh + 2);
    display.setTextColor(GxEPD_BLACK);
    display.print((char*)msg);
    display.updateWindow(x, y, w, h);
    display.powerDown();

    log_v("message displayed");
}

unsigned long nextClockTime = 0;     // Time when next clock repaint should occur
bool timeColon = false;             // Colon between hh and mm in clock to visualise ticking

/**
 * Repaint clock area two times every second
 */
void display_updateClock() {

    static unsigned long nextClockUpdateTime = 0;
    unsigned long t = millis();
    if (t < nextClockUpdateTime)
        return;
    nextClockUpdateTime = t + DISPLAY_CLOCK_REFRESH_INTERVAL_MILLIS;

    if(time_is_initialized()) {
        display.setRotation(1);
        display.setFont(&FreeMonoBold9pt7b);

        uint8_t th, tm;
        getHourAndMinute(th, tm);


        char buf[8] = {0};
        sprintf(buf, "%02d:%02d", th, tm);
        int16_t tbx, tby; uint16_t tbw, tbh;
        display.getTextBounds(buf, 0, 0, &tbx, &tby, &tbw, &tbh);

        uint16_t x = display.width() - tbw - 2;
        uint16_t y = display.height() - 2;
        uint16_t w = tbw + 2;
        uint16_t h = tbh + 2;

        log_v("refresh clock %02d:%02d rect:(%d, %d, %d, %d)", th, tm, x, y, w, h);

        display.fillRect(x, y - 20, w, 20, GxEPD_WHITE);
        display.setCursor(x, y);
        display.setTextColor(GxEPD_BLACK);
        display.print(buf);
        display.updateWindow(x, y - 20, w, 20);
        display.powerDown();
    }
}

void display_loop(BusStopData& timetable) {
    display_showWaitingTimes(timetable);
    display_updateClock();
}

/*
void showPartialUpdate()
{
    // use asymmetric values for test
    uint16_t box_x = 10;
    uint16_t box_y = 15;
    uint16_t box_w = 70;
    uint16_t box_h = 20;
    uint16_t cursor_y = box_y + box_h - 6;
    float value = 13.95;
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(0);
    // draw background
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    delay(2000);

    // partial update to full screen to preset for partial update of box window
    // (this avoids strange background effects)
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    // show where the update box is
    for (uint16_t r = 0; r < 4; r++)
    {
        display.setRotation(r);
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
        display.updateWindow(box_x, box_y, box_w, box_h, true);
        delay(1000);
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.updateWindow(box_x, box_y, box_w, box_h, true);
    }
    // show updates in the update box
    for (uint16_t r = 0; r < 4; r++)
    {
        // reset the background
        display.setRotation(0);
        display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
        display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
        display.setRotation(r);
        for (uint16_t i = 1; i <= 10; i++)
        {
            display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
            display.setCursor(box_x, cursor_y);
            display.print(value * i, 2);
            display.updateWindow(box_x, box_y, box_w, box_h, true);
            delay(2000);
        }
        delay(2000);
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.updateWindow(box_x, box_y, box_w, box_h, true);
    }
    // should have checked this, too
    box_x = GxEPD_HEIGHT - box_x - box_w - 1; // not valid for all corners
    // should show on right side of long side
    // reset the background
    display.setRotation(0);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
    // show where the update box is
    for (uint16_t r = 0; r < 4; r++)
    {
        display.setRotation(r);
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
        display.updateWindow(box_x, box_y, box_w, box_h, true);
        delay(1000);
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.updateWindow(box_x, box_y, box_w, box_h, true);
    }
    // show updates in the update box
    for (uint16_t r = 0; r < 4; r++)
    {
        // reset the background
        display.setRotation(0);
        display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
        display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
        display.setRotation(r);
        if (box_x >= display.width()) continue; // avoid delay
        for (uint16_t i = 1; i <= 10; i++)
        {
            display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
            display.setCursor(box_x, cursor_y);
            display.print(value * i, 2);
            display.updateWindow(box_x, box_y, box_w, box_h, true);
            delay(2000);
        }
        delay(2000);
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.updateWindow(box_x, box_y, box_w, box_h, true);
    }
    display.setRotation(0);
    display.powerDown();
}
*/
