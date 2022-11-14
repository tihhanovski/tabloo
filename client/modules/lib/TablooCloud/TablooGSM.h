// Sample code for https://www.aliexpress.com/item/4000456922720.html?spm=a2g0s.12269583.0.0.75154f39xjmZMx
// Based on example from TinyGSM by Volodymyr Shymanskyy 
// https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
// 

/**
 * Tabloo - opensource bus stop display
 * Connection code for GSM 
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 * Based on example from TinyGSM by Volodymyr Shymanskyy 
 * https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino 
 *
 */

#ifndef _TABLOOGSM_H_
#define _TABLOOGSM_H_

//All setup moved to preferences, see TablooSetup.h
//const char apn[]      = "internet.tele2.ee"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
//const char gprsUser[] = ""; // GPRS User
//const char gprsPass[] = ""; // GPRS Password
// SIM card PIN (leave empty, if not defined)
//const char simPIN[]   = "0000";

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <Arduino.h>
//#include <ArduinoHttpClient.h>  //see https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
#include <TablooGeneral.h>      //Various utilities
#include <TablooTime.h>
#include <TablooSetup.h>        //Settings storage

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif


//#define I2C_SDA              21
//#define I2C_SCL              22
//#include <Wire.h>
// I2C for SIM800 (to keep it running when powered from battery)
//TwoWire I2CPower = TwoWire(0);

// TinyGSM Client for Internet connection
TinyGsmClientSecure client(modem);

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00


void networking_start() {
    // Start I2C communication
    //I2CPower.begin(I2C_SDA, I2C_SCL, 400);
    //I2CPower.begin(I2C_SDA, I2C_SCL, 400000);

    // Keep power when running from battery
    //bool isOk = setPowerBoostKeepOn(1);
    //SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

    // Set modem reset, enable, power pins
    pinMode(MODEM_PWKEY, OUTPUT);
    pinMode(MODEM_RST, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);
    digitalWrite(MODEM_PWKEY, LOW);
    digitalWrite(MODEM_RST, HIGH);
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Set GSM module baud rate and UART pins
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    delay(3000);

    // Restart SIM800 module, it takes quite some time
    // To skip it, call init() instead of restart()
    log_i("Initializing modem...");
    unsigned long t = millis();
    //modem.restart();
    modem.restart();
    // use modem.init() if you don't need the complete restart
    t = millis() - t;
    log_v("Restarted modem in %d", t);

    #if CORE_DEBUG_LEVEL == 5
    String modemInfo = modem.getModemInfo();
    log_v("Modem info: %s", modemInfo);
    #endif

    char* simPIN = setup_readStringValue(SETUP_KEY_GSM_PIN);
    log_v("SIM PIN: %s", simPIN);

    // Unlock your SIM card with a PIN if needed
    if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
        log_v("Unlocking SIM with PIN");
        modem.simUnlock(simPIN);
    } else
        log_v("No need to unlock SIM");
}

SimpleDateTime networking_request_datetime() {
    int   year3    = 0;
    int   month3   = 0;
    int   day3     = 0;
    int   hour3    = 0;
    int   min3     = 0;
    int   sec3     = 0;
    float timezone = 0;

    SimpleDateTime ret;
    for (int8_t i = 5; i; i--) {
        log_v("Requesting current network time");
        if (modem.getNetworkTime(&year3, &month3, &day3, 
            &hour3, &min3, &sec3,
            &timezone)) 
        {
            log_v("Time: %d-%d-%d %d:%d:%d %.1f", year3, month3, day3, hour3, min3, sec3, timezone);

            if(year3 <= 2004 && month3 == 1 && day3 == 1) {
                //TODO !!
                log_e("Wrong datetime");
            }

            ret.hours = hour3;
            ret.minutes = min3;
            ret.seconds = sec3;
            ret.year = year3 > 2000 ? year3 - 2000 : year3;
            ret.month = month3;
            ret.day = day3;
            ret.offset = trunc(4 * timezone);   //TODO is it 
            break;
        } else {
            log_w("Couldn't get network time (try %d), retrying in 5s.", i);
            //SerialMon.print("Couldn't get network time, retrying in 15s.");
            delay(5000L);
        }
    }
    return ret;
}

