/**
 * Tabloo - opensource bus stop display
 * Main module
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 * current state of code: Proof of concept. 
 * 1. Downloads some data from server and displays it
 * 2. Possible to set up via serial port or BLE
 * 3. Uses DHT11 sensor and shows its output (temperature)
 * TODO: refactor code to get rid of spaghetti style
 * TODO: Wifi -> LoRa?
 * TODO: Larger display (2x1 modules, maybe 3x2 or 4x2 modules?)
 * TODO: Send sensor and telemetry data to server
 * TODO: Bug in setup - data corrupted sometimes when {setup set xxx=yyy} used
 * TODO: Better BLE code
 * TODO: CPU panics (and controller reboots) if no data fetched from HTTP (any error) 
 * 
 */

#define BLEINPUT_ENABLED false
#define LOGO_ENABLED true
#define MATRIX_HUB75_ENABLED true
#define EXTERNAL_SENSORS_ENABLED false
#define WIFI_ENABLED false

#define UARTIO_DEBUG true
#include <HardwareSerial.h>
#include <UARTIO.h>

HardwareSerial SerialPort(2); // use UART2
UARTIO io(SerialPort);


#include <Arduino.h>

// ESP32 internal SD filesystem. Used to store settings etc
#include <SPIFFS.h>

// Temperature and humidity sensor library
// see https://github.com/beegee-tokyo/DHTesp
// #include <DHTesp.h>         

#include "Settings.h"       // Settings module - key-value storage
#include "SerialInput.h"    // Getting input from serial

#if WIFI_ENABLED
#include "Connection.h"     // Wifi stub for connection
#endif

// RGB LED panel library, 
// see https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA
#if MATRIX_HUB75_ENABLED
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Marquee.h"        // Marquee library
#endif


#if LOGO_ENABLED
#include "TalTechLogo.h"    // hardcoded Taltech logo
#endif

#include "Util.h"           // Misc utilities will be here

#if BLEINPUT_ENABLED
#include "BLEInput.h"       // BLE setup TODO
#endif

#include <BusStopData.h>

//#include "Timetables.h"     // Timetables code

#if EXTERNAL_SENSORS_ENABLED
#include "ExternalSensors.h"        // I2C connectivity with external sensors
#endif

//HardwareSerial SerialPort(2); // use UART2


// #define DHTPIN 21               // Digital pin connected to the DHT sensor
// #define DHTTYPE DHTesp::DHT11   // DHT 11
Settings settings(10);      // TODO set proper max keys count

// DHTesp dht;

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

const char* SETUP_BUSSTOP_ID = "si";    // Bus stop ID setup variable
const char* SETUP_WIFI_SSID = "ws";     // Wifi SSID setup var
const char* SETUP_WIFI_PASSWORD = "wp"; // Wifi password setup variable

#if MATRIX_HUB75_ENABLED
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32      // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1       // Total number of panels chained one to another

MatrixPanel_I2S_DMA *display = nullptr;

const uint16_t black = display->color444(0, 0, 0);
const uint16_t red = display->color444(15, 0, 0);
Marquees scrolls;           // Marquees collection

/**
 * Show the error message on matrix panel as a marquee
 * @param msg error message
 */
#endif

void displayError(const char* msg) {
  Serial.print("ERROR! ");
  Serial.println(msg);
  #if MATRIX_HUB75_ENABLED
  scrolls.clear();
  display->fillRect(0, 0, display->width(), display->height(), black);
  scrolls.add(new Marquee(msg, display, 0, 0, display->width(), 8, 300, black));
  #endif
}

char* data;                 // Buffer for data retrieved from server
size_t dataSize;            // Size of data
BusStopData timetable;        // Timetable object

unsigned long timeDelta;    // Difference (in msec) between microcontroller millis and actual current local time got from server

/**
 * Setup the time delta
 * @param h hours (0 - 23)
 * @param m minutes (0 - 59)
 * @param s seconds (0 - 59)
 */
void setTime(uint8_t h, uint8_t m, uint8_t s) {
  timeDelta = 1000 * (h * 3600 + m * 60 + s) - millis();
}


/**
 * Load data from server and display it
 */
void loadData(char* msg, uint32_t size) {
  /*char* stopId = settings.get(SETUP_BUSSTOP_ID);
  if(stopId == nullptr)
  {
    setTime(0, 0, 0);
    displayError("No stop ID in setup, please set");
    return;
  }*/

  dataSize = size;
  data = new char[dataSize];
  memcpy(data, msg, dataSize);


  //fetchData(stopId, data, dataSize);

  Serial.print("Setting time ... ");
  setTime(data[0], data[1], data[2]);
  Serial.println("OK");

  timetable.initialize(data);

  #if MATRIX_HUB75_ENABLED
  scrolls.clear();
  display->setTextColor(display->color444(0,0,15));
  #endif
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

    #if MATRIX_HUB75_ENABLED
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
    #endif
  }

  outputMemoryData();
}

/**
 * Start the system
 */
void start()
{
  Serial.println("Starting");

  #if MATRIX_HUB75_ENABLED
  // Show beautiful splash screen with taltech logo for 1.5 seconds
  display->setTextWrap(false);
  display->setTextColor(display->color444(15,0,0));
  display->fillRect(0, 0, display->width(), display->height(), black);


  #if LOGO_ENABLED
  display->drawBitmap(0, 0, TalTechLogo, 64, 32, black, display->color444(15,15,15));
  delay(1500);
  #endif

  // Clear the screen 
  display->fillRect(0, 0, display->width(), display->height(), black);
  #endif
  
  #if WIFI_ENABLED
  loadData();
  #endif
}

