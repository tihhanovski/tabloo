/**
 * Tabloo - opensource bus stop display
 * Marquee to show long texts on small display
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 * TODO: comments
 * 
 */

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Arduino.h>

#ifdef USE_GFX_ROOT
#define DISPLAYCLASS GFX
#elif !defined NO_GFX
#define DISPLAYCLASS Adafruit_GFX
#endif


class Marquee {
    String  str;
    DISPLAYCLASS *display = nullptr;

    long strLength;
    long widthInChars;
    long pos = 0;
    long charWidth = 6;
    uint16_t background;
    uint8_t x; 
    uint8_t y;
    uint8_t w;
    uint8_t h;
    bool bHalfStep = false;

    GFXcanvas1 *canvas, *canvas2;

public:

    unsigned long nextScrollTime = 0;
    unsigned long mspp = 1000;

    const char* getMessage()
    {
        return str.c_str();
    }

    Marquee(String msg, DISPLAYCLASS *display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, unsigned long msecPerPixels, uint16_t bg)
    {
        widthInChars = w / charWidth + 1;
        str = "           " + msg;
        strLength = str.length();
        
        this->display = display;
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        mspp = msecPerPixels;
        background = bg;

        canvas = new GFXcanvas1(w + 8, h);
        canvas2 = new GFXcanvas1(w, h);
    }

    ~Marquee()
    {
        delete canvas;
        delete canvas2;
    }

    void loop() {
        long pp = pos / charWidth;
        long lastPos = pp + widthInChars;
        if(lastPos > strLength)
            lastPos = strLength;
        if(pp >= str.length())
        {
            pos = 0;
        }

        //if(bHalfStep)
        canvas->fillScreen(background);
        canvas->setCursor(- (pos % charWidth), 0);
        canvas->print(str.substring(pp));

        canvas2->drawBitmap(0, 0, canvas->getBuffer(), w + 8, h, 65000, background);

        display->drawBitmap(x, y, canvas2->getBuffer(), w, h, 65000, background);
        //display->drawRect(x, y, w, h, display->color444(0, 10, 15));

        //if(!bHalfStep)
        pos++;

        bHalfStep = !bHalfStep;
    }
};

struct MarqueeArrayItem{
  Marquee* item;
  MarqueeArrayItem* next = nullptr;
};

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
    Serial.print("Marquees.clear: delete '");
    Serial.print(items->item->getMessage());
    Serial.println("'");

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