boolean networking_connect() {
    if (!modem.isNetworkConnected()) {
        log_i("Network disconnected");
        if (!modem.waitForNetwork(120000L, true)) {  //180000L
            log_w("Network connection failed");
            delay(1000);    ///10000
            return false;
        }
        if (modem.isNetworkConnected()) {
            log_i("Network connected");
            log_i("Signal quality: %d", modem.getSignalQuality());
        }
    }

    if (!modem.isGprsConnected()) {
        bool success  = false;
        char* apn = setup_getStringValue(SETUP_KEY_GSM_APN);
        char* gprsUser = setup_getStringValue(SETUP_KEY_GSM_USER);
        char* gprsPass = setup_getStringValue(SETUP_KEY_GSM_PASS);
        if(strlen(apn)) {
            log_i("GPRS disconnected, connecting to %s as '%s' : '%s'", apn, gprsUser, gprsPass);
            success = modem.gprsConnect(apn, gprsUser, gprsPass);
        }
        else
            log_e("No APN provided. cant connect to GPRS");
        delete apn;
        delete gprsUser;
        delete gprsPass;

        if (!success) {
            log_w("GPRS disconnection failed");
            delay(1000);   //10000
            return false;
        }
        if (modem.isGprsConnected()) { 
            log_i("GPRS connected"); 
            log_i("Signal quality: %d", modem.getSignalQuality());
        }
    }
    return true;
}


/*void checkLocation() {
    #ifdef TINY_GSM_MODEM_HAS_GSM_LOCATION
        float lat      = 0;
        float lon      = 0;
        float accuracy = 0;
        int   year     = 0;
        int   month    = 0;
        int   day      = 0;
        int   hour     = 0;
        int   min      = 0;
        int   sec      = 0;
        for (int8_t i = 3; i; i--) {
            log_v("%d.\tRequesting current GSM location", i);
            if (modem.getGsmLocation(&lat, &lon, &accuracy, &year, &month, &day, &hour, &min, &sec)) {
                log_v("Latitude: %f, longitude: %f", lat, lon);
                log_v("Accuracy: %f", accuracy);
                log_v("Year: %d, month: %d, day: %d", year, month, day);
                log_v("Hour: %d, min: %d, sec: %d", hour, min, sec);
                break;
            } else {
                log_v("Couldn't get GSM location, retrying in 15s.");
                delay(15000L);
            }
        }
        log_v("Retrieving GSM location again as a string");
        String location = modem.getGsmLocation();
        log_v("GSM Based Location String: %s", location);

    #else
        log_v("No location");
    #endif
}*/

/*SimpleTime network_request_time() {
    int   year3    = 0;
    int   month3   = 0;
    int   day3     = 0;
    int   hour3    = 0;
    int   min3     = 0;
    int   sec3     = 0;
    float timezone = 0;

    SimpleTime ret;
    for (int8_t i = 5; i; i--) {
        log_v("Requesting current network time");
        if (modem.getNetworkTime(&year3, &month3, &day3, 
            &hour3, &min3, &sec3,
            &timezone)) 
        {
            log_v("Year: %d-%d-%d %d:%d:%d %.1f", year3, month3, day3, hour3, min3, sec3, timezone);

            ret.hours = hour3;
            ret.minutes = min3;
            ret.seconds = sec3;
            break;
        } else {
            log_w("Couldn't get network time (try %d), retrying in 15s.", i);
            //SerialMon.print("Couldn't get network time, retrying in 15s.");
            delay(15000L);
        }
    }
    return ret;
}*/

/*bool setPowerBoostKeepOn(int en){
    I2CPower.beginTransmission(IP5306_ADDR);
    I2CPower.write(IP5306_REG_SYS_CTL0);
    if (en) {
        I2CPower.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    } else {
        I2CPower.write(0x35); // 0x37 is default reg value
    }
    return I2CPower.endTransmission() == 0;
}*/


#endif