const char* CMD_SETUP_SAVE = "setup save";                    // Comand to save setup
const char* CMD_SETUP_SET = "setup set ";                     // Command to set setup variable
const size_t CMD_SETUP_SET_LEN = strlen(CMD_SETUP_SET);
const char* CMD_SETUP_LIST = "setup list";                    // Command to show setup variables and their values

/**
 * Process command received
 * @param command c string
 */
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

bool startSuccessfull;    // True if start was successfull

SerialInput serialInput(processCommand);

void initMatrixDisplay() {
  #if MATRIX_HUB75_ENABLED
  // Display module configuration
  
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

  display->setCursor(0, 0);
  display->print("Connecting...");
  #endif
}

boolean bLedOn = false;

void setup() {

  startSuccessfull = false;

  #if BLEINPUT_ENABLED
  ble_onCommandReceived = processCommand;
  #endif

  Serial.begin(115200);

  // UART
  SerialPort.begin(15200, SERIAL_8N1, 33, 32);
  delay(1000);
  Serial.println("UART slave:");
  pinMode(2, OUTPUT);

  
  /*SerialPort.begin(15200, SERIAL_8N1, 33, 32);
  delay(1000);
  Serial.println("UART slave started on 33 and 32");
  pinMode(2, OUTPUT);
  */
  // UART


  #if EXTERNAL_SENSORS_ENABLED
  startExternalSensors();
  #endif

  // Start internal filesystem
  // Settings (and more) is stored here
  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS Mount Failed");
  }

  outputMemoryData();

  initMatrixDisplay();


  #if BLEINPUT_ENABLED
  startBleSetup();
  #endif

  // dht.setup(DHTPIN, DHTTYPE);
	// Serial.println("DHT initiated");



  #if WIFI_ENABLED
  const char* wifiSSID = settings.get(SETUP_WIFI_SSID);
  if(wifiSSID == nullptr)
  {
    Serial.println("No Wifi SSID found in settings.");
    return;
  }

  // Start Wifi connection
  if (!startConnection(wifiSSID, settings.get(SETUP_WIFI_PASSWORD)))
    return;
  start();
  #endif

  startSuccessfull = true;
}


void getCurrentHoursAndMinutes(uint8_t& h, uint8_t& m)
{
  unsigned long tt = (timeDelta + millis()) / 60000;
  h = (tt / 60) % 24;
  m = tt % 60;
}


unsigned long nextTimeTime = 0;     // Time when next clock repaint should occur
bool timeColon = false;             // Colon between hh and mm in clock to visualise ticking

/**
 * Repaint clock area two times every second
 */
void clock_show() {
  unsigned long t = millis();
  if(t >= nextTimeTime)
  {
    int16_t bx, by;
    uint16_t bw, bh;

    display->getTextBounds(String("00"), 0, 0, &bx, &by, &bw, &bh);
    #if MATRIX_HUB75_ENABLED
    nextTimeTime = t + 500;
    timeColon = !timeColon;
    int x = display->width() - bw * 2 - 3;
    int y = display->height() - bh;
    display->fillRect(x, y, bw * 2 + 3, bh, black);

    uint8_t h;
    uint8_t m;
    getCurrentHoursAndMinutes(h, m);

    String hh = (h < 10 ? "0" : "") + String(h);
    String mm = (m < 10 ? "0" : "") + String(m);

    display->setTextColor(display->color444(0,15,0));
    display->setCursor(x, y);
    display->print(hh);
    if(timeColon)
    {
      display->setCursor(x + bw - 3, y);
      display->print(":");
    }
    display->setCursor(x + bw + 2, y);
    display->print(mm);
    #endif
  }
}

#if MATRIX_HUB75_ENABLED
GFXcanvas1* cvsTimesToWait = nullptr;
unsigned long nextTimeWait = 0;     // Time when waiting times will be calculated next time

/**
 * Show times to wait for every line
 */
void waiting_times_show() {
  unsigned long t = millis();
  if(cvsTimesToWait == nullptr)
    cvsTimesToWait = new GFXcanvas1(18, display->height() - 8);
  if (t >= nextTimeWait)
  {
    Serial.println("times to wait:");
    nextTimeWait = t + 30000;       // Recalculate every 30 seconds

    uint8_t h;
    uint8_t m;
    getCurrentHoursAndMinutes(h, m);

    cvsTimesToWait->fillRect(0, 0, 18, 24, 0);

    for(uint8_t i = 0; i < timetable.lineCount; i++) {
      // For every line
      cvsTimesToWait->setCursor(0, i * 8);
      cvsTimesToWait->print(timetable.getWaitingTime(i, h, m));
    }
  }
  display->drawBitmap(display->width() - 18, 0, cvsTimesToWait->getBuffer(), 18, 24, display->color444(15,0,0), 0);
}
#endif

unsigned long nextDhtPollingTime = 0;

bool b = false;

void loop() {

  if(startSuccessfull)
  {
    #if MATRIX_HUB75_ENABLED
    scrolls.loop();
    waiting_times_show();
    //display_dht();
    clock_show();
    #endif

    //processExternalSensors();
  }

  if (SerialPort.available())
  {

    unsigned long l = millis();

    UARTIO_Message msg;
    io.readMessage(msg);
    l = millis() - l;
    Serial.print("Received in ");
    Serial.println(l);

    display->drawRect(0, 0, display->width(), display->height(), red);
    delay(500);

    loadData(msg.body, msg.length);
    delete msg.body;

    b = !b;

    digitalWrite(2, b);
  }

  serialInput.loop();
  delay(50);
}