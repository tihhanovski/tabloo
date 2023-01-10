#ifndef _TABLOOMATRIXPANEL_H_
#define _TABLOOMATRIXPANEL_H_

/**
 * Tabloo - opensource bus stop display
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 * Matrix panel functions
 * 
 */


// RGB LED panel library, 
// see https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>

// Fonts converted using software from
// https://www.sigmdel.ca/michel/program/misc/gfxfont_8bit_en.html
// BitCasual is public domain font by geoff, see
// http://www.pentacom.jp/pentacom/bitfontmaker2/gallery/?id=353
#include <BitCasual8pt8b.h>
GFXfont defaultFont = BitCasual8pt8b;

#define ROWHEIGHT defaultFont.yAdvance
#define FONT_BASELINE ROWHEIGHT - 4

#include <TablooGeneral.h>
#include <TablooTime.h>
#include "Marquee.h"        // Marquee library
#include <BusStopData.h>

#include "TalTechLogo.h"    // hardcoded Taltech logo




MatrixPanel_I2S_DMA* dma_display = nullptr;
//VirtualMatrixPanel displayx(*dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, SERPENT, TOPDOWN);
VirtualMatrixPanel* display = nullptr;

const uint16_t DISPLAY_COLOR_BLACK = dma_display->color444(0, 0, 0);
const uint16_t DISPLAY_COLOR_RED = dma_display->color444(15, 0, 0);
Marquees scrolls;           // Marquees collection

/**
 * Show the error message on matrix panel as a marquee
 * @param msg error message
 */
void displayError(const char* msg) {
    log_i("Display error '%s'", msg);
    scrolls.clear();
    display->fillRect(0, 0, display->width(), display->height(), DISPLAY_COLOR_BLACK);
    scrolls.add(new Marquee(msg, display, 0, 0, display->width(), 8, 300, DISPLAY_COLOR_BLACK));
}

// convert message to correctly display umlauts and dieresises 
// with adafruit fonts, 
// see https://www.sigmdel.ca/michel/program/misc/gfxfont_8bit_en.html
void convert_inplace_to_cp(uint8_t* msg, uint32_t msgLength) {
    uint8_t* c = msg;
    // If msgLength is 0, then 
    for(uint32_t i = 0; (msgLength && (i < msgLength)) || *c; i++) {
        if (0xA0 <= *c && *c <= 0xFF)
            *c = *c - 32;
        c++;
    }
}

// Just output incoming message to the lower row
void matrix_outputMessage(uint8_t* message, uint32_t messageLength) {
    int16_t x, y, w, h;
    x = 0;
    y = display->height() - ROWHEIGHT - 2;
    h = ROWHEIGHT + 2;
    w = 96;

    display->setFont(&defaultFont);
    display->fillRect(x, y, w, h, DISPLAY_COLOR_BLACK);
    display->setTextColor(DISPLAY_COLOR_RED);
    display->setCursor(x + 2, y + FONT_BASELINE);

    convert_inplace_to_cp(message, messageLength);
    display->write(message, messageLength);

    display->drawRect(x, y, w, h, DISPLAY_COLOR_RED);
}

void matrix_startScrolls(BusStopData &timetable) {

    // calculate space needed for bus short names
    // also convert all the strings to output them correctly
    uint16_t lnw = 0;
    int16_t bx, by;
    uint16_t bw, bh;
    display->setFont(&defaultFont);
    for(uint8_t i = 0; i < timetable.lineCount; i++) {
        LineData l = timetable.lines[i];
        display->getTextBounds(l.shortName, 0, 0, &bx, &by, &bw, &bh);
        if(bw > lnw)
            lnw = bw;

        // convert strings:
        convert_inplace_to_cp((uint8_t*)l.shortName, 0);
        convert_inplace_to_cp((uint8_t*)l.longName, 0);
        convert_inplace_to_cp((uint8_t*)l.headSign, 0);
    }
    lnw += 2;

    scrolls.clear();
    display->setTextColor(display->color444(0,0,15));

    for(uint8_t i = 0; i < timetable.lineCount; i++)
    {
        LineData l = timetable.lines[i];
        log_v("Line %d: %s\t%s\t%s", i, l.shortName, l.longName, l.headSign);

        display->fillRect(0, i * ROWHEIGHT, lnw, ROWHEIGHT, DISPLAY_COLOR_BLACK);
        display->setCursor(1, i * ROWHEIGHT + FONT_BASELINE);
        display->print(l.shortName);
        
        scrolls.add(new Marquee(
            l.longName, display, 
            lnw, 
            i * ROWHEIGHT,  //TODO 
            display->width() - lnw - 18,     //TODO why 18?
            ROWHEIGHT, 
            150, //msecs per pixel
            DISPLAY_COLOR_BLACK
            ));
    }
}

