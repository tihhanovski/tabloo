/**
 * Tabloo - opensource bus stop display
 * Marquee to show long texts on small display
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 */

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Arduino.h>

#ifdef USE_GFX_ROOT
#define DISPLAYCLASS GFX
#elif !defined NO_GFX
#define DISPLAYCLASS Adafruit_GFX
#endif

#define DOUBLEBUFFER_CANVAS_ADDITIONAL_WIDTH 16


/**
 * Own marquee will be created for every line
*/
class Marquee {
    const uint8_t* str;
    DISPLAYCLASS *display = nullptr;

    // long strLength;
    // long widthInChars;
    long pos = 0;
    // long charWidth = 6;
    uint16_t background;
    uint8_t x; 
    uint8_t y;
    uint8_t w;
    uint8_t h;
    uint16_t textWidth;
    // bool bHalfStep = false;

    GFXcanvas1 *canvas, *canvas2;

public:

    unsigned long nextScrollTime = 0;
    unsigned long mspp = 1000;

    const uint8_t* getMessage()
    {
        return str;
    }

    Marquee(const uint8_t* msg, DISPLAYCLASS *display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, unsigned long msecPerPixels, uint16_t bg)
    {
        // Create two canvas for double buffering
        // One canvas will be slightly larger for "smooth right border" look
        canvas = new GFXcanvas1(w + DOUBLEBUFFER_CANVAS_ADDITIONAL_WIDTH, h); // TODO why 16?
        canvas->setFont(&defaultFont);
        canvas2 = new GFXcanvas1(w, h);

        str = msg;

        // Calculate string width
        // TODO rewrite for more efficient code
        int16_t bx, by;
        uint16_t bh, bw;
        uint8_t cs[2];
        cs[1] = 0;
        textWidth = 0;

        for(const uint8_t* c = str; *c; c++) {
            cs[0] = *c;
            canvas->getTextBounds((const char*)cs, 0, 0, &bx, &by, &bw, &bh);
            textWidth += bw;
        }

        this->display = display;
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        mspp = msecPerPixels;
        background = bg;
        pos = 0;
    }

    /**
     * Destructor
     * free canvas memory
    */
    ~Marquee()
    {
        delete canvas;
        delete canvas2;
    }

    void loop() {
        canvas->fillScreen(background);

        canvas->setCursor(-pos, FONT_BASELINE);
        canvas->write((const char*)str);

        long p2 = (textWidth - pos) + w / 2 + 30;
        if(p2 < w) {
            canvas->setCursor(p2, FONT_BASELINE);
            canvas->write((const char*)str);
        }

        canvas2->drawBitmap(0, 0, canvas->getBuffer(), canvas->width(), canvas->height(), 65000, background);
        display->drawBitmap(x, y, canvas2->getBuffer(), w, h, 65000, background);

        pos++;
        if(p2 <= 1)
            pos = 0;
    }
};

/**
 * Marquee list item
*/
struct MarqueeArrayItem{
    Marquee* item;
    MarqueeArrayItem* next = nullptr;
};

/**
 * List of marquees
*/
class Marquees {

    MarqueeArrayItem* items = nullptr;

    String toScroll;
    unsigned long nextScrollTime = 0;
    long pos = 0;
    long charWidth = 6;

    MatrixPanel_I2S_DMA *display = nullptr;

public:
    void setDisplay(MatrixPanel_I2S_DMA *d) {
        display = d;
    }

    void clear()
    {
        if(items == nullptr)  // nothing left to clean 
            return;
        log_v("delete %s", items->item->getMessage());

        delete items->item;
        MarqueeArrayItem* n = items->next;
        delete items;
        items = n;
        clear();
    }

    void add(Marquee* m)
    {
        MarqueeArrayItem* newItem = new MarqueeArrayItem();
        newItem->item = m;
        newItem->next = items;
        items = newItem;    
    }

    void loop() {
        unsigned long t = millis();
        MarqueeArrayItem* i = items;
        while(i != nullptr) {
            if(t >= i->item->nextScrollTime)
            {
                i->item->loop();
                i->item->nextScrollTime = t + i->item->mspp;
            }
            i = i->next;
        }
    }
};
