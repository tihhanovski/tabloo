#ifndef _TABLOOMATRIXPANEL_H_
#define _TABLOOMATRIXPANEL_H_

// RGB LED panel library, 
// see https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <TablooGeneral.h>
#include <TablooTime.h>
#include "Marquee.h"        // Marquee library
#include <BusStopData.h>

#include "TalTechLogo.h"    // hardcoded Taltech logo

MatrixPanel_I2S_DMA *display = nullptr;

const uint16_t COLOR_BLACK = display->color444(0, 0, 0);
const uint16_t COLOR_RED = display->color444(15, 0, 0);
Marquees scrolls;           // Marquees collection

/**
 * Show the error message on matrix panel as a marquee
 * @param msg error message
 */
void displayError(const char* msg) {
    log_i("Display error '%s'", msg);
    scrolls.clear();
    display->fillRect(0, 0, display->width(), display->height(), COLOR_BLACK);
    scrolls.add(new Marquee(msg, display, 0, 0, display->width(), 8, 300, COLOR_BLACK));
}

void matrix_startScrolls(BusStopData &timetable) {
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

        display->fillRect(0, i * 8, 12, 8, COLOR_BLACK);
        display->setCursor(0, i * 8);
        display->print(timetable.lines[i].shortName);
        
        scrolls.add(new Marquee(
            timetable.lines[i].longName, display, 
            12, 
            i * 8,  //TODO 
            display->width() - 30, 
            8, 
            150, 
            COLOR_BLACK
            ));
    }
}

void matrix_splashscreen() {
    log_v("Showing splash screen");

    // Show beautiful splash screen with taltech logo for 1.5 seconds
    display->setTextWrap(false);
    display->setTextColor(display->color444(15,0,0));
    log_v("Showing logo");
    display->fillRect(0, 0, display->width(), display->height(), COLOR_BLACK);
    for(uint8_t i = 1; i < 16; i++) {
        display->drawBitmap(0, 0, TalTechLogo, 64, 32, COLOR_BLACK, display->color444(15,i,i));
        delay(50);
    }
    delay(1000);
    for(uint8_t i = 15; i > 1; i--) {
        display->drawBitmap(0, 0, TalTechLogo, 64, 32, COLOR_BLACK, display->color444(15,i,i));
        delay(50);
    }
    log_v("Logo hidden");

    // Clear the screen 
    display->fillRect(0, 0, display->width(), display->height(), COLOR_BLACK);
    delay(500);
}

void matrix_init(uint16_t resX, uint16_t resY, uint8_t chain) {
    // Display module configuration
    
    HUB75_I2S_CFG mxconfig(
        resX,   // module width
        resY,   // module height
        chain    // Chain length
    );

    mxconfig.gpio.e = 18;
    mxconfig.clkphase = false;
    mxconfig.driver = HUB75_I2S_CFG::FM6126A;

    // Display Setup
    display = new MatrixPanel_I2S_DMA(mxconfig);
    display->begin();
    display->setBrightness8(30); //0-255
    display->clearScreen();
    display->fillScreen(COLOR_BLACK);

    display->setTextSize(1);     // size 1 == 8 pixels high
    display->setTextWrap(true); // Don't wrap at end of line - will do ourselves
    display->setTextColor(display->color444(15,15,15));
}

unsigned long nextTimeWait = 0;     // Time when waiting times will be calculated next time

void matrix_resetCurrentTime() {
    nextTimeWait = 0;
}

GFXcanvas1* cvsTimesToWait = nullptr;
/**
 * Show times to wait for every line
 */
void waiting_times_show(BusStopData& timetable) {

    const uint8_t TIMES_W = 18;
    const uint8_t TIMES_H = display->height() - 8;

    const int16_t TIMES_X = display->width() - TIMES_W;
    const int16_t TIMES_Y = 0;
    
    const unsigned long TIMES_RECALC_WAIT = 30000UL;

    if(!timetable.lineCount)
        return;
    unsigned long t = millis();
    if(cvsTimesToWait == nullptr)
        cvsTimesToWait = new GFXcanvas1(TIMES_W, TIMES_H);
    if (t >= nextTimeWait)
    {
        nextTimeWait = t + TIMES_RECALC_WAIT;       // Recalculate every 30 seconds


        if(isTimeInitialized()) {
            uint8_t h;
            uint8_t m;
            uint8_t dow = getDayOfWeek();
            getHourAndMinute(h, m);
            log_v("recalculating times for %d:%d, dow=%d", h, m, dow);

            cvsTimesToWait->fillRect(0, 0, TIMES_W, TIMES_H, 0);
            for(uint8_t i = 0; i < timetable.lineCount; i++) {
                // For every line

                uint16_t minutesToWait = timetable.getWaitingTime(i, h, m, dow);

                int16_t bx, by;
                uint16_t bw, bh;
                char buf[5] = {0};
                sprintf(buf, "%d", minutesToWait);
                display->getTextBounds(String(buf), 0, 0, &bx, &by, &bw, &bh);
                //log_v("Width for %d is %d", minutesToWait, bw - bx);

                cvsTimesToWait->setCursor(TIMES_W - bw + bx, i * 8);
                cvsTimesToWait->print(minutesToWait);
            }
        } else {
            cvsTimesToWait->drawRoundRect(0, 0, TIMES_W, TIMES_H, 3, COLOR_RED);
        }
    }
    display->drawBitmap(TIMES_X, TIMES_Y, cvsTimesToWait->getBuffer(), TIMES_W, TIMES_H, display->color444(15,0,0), 0);
}

unsigned long nextClockTime = 0;     // Time when next clock repaint should occur
bool timeColon = false;             // Colon between hh and mm in clock to visualise ticking

/**
 * Repaint clock area two times every second
 */
void clock_show() {
    unsigned long t = millis();
    if(t >= nextClockTime)
    {


        
        int16_t bx, by;
        uint16_t bw, bh;

        display->getTextBounds(String("00"), 0, 0, &bx, &by, &bw, &bh);

        nextClockTime = t + 500;
        timeColon = !timeColon;
        int x = display->width() - bw * 2 - 3;
        int y = display->height() - bh;
        display->fillRect(x, y, bw * 2 + 3, bh, COLOR_BLACK);

        uint8_t h;
        uint8_t m;
        getHourAndMinute(h, m);

        String hh, mm;

        if(isTimeInitialized()) {
            hh = (h < 10 ? "0" : "") + String(h);
            mm = (m < 10 ? "0" : "") + String(m);
        } else {
            hh = "--";
            mm = "--";
        }

        display->setTextColor(display->color444(0,15,0));
        display->setCursor(x, y);
        display->print(hh);
        if(timeColon)
        {
            display->setCursor(x + bw - 2, y);
            display->print(":");
        }
        display->setCursor(x + bw + 2, y);
        display->print(mm);
    }
}

void matrix_loop(BusStopData& timetable) {
    scrolls.loop();
    waiting_times_show(timetable);
    clock_show();
}




#endif