void matrix_splashscreen() {
    log_v("Showing splash screen");

    const uint16_t logo_width = 64;
    const uint16_t logo_height = 32;

    const uint16_t logo_left = (display->width() - logo_width) / 2;
    const uint16_t logo_top = (display->height() - logo_height) / 2;

    // Show beautiful splash screen with taltech logo for 1.5 seconds
    display->setTextWrap(false);
    display->setTextColor(display->color444(15,0,0));
    display->fillRect(0, 0, display->width(), display->height(), DISPLAY_COLOR_BLACK);

    log_v("Showing logo");
    for(uint8_t i = 1; i < 16; i++) {
        display->drawBitmap(logo_left, logo_top, TalTechLogo, logo_width, logo_height, 
            DISPLAY_COLOR_BLACK, display->color444(15,i,i));
        delay(50);
    }
    delay(1000);
    for(uint8_t i = 15; i > 1; i--) {
        display->drawBitmap(logo_left, logo_top, TalTechLogo, logo_width, logo_height, 
            DISPLAY_COLOR_BLACK, display->color444(15,i,i));
        delay(50);
    }
    log_v("Logo hidden");

    // Clear the screen 
    display->fillRect(0, 0, display->width(), display->height(), DISPLAY_COLOR_BLACK);
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
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(30); //0-255
    display = new VirtualMatrixPanel(*dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, SERPENT, TOPDOWN);

    display->clearScreen();
    display->fillScreen(DISPLAY_COLOR_BLACK);

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
    const uint8_t TIMES_H = display->height() - ROWHEIGHT;

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


        if(time_is_initialized()) { //isTimeInitialized()
            uint8_t h;
            uint8_t m;
            uint8_t dow = time_get_day_of_week(); //getDayOfWeek();
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
                cvsTimesToWait->setFont(&defaultFont);
                cvsTimesToWait->getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
                //log_v("Width for %d is %d", minutesToWait, bw - bx);

                cvsTimesToWait->setCursor(TIMES_W - bw + bx, i * ROWHEIGHT + FONT_BASELINE);
                cvsTimesToWait->print(minutesToWait);
                log_v("%d : %d min", i, minutesToWait);
            }
        } else {
            cvsTimesToWait->drawRoundRect(0, 0, TIMES_W, TIMES_H, 3, DISPLAY_COLOR_RED);
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

        display->getTextBounds(String("99"), 0, 0, &bx, &by, &bw, &bh);

        nextClockTime = t + 500;
        timeColon = !timeColon;
        int x = display->width() - bw * 2 - 3;
        int y = display->height() - ROWHEIGHT;
        display->fillRect(x - 2, y, bw * 2 + 5, ROWHEIGHT, DISPLAY_COLOR_BLACK);
        display->drawRect(x - 2, y, bw * 2 + 5, ROWHEIGHT, DISPLAY_COLOR_RED);

        uint8_t h;
        uint8_t m;
        getHourAndMinute(h, m);

        String hh, mm;

        if(time_is_initialized()) {  //isTimeInitialized()
            hh = (h < 10 ? "0" : "") + String(h);
            mm = (m < 10 ? "0" : "") + String(m);
        } else {
            hh = "--";
            mm = "--";
        }

        display->setTextColor(display->color444(0,15,0));
        display->setCursor(x, y + FONT_BASELINE);
        display->print(hh);
        if(timeColon)
        {
            display->setCursor(x + bw - 2, y + FONT_BASELINE);
            display->print(":");
        }
        display->setCursor(x + bw + 2, y + FONT_BASELINE);
        display->print(mm);
    }
}

void matrix_loop(BusStopData& timetable) {
    scrolls.loop();
    waiting_times_show(timetable);
    clock_show();
}




#endif